/*
 * init.c
 *
 *  Created on: Jan 31, 2026
 *      Author: kth59
 */
#include "main.h"
#include "tim.h"

#include "init.h"
#include "error.h"

#include "drv8316crq1.h"
#include "lsm6ds3tr-c.h"
#include "mt6701.h"
#include "vl53l4cx.h"
#include "st7789.h"

#define IMU_EN
#define DRV8316C_L_EN
#define DRV8316C_R_EN
//#define ENCODER_L_EN
//#define ENCODER_R_EN
//#define SENSOR_TOF_EN

void MX_User_Init() {
	ST7789_Init();
	ST7789_FillScreen(ST7789_BLACK);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 200);

	HAL_Delay(10);

#ifdef IMU_EN
	if (LSM6DS3_Init() != HAL_OK) {
		sprintf(error_log, " IMU ERROR!");
		Error_Handler();
	}
#endif

	DRV8316C_REG_Typedef reg_status;
#ifdef DRV8316C_L_EN
	HAL_GPIO_WritePin(MTR_nSLEEP_GPIO_Port, MTR_nSLEEP_Pin, GPIO_PIN_SET);
	DRV8316C_Init(&DRV8316C_L, DRV8316C_SPI, MTR_L_CS_GPIO_Port, MTR_L_CS_Pin,
	MTR_L_nFAULT_GPIO_Port, MTR_L_nFAULT_Pin, MTR_L_DRVOFF_GPIO_Port,
	MTR_L_DRVOFF_Pin);
	HAL_Delay(10);
	if (DRV8316C_UnlockRegister(&DRV8316C_L) != HAL_OK) {
		sprintf(error_log, " DRV8316C LEFT UNLOCK ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if (DRV8316C_ApplyDefaultConfig(&DRV8316C_L) != HAL_OK) {
		sprintf(error_log, " DRV8316C LEFT CONFIG ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if (DRV8316C_LockRegister(&DRV8316C_L) != HAL_OK) {
		sprintf(error_log, " DRV8316C LEFT LOCK ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if ((reg_status = DRV8316C_VerifyConfig(&DRV8316C_L)) != REG_OK) {
		sprintf(error_log, "DRV8316C LEFT CONFIG %d!", reg_status);
	}
#endif

#ifdef DRV8316C_R_EN
	HAL_GPIO_WritePin(MTR_nSLEEP_GPIO_Port, MTR_nSLEEP_Pin, GPIO_PIN_SET);
	DRV8316C_Init(&DRV8316C_R, DRV8316C_SPI, MTR_R_CS_GPIO_Port, MTR_R_CS_Pin,
	MTR_R_nFAULT_GPIO_Port, MTR_R_nFAULT_Pin, MTR_R_DRVOFF_GPIO_Port,
	MTR_R_DRVOFF_Pin);
	HAL_Delay(10);
	if (DRV8316C_UnlockRegister(&DRV8316C_R) != HAL_OK) {
		sprintf(error_log, " DRV8316C RIGHT UNLOCK ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if (DRV8316C_ApplyDefaultConfig(&DRV8316C_R) != HAL_OK) {
		sprintf(error_log, " DRV8316C RIGHT CONFIG ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if (DRV8316C_LockRegister(&DRV8316C_R) != HAL_OK) {
		sprintf(error_log, " DRV8316C RIGHT LOCK ERROR!");
		Error_Handler();
	}
	HAL_Delay(10);
	if ((reg_status = DRV8316C_VerifyConfig(&DRV8316C_R)) != REG_OK) {
		sprintf(error_log, "DRV8316C RIGHT CONFIG %d!", reg_status);
	}
#endif

#ifdef ENCODER_L_EN
	if (MT6701_Init(&encDataL) != HAL_OK) {
		sprintf(error_log, " Encoder LEFT ERROR!");
		Error_Handler();
	}
#endif
#ifdef ENCODER_R_EN
	if (MT6701_Init(&encDataR) != HAL_OK) {
		sprintf(error_log, " Encoder RIGHT ERROR!");
		Error_Handler();
	}
#endif

#ifdef SENSOR_TOF_EN
	if (MX_VL53L4CX_Init() != HAL_OK) {
		sprintf(error_log, " ToF Init ERROR!");
		Error_Handler();
	}
#endif
}

void IMU_Start() {
	Gyro_Calibrate_Z_Only();
	ST7789_FillScreen(ST7789_BLACK);
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start(&htim2);
}
