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


void Sensor_Start(){

}


