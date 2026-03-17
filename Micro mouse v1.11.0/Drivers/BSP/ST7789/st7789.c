/*
 * st7789.c
 *
 *  Created on: Dec 20, 2025
 *      Author: kth59
 */

#include "st7789.h"
#include "spi.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h> // memset 사용을 위해 추가

// ==========================================================
// [설정] 프레임버퍼 정의
// ==========================================================
// 240 * 135 * 2 bytes = 약 64.8 KB
// DMA 전송을 위해 volatile 선언 및 32비트 정렬 권장
#if defined(__GNUC__)
__attribute__((aligned(32)))

#endif
volatile uint16_t st7789_framebuffer[ST7789_WIDTH * ST7789_HEIGHT];

// 색상 엔디언 변환 매크로 (Little Endian MCU -> Big Endian LCD)
#define SWAP_BYTES(color) ((uint16_t)(((color) >> 8) | ((color) << 8)))

// SPI 핸들 (st7789.h에서 define으로 설정된 이름 사용)
extern SPI_HandleTypeDef ST7789_SPI_PORT;

// ==========================================================
// [내부 함수] 기본 제어
// ==========================================================

static void ST7789_Delay(uint32_t ms) {
	HAL_Delay(ms);
}

static void ST7789_Select(void) {
	HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET);
}

static void ST7789_UnSelect(void) {
	HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET);
}

// 초기화 명령 전송용 (Blocking 모드 유지)
void ST7789_WriteCommand(uint8_t cmd) {
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET); // Command
	HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
	ST7789_UnSelect();
}

void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET); // Data
	HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
	ST7789_UnSelect();
}

static void ST7789_WriteSmallData(uint8_t data) {
	ST7789_WriteData(&data, 1);
}

static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1,
		uint16_t y1) {
	uint16_t x_start = x0 + ST7789_X_SHIFT;
	uint16_t x_end = x1 + ST7789_X_SHIFT;
	uint16_t y_start = y0 + ST7789_Y_SHIFT;
	uint16_t y_end = y1 + ST7789_Y_SHIFT;

	ST7789_WriteCommand(0x2A); // CASET
	{
		uint8_t data[] = { (x_start >> 8) & 0xFF, x_start & 0xFF, (x_end >> 8)
				& 0xFF, x_end & 0xFF };
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0x2B); // RASET
	{
		uint8_t data[] = { (y_start >> 8) & 0xFF, y_start & 0xFF, (y_end >> 8)
				& 0xFF, y_end & 0xFF };
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0x2C); // RAMWR
}

// ==========================================================
// [초기화] ST7789_Init
// ==========================================================
void ST7789_Init(void) {
	ST7789_WriteCommand(0x01); // SWRESET
	ST7789_Delay(150);

	ST7789_WriteCommand(0x11); // SLPOUT
	ST7789_Delay(255);

	ST7789_WriteCommand(0x3A); // COLMOD
	ST7789_WriteSmallData(0x55); // 16-bit color

	ST7789_WriteCommand(0x36); // MADCTL
	ST7789_WriteSmallData(0xA0); // 가로 모드 (필요시 0x70 등 변경)

	ST7789_WriteCommand(0x21); // INVON (색상 반전)

	ST7789_WriteCommand(0x29); // DISPON
	ST7789_Delay(10);

	// 프레임버퍼 초기화 및 화면 클리어
	ST7789_FillScreen(ST7789_BLACK);
	ST7789_UpdateScreen(); // 초기 검은 화면 즉시 전송
}

// ==========================================================
// [핵심] 화면 갱신 함수 (DMA 사용)
// 메인 루프나 타이머에서 주기적으로 호출해주세요.
// ==========================================================
void ST7789_UpdateScreen(void) {
	// [중요] SPI가 현재 전송 중(BUSY)이라면 기다리지 말고 함수를 바로 종료(Skip)합니다.
	// 이렇게 해야 모터 제어 루프가 멈추지 않습니다.
	if (ST7789_SPI_PORT.State != HAL_SPI_STATE_READY) {
		return;
	}

	// --- 아래는 기존 코드 ---
	ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

	// 캐시 클린 (H5 시리즈 D-Cache가 켜져있을 경우)
	// SCB_CleanDCache();

	// DMA 전송 시작
	if (HAL_SPI_Transmit_DMA(&ST7789_SPI_PORT, (uint8_t*) st7789_framebuffer,
			sizeof(st7789_framebuffer)) != HAL_OK) {
		ST7789_UnSelect();
	}
}

