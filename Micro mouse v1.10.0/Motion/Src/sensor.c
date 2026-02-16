/*
 * sensor.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#include "sensor.h"
#include "adc.h"

#include "lsm6ds3tr-c.h"
#include "foc.h"

uint16_t sensor_L;
uint16_t sensor_CL;
uint16_t sensor_CR;
uint16_t sensor_R;

void TIM6_IRQ_Handle() {
	static uint32_t prev_tick = 0;
	uint32_t cur_tick = TIM2->CNT;
	float dt = (cur_tick - prev_tick) * 0.000001f;

	prev_tick = cur_tick;

	LSM6DS3_ReadGyro_Z_Only(&imu_data);

	float gyro_z_dps = imu_data.Gyro_Z - imu_data.Gyro_Z_Offset;

	if (gyro_z_dps > -0.5f && gyro_z_dps < 0.5f) {
		gyro_z_dps = 0.0f;
	}

	imu_data.Yaw_Angle += gyro_z_dps * dt;

	if (imu_data.Yaw_Angle >= 360.0f)
		imu_data.Yaw_Angle -= 360.0f;
	else if (imu_data.Yaw_Angle < 0.0f)
		imu_data.Yaw_Angle += 360.0f;
}

void TIM15_IRQ_Handle() {
	HAL_GPIO_WritePin(IR_EN_GPIO_Port, IR_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PT_EN_GPIO_Port, PT_EN_Pin, GPIO_PIN_SET);
}

void ADC2_Callback_Handle() {
	HAL_GPIO_WritePin(IR_EN_GPIO_Port, IR_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PT_EN_GPIO_Port, PT_EN_Pin, GPIO_PIN_RESET);

	sensor_L = (uint16_t) adc2_buffer[0];
	sensor_CL = (uint16_t) adc2_buffer[1];
	sensor_CR = (uint16_t) adc2_buffer[2];
	sensor_R = (uint16_t) adc2_buffer[3];

	// Calculate_Line_Position();
}

void Sensor_Start(){
	ADC2_Start();
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim15);
}


