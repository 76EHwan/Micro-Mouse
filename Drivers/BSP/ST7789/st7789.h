/*
 * st7789.c
 *
 *  Created on: Dec 20, 2025
 *      Author: kth59
 */


#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"
#include "fonts.h"

// ==========================================================
// [사용자 설정] 화면 해상도 및 오프셋 (1.14인치 전용)
// ==========================================================
// [수정] 세로 모드 해상도 (135 x 240)
#define ST7789_WIDTH  240
#define ST7789_HEIGHT 135

// [수정] 세로 모드 오프셋 (화면이 한쪽으로 쏠리면 이 값을 조절하세요)
#define ST7789_X_SHIFT 40
#define ST7789_Y_SHIFT 52

// ==========================================================
// [사용자 설정] 핀 정의 (main.h의 define에 맞춰 수정하세요)
// ==========================================================
// 예: CubeMX에서 핀 이름을 LCD_CS, LCD_DC... 로 지었다면 그대로 두셔도 됩니다.
#define ST7789_SPI_PORT hspi3  // 사용 중인 SPI 핸들 (hspi1, hspi2 등)

#define ST7789_DC_PORT  LCD_DC_GPIO_Port
#define ST7789_DC_PIN   LCD_DC_Pin

#define ST7789_CS_PORT  LCD_CS_GPIO_Port
#define ST7789_CS_PIN   LCD_CS_Pin

// ==========================================================
// 색상 정의 (RGB565)
// ==========================================================
#define ST7789_BLACK   0x0000
#define ST7789_BLUE    0x001F
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_CYAN    0x07FF
#define ST7789_MAGENTA 0xF81F
#define ST7789_YELLOW  0xFFE0
#define ST7789_WHITE   0xFFFF

// ==========================================================
// 함수 선언
// ==========================================================
void ST7789_Init(void);
void ST7789_UpdateScreen(void);
void ST7789_FillScreen(uint16_t color);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_WriteCommand(uint8_t cmd);
void ST7789_WriteData(uint8_t *buff, size_t buff_size);
void ST7789_DrawUser8x16(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bgcolor);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void LCD_Printf(uint16_t x, uint16_t y, uint16_t fontcolor, uint16_t bgcolor, const char* fmt, ...);
#endif
