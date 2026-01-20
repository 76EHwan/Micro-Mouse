/*
 * vl53l4cx.c
 *
 *  Created on: Jan 17, 2026
 *      Author: kth59
 */
#include "main.h"
#include "i2c.h"
#include "vl53l4cx.h"
#include "lcd.h"
#include "tim.h"

#define VL53L4CX_I2C &hi2c2

VL53LX_Dev_t dev;
VL53LX_DEV vl53lx = &dev;
int status;

volatile uint8_t is_vl53lx_ready[VL53L4CX_NUM];

uint8_t byteData;
uint16_t wordData;
VL53LX_MultiRangingData_t MultiRangingData;
VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
uint8_t NewDataReady = 0;
int no_of_object_found = 0, j;

void VL53L4CX_Init(VL53LX_DEV Dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
		uint16_t new_address) {
	Dev->I2cHandle = VL53L4CX_I2C;
	Dev->I2cDevAddr = VL53LX_SLAVE_ADDRESS_DEFAULT;

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(2);
	status = VL53LX_WaitDeviceBooted(Dev);
	status = VL53LX_DataInit(Dev);

	VL53LX_SetDeviceAddress(Dev, new_address); // 예: 0x52 -> 0x56
	Dev->I2cDevAddr = new_address;
}

void MX_VL53L4CX_Init() {
	HAL_GPIO_WritePin(XSHUT0_GPIO_Port, XSHUT0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT1_GPIO_Port, XSHUT1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT2_GPIO_Port, XSHUT2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(XSHUT3_GPIO_Port, XSHUT3_Pin, GPIO_PIN_RESET);

	VL53L4CX_Init(vl53lx + 0, XSHUT0_GPIO_Port, XSHUT0_Pin,
	VL53LX_SLAVE_ADDRESS_DEFAULT + 0x08);
	VL53L4CX_Init(vl53lx + 1, XSHUT1_GPIO_Port, XSHUT1_Pin,
	VL53LX_SLAVE_ADDRESS_DEFAULT + 0x06);
	VL53L4CX_Init(vl53lx + 2, XSHUT2_GPIO_Port, XSHUT2_Pin,
	VL53LX_SLAVE_ADDRESS_DEFAULT + 0x04);
	VL53L4CX_Init(vl53lx + 3, XSHUT3_GPIO_Port, XSHUT3_Pin,
	VL53LX_SLAVE_ADDRESS_DEFAULT + 0x02);
}

void VL53L4CX_Start() {
	for (uint8_t i = 0; i < VL53L4CX_NUM; i++) {
		status = VL53LX_StartMeasurement(vl53lx + i);
	}
	HAL_TIM_Base_Start_IT(&htim3);
}

