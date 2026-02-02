/*
 * drive.c
 *
 *  Created on: Nov 11, 2025
 *      Author: kth59
 */

#include "menu.h"
#include "adc.h"

uint8_t battery_percent;

void Show_Battery() {
	LCD_Printf(25, 0, ST7789_WHITE, ST7789_BLACK, "%.2f", vbattery);
}
