/*
 * vl53l4cx.c
 *
 *  Created on: Jan 17, 2026
 *      Author: kth59
 */
#include "main.h"
#include "i2c.h"
#include "vl53l4cx.h"
#include "tim.h"
#include "error.h"

#define VL53L4CX_I2C &hi2c1

VL53LX_Dev_t dev[VL53L4CX_NUM];
VL53LX_DEV vl53lx = dev;
int status;

uint8_t is_vl53lx_ready[VL53L4CX_NUM];

uint8_t byteData;
uint16_t wordData;
VL53LX_MultiRangingData_t MultiRangingData[VL53L4CX_NUM];
VL53LX_MultiRangingData_t *pMultiRangingData = MultiRangingData;
uint8_t NewDataReady = 0;
int no_of_object_found = 0, j;

HAL_StatusTypeDef VL53L4CX_Init(VL53LX_DEV Dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
		uint16_t new_address, uint8_t i) {
	Dev->I2cHandle = VL53L4CX_I2C;
	Dev->I2cDevAddr = VL53LX_SLAVE_ADDRESS_DEFAULT;

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	status = VL53LX_WaitDeviceBooted(Dev);
	if (status != 0) {
		sprintf(error_log, " %dToF Device boot fail", i);
		Error_Handler();
		return HAL_ERROR;
	}
	status = VL53LX_DataInit(Dev);
	if (status != 0) {
		sprintf(error_log, " %dToF Data init fail", i);
		Error_Handler();
		return HAL_ERROR;
	}

	VL53LX_SetDeviceAddress(Dev, new_address); // 예: 0x52 -> 0x56
	Dev->I2cDevAddr = new_address;
	return HAL_OK;
}

HAL_StatusTypeDef MX_VL53L4CX_Init() {
	VL53L4CX_Init(vl53lx + 0, IR_IN_XSHUT0_GPIO_Port, IR_IN_XSHUT0_Pin, VL53LX_SLAVE_ADDRESS_DEFAULT + 6, 1);
	VL53L4CX_Init(vl53lx + 1, IR_IN_XSHUT1_GPIO_Port, IR_IN_XSHUT1_Pin, VL53LX_SLAVE_ADDRESS_DEFAULT + 4, 2);
	VL53L4CX_Init(vl53lx + 2, IR_IN_XSHUT2_GPIO_Port, IR_IN_XSHUT2_Pin, VL53LX_SLAVE_ADDRESS_DEFAULT + 2, 3);
	VL53L4CX_Init(vl53lx + 3, IR_IN_XSHUT3_GPIO_Port, IR_IN_XSHUT3_Pin, VL53LX_SLAVE_ADDRESS_DEFAULT + 0, 4);

	return HAL_OK;
}

void VL53L4CX_Start() {
	for (uint8_t i = 0; i < VL53L4CX_NUM; i++) {
		status = VL53LX_StartMeasurement(vl53lx + i);
	}
}

