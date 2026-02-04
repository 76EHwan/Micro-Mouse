/*
 * drive.c
 *
 *  Created on: Nov 11, 2025
 *      Author: kth59
 */

#include "menu.h"
#include "adc.h"
#include "lsm6ds3tr-c.h"

uint8_t battery_percent;

void Show_Battery() {
	LCD_Printf(25, 0, ST7789_WHITE, ST7789_BLACK, "%.2f", vbattery);
}

void Show_IMU() {
	LCD_Printf(17, 0, ST7789_WHITE, ST7789_BLACK, "%3.2f  ", imu_data.Yaw_Angle);
}