// ==========================================================
// [그리기 함수] 이제 SPI를 안 쓰고 RAM(버퍼)만 건드립니다. (매우 빠름)
// ==========================================================

// 화면 채우기 (memset 사용 최적화 시도 가능하나 루프가 안전)
void ST7789_FillScreen(uint16_t color) {
	uint16_t swapped_color = SWAP_BYTES(color);
	for (int i = 0; i < ST7789_WIDTH * ST7789_HEIGHT; i++) {
		st7789_framebuffer[i] = swapped_color;
	}
}

// 점 찍기
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
		return;

	// 인덱스 계산 및 색상 변환 후 저장
	st7789_framebuffer[y * ST7789_WIDTH + x] = SWAP_BYTES(color);
}

// 이미지 그리기 (배열 복사)
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		const uint16_t *data) {
	if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
		return;
	if ((x + w) > ST7789_WIDTH)
		w = ST7789_WIDTH - x;
	if ((y + h) > ST7789_HEIGHT)
		h = ST7789_HEIGHT - y;

	for (uint16_t row = 0; row < h; row++) {
		for (uint16_t col = 0; col < w; col++) {
			// 원본 데이터가 이미 Big Endian(Byte swapped)되어 있는지 확인 필요.
			// 보통 이미지 배열은 0xRRGGBB -> RGB565 변환된 값이므로
			// 여기서는 원본 데이터를 그대로 쓴다고 가정하고, 필요시 SWAP_BYTES 추가
			// 일반적인 변환 툴은 Little Endian으로 줄 수 있으므로 SWAP 적용
			st7789_framebuffer[(y + row) * ST7789_WIDTH + (x + col)] =
					SWAP_BYTES(data[row * w + col]);
		}
	}
}

// 8x16 폰트 그리기 (버퍼에 쓰기)
void ST7789_DrawUser8x16(uint16_t x, uint16_t y, char *str, uint16_t color,
		uint16_t bgcolor) {
	uint8_t i, j;
	char ch;
	uint16_t color_swapped = SWAP_BYTES(color);
	uint16_t bgcolor_swapped = SWAP_BYTES(bgcolor);

	while (*str) {
		ch = *str;
		if (ch < 32 || ch > 126) {
			str++;
			continue;
		}

		// 화면 밖으로 나가면 그리지 않음
		if (x + 8 > ST7789_WIDTH || y + 16 > ST7789_HEIGHT) {
			str++;
			continue; // 혹은 break;
		}

		const uint8_t *pData = asc2_1608[ch - 32];

		for (i = 0; i < 8; i++) { // 8 Columns
			uint8_t top = pData[i * 2];
			uint8_t bot = pData[i * 2 + 1];

			for (j = 0; j < 8; j++) { // 8 Rows (Top part)
				if (top & (0x80 >> j))
					st7789_framebuffer[(y + j) * ST7789_WIDTH + (x + i)] =
							color_swapped;
				else
					st7789_framebuffer[(y + j) * ST7789_WIDTH + (x + i)] =
							bgcolor_swapped;
			}
			for (j = 0; j < 8; j++) { // 8 Rows (Bottom part)
				if (bot & (0x80 >> j))
					st7789_framebuffer[(y + j + 8) * ST7789_WIDTH + (x + i)] =
							color_swapped;
				else
					st7789_framebuffer[(y + j + 8) * ST7789_WIDTH + (x + i)] =
							bgcolor_swapped;
			}
		}
		x += 8;
		str++;
	}
}

// Printf 함수
void LCD_Printf(uint16_t x, uint16_t y, uint16_t fontcolor, uint16_t bgcolor,
		const char *fmt, ...) {
	static char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	ST7789_DrawUser8x16(x * 8, y * 16, buf, fontcolor, bgcolor);
}

void TIM4_IRQ_Handle() {
	static uint16_t lcd_frame_count = 0;
	lcd_frame_count++;

	if (lcd_frame_count >= 833) {
		lcd_frame_count = 0;

		// 화면 갱신 요청
		ST7789_UpdateScreen();

		// [디버깅] SPI 상태 확인 (만약 BUSY라면 인터럽트 설정 문제)
		if (HAL_SPI_GetState(&ST7789_SPI_PORT) != HAL_SPI_STATE_READY) {
			// 여기에 브레이크포인트를 걸거나 LED를 켜보세요.
			// 이 안으로 들어온다면 100% SPI 인터럽트가 안 켜진 것입니다.
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		}

		TRIG_TOGGLE;
	}
}

void SPI1_Tx_IRQ() {
	ST7789_UnSelect();
}

