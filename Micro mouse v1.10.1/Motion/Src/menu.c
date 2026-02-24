/*
 * drive.c
 *
 *  Created on: Nov 11, 2025
 *      Author: kth59
 */

#include "main.h"
#include "menu.h"
#include "sensor.h"
#include "adc.h"
#include "lsm6ds3tr-c.h"
#include "foc.h"

uint8_t battery_percent;

void Show_Battery() {
	vbattery = adc1_buffer[0] * 4.f * 3.3f / (1<<12);
	LCD_Printf(25, 0, ST7789_WHITE, ST7789_BLACK, "%.2f", vbattery);
}

void Show_IMU() {
	LCD_Printf(17, 0, ST7789_WHITE, ST7789_BLACK, "%3.2f  ",
			imu_data.Yaw_Angle);
}

void Show_Current() {
	Calc_DRV8316C_Current();
	LCD_Printf(0, 4, ST7789_WHITE, ST7789_BLACK, "L U: %.2f ",
			focL.Iu);
	LCD_Printf(11, 4, ST7789_WHITE, ST7789_BLACK, "V: %.2f ",
			focL.Iv);
	LCD_Printf(20, 4, ST7789_WHITE, ST7789_BLACK, "W: %.2f ",
			focL.Iw);
	LCD_Printf(0, 5, ST7789_WHITE, ST7789_BLACK, "R U: %.2f ",
			focR.Iu);
	LCD_Printf(11, 5, ST7789_WHITE, ST7789_BLACK, "V: %.2f ",
			focR.Iv);
	LCD_Printf(20, 5, ST7789_WHITE, ST7789_BLACK, "W: %.2f ",
			focR.Iw);
}

void Show_IR() {
	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "%04X %04X %04X %04X",
			sensor_L, sensor_CL, sensor_CR, sensor_R);
}
