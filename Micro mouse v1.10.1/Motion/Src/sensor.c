/*
 * sensor.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#include "sensor.h"
#include "adc.h"

#include "foc.h"

#include "lsm6ds3tr-c.h"
#include "vl53l4cx.h"

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

void Sensor_Start() {
	VL53L4CX_Start();
}

void Sensor_Get_Dist() {
	for (uint8_t i = 0; i < VL53L4CX_NUM; i++) {
		uint8_t dataReady = 0;

		// 1. 데이터가 준비되었는지 확인 (비차단 방식)
		HAL_StatusTypeDef status = VL53LX_GetMeasurementDataReady(vl53lx + i,
				&dataReady);

		if ((status == 0) && (dataReady == 1)) {
			// 2. 준비되었다면 데이터 읽어오기 (MultiRangingData 변수 업데이트)
			VL53LX_GetMultiRangingData(vl53lx + i, &MultiRangingData[i]);

			// 3. 인터럽트 클리어 (이걸 해야 다음 측정이 시작됩니다!)
			VL53LX_ClearInterruptAndStartMeasurement(vl53lx + i);
		}
	}
	sensor_L = (pMultiRangingData + 0)->RangeData[0].RangeMilliMeter;
	sensor_CL = (pMultiRangingData + 1)->RangeData[0].RangeMilliMeter;
	sensor_CR = (pMultiRangingData + 2)->RangeData[0].RangeMilliMeter;
	sensor_R = (pMultiRangingData + 3)->RangeData[0].RangeMilliMeter;
}
