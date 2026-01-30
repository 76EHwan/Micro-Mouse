/*
 * st7789.c
 *
 *  Created on: Dec 20, 2025
 *      Author: kth59
 */


#include "st7789.h"
#include "spi.h" // spi.h가 포함되어야 hspi1을 인식합니다.
#include <stdio.h>
#include <stdarg.h>

extern SPI_HandleTypeDef ST7789_SPI_PORT;

// 간단한 딜레이 매크로
static void ST7789_Delay(uint32_t ms) {
    HAL_Delay(ms);
}

// CS 핀 제어
static void ST7789_Select(void) {
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
}
static void ST7789_UnSelect(void) {
    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}

// 커맨드 전송 (DC = 0)
void ST7789_WriteCommand(uint8_t cmd) {
    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET); // Command Mode
    HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
    ST7789_UnSelect();
}

// 데이터 전송 (DC = 1)
void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET); // Data Mode
    HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
    ST7789_UnSelect();
}

// 1바이트 데이터 전송 래퍼
static void ST7789_WriteSmallData(uint8_t data) {
    ST7789_WriteData(&data, 1);
}

// 윈도우 설정 (핵심: Offset 적용)
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint16_t x_start = x0 + ST7789_X_SHIFT;
    uint16_t x_end = x1 + ST7789_X_SHIFT;
    uint16_t y_start = y0 + ST7789_Y_SHIFT;
    uint16_t y_end = y1 + ST7789_Y_SHIFT;

    ST7789_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x_start >> 8) & 0xFF, x_start & 0xFF, (x_end >> 8) & 0xFF, x_end & 0xFF };
        ST7789_WriteData(data, sizeof(data));
    }

    ST7789_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y_start >> 8) & 0xFF, y_start & 0xFF, (y_end >> 8) & 0xFF, y_end & 0xFF };
        ST7789_WriteData(data, sizeof(data));
    }

    ST7789_WriteCommand(0x2C); // RAMWR
}

// 초기화 함수
void ST7789_Init(void) {
    ST7789_WriteCommand(0x01); // SWRESET
    ST7789_Delay(150);

    ST7789_WriteCommand(0x11); // SLPOUT (Sleep Out)
    ST7789_Delay(255);

    ST7789_WriteCommand(0x3A); // COLMOD (Color Mode)
    ST7789_WriteSmallData(0x55); // 16-bit color (65k)

    // [중요] 방향 설정 (MADCTL) - 가로 모드 예시
    // 0x36 레지스터 비트: MY MX MV ML RGB MH ...
    // 가로 모드(LandScape) 설정 값: 보통 0x70 또는 0xA0 등을 사용
    ST7789_WriteCommand(0x36);
    ST7789_WriteSmallData(0xA0); // 방향에 따라 0x00, 0x60, 0x70, 0xC0 등으로 변경 필요

    // [중요] 색상 반전 (IPS 패널은 보통 반전 켜야 함)
    ST7789_WriteCommand(0x21); // INVON (Inversion On) -> 색이 이상하면 0x20(OFF)으로 변경

    ST7789_WriteCommand(0x29); // DISPON (Display On)
    ST7789_Delay(10);

    // 화면 검은색으로 클리어
    ST7789_FillScreen(ST7789_BLACK);
}

// 화면 전체 채우기
void ST7789_FillScreen(uint16_t color) {
    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    // 버퍼를 사용해 한 번에 쏘는 것이 빠름 (메모리 부족 시 루프로 변경)
    // 여기서는 간단히 픽셀 단위 전송을 예시로 들거나, 라인 버퍼를 쓸 수 있음
    // 아래는 단순 무식한 방법 (최적화 필요 시 DMA 권장)
    uint16_t i, j;
    uint8_t data[2] = { (color >> 8) & 0xFF, color & 0xFF };

    ST7789_Select();
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

    for(i = 0; i < ST7789_WIDTH; i++) {
        for(j = 0; j < ST7789_HEIGHT; j++) {
             HAL_SPI_Transmit(&ST7789_SPI_PORT, data, 2, 10);
        }
    }
    ST7789_UnSelect();
}

// 점 찍기
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if(x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;

    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[2] = { (color >> 8) & 0xFF, color & 0xFF };
    ST7789_WriteData(data, 2);
}

// 사용자 정의 8x16 폰트 전용 출력 함수
// [최종] 8x16 폰트 (초기 코드 로직 + 90도 회전 적용)
// [진짜 최종] 8x16 폰트 (세로 방향 + MSB First)
// 데이터 구조: [Col 0 Top] [Col 0 Bot] [Col 1 Top] [Col 1 Bot] ...
void ST7789_DrawUser8x16(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bgcolor) {
    uint8_t i, j;
    uint8_t top_byte, bot_byte;
    char ch;

    while (*str) {
        ch = *str;
        if (ch < 32 || ch > 126) {
            str++;
            continue;
        }

        const uint8_t *pData = asc2_1608[ch - 32];

        if (x + 8 >= ST7789_WIDTH) {
            x = 0;
            y += 16;
            if (y + 16 >= ST7789_HEIGHT) break;
        }

        // 왼쪽에서 오른쪽으로 8개 기둥(Column)을 그립니다.
        for (i = 0; i < 8; i++) {
            top_byte = pData[i * 2];     // 윗부분 데이터
            bot_byte = pData[i * 2 + 1]; // 아랫부분 데이터

            // [핵심 수정] 비트 7(MSB)이 가장 위쪽 픽셀입니다. (0x80부터 검사)

            // 1. 윗부분 8픽셀 그리기 (y ~ y+7)
            for (j = 0; j < 8; j++) {
                if ((top_byte << j) & 0x80) { // 비트를 왼쪽으로 밀면서 최상위 비트 확인
                    ST7789_DrawPixel(x + i, y + j, color);
                } else {
                    ST7789_DrawPixel(x + i, y + j, bgcolor);
                }
            }

            // 2. 아랫부분 8픽셀 그리기 (y+8 ~ y+15)
            for (j = 0; j < 8; j++) {
                if ((bot_byte << j) & 0x80) {
                    ST7789_DrawPixel(x + i, y + j + 8, color);
                } else {
                    ST7789_DrawPixel(x + i, y + j + 8, bgcolor);
                }
            }
        }

        x += 8;
        str++;
    }
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
    if((x + w - 1) >= ST7789_WIDTH) return;
    if((y + h - 1) >= ST7789_HEIGHT) return;

    // 1. 이미지가 들어갈 사각형 영역(Window)을 잡습니다.
    ST7789_Select();
    ST7789_SetAddressWindow(x, y, x+w-1, y+h-1);

    // 2. 데이터를 전송합니다.
    // (RGB565는 픽셀당 2바이트이므로, 전체 크기 * 2 만큼 보냅니다)
    ST7789_WriteData((uint8_t*)data, sizeof(uint16_t) * w * h);

    ST7789_UnSelect();
}

void LCD_Printf(uint16_t x, uint16_t y, uint16_t fontcolor, uint16_t bgcolor, const char* fmt, ...) {
    static char buf[128]; // 텍스트를 저장할 버퍼 (필요에 따라 크기 조절, 보통 128이면 충분)
    va_list args;

    // 가변 인자 처리 시작
    va_start(args, fmt);

    // 포맷 문자열과 변수들을 결합하여 문자열(buf)로 변환
    vsnprintf(buf, sizeof(buf), fmt, args);

    // 가변 인자 처리 끝
    va_end(args);

    // 만들어진 문자열을 기존 문자열 출력 함수로 전달
    ST7789_DrawUser8x16(8*x, 16*y, buf, fontcolor, bgcolor);
}
