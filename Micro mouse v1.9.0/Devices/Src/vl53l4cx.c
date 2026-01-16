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

int status;
VL53LX_Dev_t dev;
VL53LX_DEV Dev = &dev;
volatile int IntCount;

uint8_t byteData;
uint16_t wordData;
VL53LX_MultiRangingData_t MultiRangingData;
VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
uint8_t NewDataReady = 0;
int no_of_object_found = 0, j;

void VL53L4CX_Init() {
	Dev->I2cHandle = &hi2c2;
	Dev->I2cDevAddr = 0x52;

	HAL_GPIO_WritePin(XSHUT0_GPIO_Port, XSHUT0_Pin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(XSHUT0_GPIO_Port, XSHUT0_Pin, GPIO_PIN_SET);
	HAL_Delay(5);

	VL53LX_SetDeviceAddress(Dev, 0x56); // 예: 0x52 -> 0x56
	Dev->I2cDevAddr = 0x56;

	VL53LX_RdByte(Dev, 0x010F, &byteData);
	LCD_Printf(0, 1, "Model_ID: %02X", byteData);
	VL53LX_RdByte(Dev, 0x0110, &byteData);
	LCD_Printf(0, 2, "Module_Type: %02X", byteData);
	VL53LX_RdWord(Dev, 0x010F, &wordData);
	LCD_Printf(0, 3, "VL53L4CX: %02X", wordData);

	status = VL53LX_WaitDeviceBooted(Dev);
	status = VL53LX_DataInit(Dev);
	status = VL53LX_StartMeasurement(Dev);

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == TOF_INT0_Pin){
		IntCount++;
	}
}

