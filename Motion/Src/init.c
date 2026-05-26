/*
 * init.c
 *
 *  Created on: Jan 31, 2026
 *      Author: kth59
 */
#include "main.h"
#include "lptim.h"

#include "init.h"
#include "error.h"

#include "lsm6ds3tr-c.h"
#include "mt6701.h"
#include "vl53l4cx.h"
#include "st7789.h"
#include "foc.h"

#define IMU_EN
#define DRV8316C_L_EN
#define DRV8316C_R_EN
//#define ENCODER_L_EN
//#define ENCODER_R_EN
//#define SENSOR_TOF_EN
#define FOC_EN

void DWT_Init(void) {
    // 1. DWT 유닛 활성화 (CoreDebug->DEMCR)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 2. 사이클 카운터 값 초기화
    DWT->CYCCNT = 0;

    // 3. 사이클 카운터 시작 (DWT->CTRL)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void MX_User_Init() {
	DWT_Init();
	ST7789_Init();
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
	uint8_t rxBufferL[3] = {0, 0, 0};
	MT6701_Init(&encDataL, rxBufferL);

	if (encDataL.status != HAL_OK) {
	    // %02X를 사용하여 0을 채우고, rxBufferL[0]부터 순서대로 출력
	    sprintf(error_log, " L: %02X  %02X%02X%02X", encDataL.status, rxBufferL[0], rxBufferL[1], rxBufferL[2]);
	    Error_Handler();
	}
#endif
#ifdef ENCODER_R_EN
	uint8_t rxBufferR[3] = {0,0,0};
		MT6701_Init(&encDataR, rxBufferR);
		if (encDataR.status != HAL_OK) {
			sprintf(error_log, " L: %X", encDataR.status);
			Error_Handler();
		}
#endif

#ifdef SENSOR_TOF_EN
	if (MX_VL53L4CX_Init() != HAL_OK) {
		sprintf(error_log, " ToF Init ERROR!");
		Error_Handler();
	}
#endif

#ifdef FOC_EN
	FOC_Init(&focL, &htim1, &encDataL, &DRV8316C_L, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3);
	FOC_Init(&focR, &htim8, &encDataR, &DRV8316C_R, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3);
#endif
}

void IMU_Start() {
	Gyro_Calibrate_Z_Only();
	ST7789_FillScreen(ST7789_BLACK);
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start(&htim2);
}
