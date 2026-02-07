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
#include "drv8316crq1.h"

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
	LCD_Printf(0, 1, ST7789_WHITE, ST7789_BLACK, "L U: %.2f ",
			DRV8316C_L.u_current);
	LCD_Printf(11, 1, ST7789_WHITE, ST7789_BLACK, "V: %.2f ",
			DRV8316C_L.v_current);
	LCD_Printf(20, 1, ST7789_WHITE, ST7789_BLACK, "W: %.2f ",
			DRV8316C_L.w_current);
	LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "R U: %.2f ",
			DRV8316C_R.u_current);
	LCD_Printf(11, 2, ST7789_WHITE, ST7789_BLACK, "V: %.2f ",
			DRV8316C_R.v_current);
	LCD_Printf(20, 2, ST7789_WHITE, ST7789_BLACK, "W: %.2f ",
			DRV8316C_R.w_current);

}

void Show_IR() {
	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "%04X %04X %04X %04X",
			adc2_buffer[0], adc2_buffer[1], adc2_buffer[2], adc2_buffer[3]);
}
