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
	LCD_Printf(0, 7, ST7789_WHITE, ST7789_BLACK, "L U:%.3f ",
			focL.Iu);
	LCD_Printf(11, 7, ST7789_WHITE, ST7789_BLACK, "V:%.3f ",
			focL.Iv);
	LCD_Printf(20, 7, ST7789_WHITE, ST7789_BLACK, "W:%.3f ",
			-focL.Iu-focL.Iv);
//	LCD_Printf(0, 7, ST7789_WHITE, ST7789_BLACK, "R U:%.3f ",
//			focR.Iu);
//	LCD_Printf(11, 7, ST7789_WHITE, ST7789_BLACK, "V:%.3f ",
//			focR.Iv);
//	LCD_Printf(20, 7, ST7789_WHITE, ST7789_BLACK, "W:%.3f ",
//			-focR.Iu-focR.Iv);
}

void Show_ToF() {
	LCD_Printf(0, 6, ST7789_WHITE, ST7789_BLACK, "L:%4d CL:%4d CR:%4d R:%4d ",
			sensor_L, sensor_CL, sensor_CR, sensor_R);
}
