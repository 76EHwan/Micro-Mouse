/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define TRIG_ON		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET)
#define TRIG_OFF	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)
#define TRIG_TOGGLE HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LD2_Pin GPIO_PIN_13
#define LD2_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_14
#define LCD_CS_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_15
#define LCD_DC_GPIO_Port GPIOC
#define SOA_R_Pin GPIO_PIN_0
#define SOA_R_GPIO_Port GPIOC
#define SOB_R_Pin GPIO_PIN_1
#define SOB_R_GPIO_Port GPIOC
#define SOC_R_Pin GPIO_PIN_2
#define SOC_R_GPIO_Port GPIOC
#define ADC_BAT_Pin GPIO_PIN_3
#define ADC_BAT_GPIO_Port GPIOC
#define SOA_L_Pin GPIO_PIN_0
#define SOA_L_GPIO_Port GPIOA
#define SOB_L_Pin GPIO_PIN_1
#define SOB_L_GPIO_Port GPIOA
#define SOC_L_Pin GPIO_PIN_2
#define SOC_L_GPIO_Port GPIOA
#define MTR_R_DRVOFF_Pin GPIO_PIN_3
#define MTR_R_DRVOFF_GPIO_Port GPIOA
#define MTR_L_DRVOFF_Pin GPIO_PIN_4
#define MTR_L_DRVOFF_GPIO_Port GPIOA
#define MTR_R_nFAULT_Pin GPIO_PIN_5
#define MTR_R_nFAULT_GPIO_Port GPIOA
#define MTR_L_nFAULT_Pin GPIO_PIN_6
#define MTR_L_nFAULT_GPIO_Port GPIOA
#define MTR_nSLEEP_Pin GPIO_PIN_7
#define MTR_nSLEEP_GPIO_Port GPIOA
#define XSHUT3_Pin GPIO_PIN_4
#define XSHUT3_GPIO_Port GPIOC
#define XSHUT2_Pin GPIO_PIN_5
#define XSHUT2_GPIO_Port GPIOC
#define XSHUT1_Pin GPIO_PIN_0
#define XSHUT1_GPIO_Port GPIOB
#define XSHUT0_Pin GPIO_PIN_1
#define XSHUT0_GPIO_Port GPIOB
#define FAN_Pin GPIO_PIN_2
#define FAN_GPIO_Port GPIOB
#define TOF_SCL_Pin GPIO_PIN_10
#define TOF_SCL_GPIO_Port GPIOB
#define TOF_SDA_Pin GPIO_PIN_12
#define TOF_SDA_GPIO_Port GPIOB
#define ENC_SCK_Pin GPIO_PIN_13
#define ENC_SCK_GPIO_Port GPIOB
#define ENC_MISO_Pin GPIO_PIN_14
#define ENC_MISO_GPIO_Port GPIOB
#define ENC_R_CS_Pin GPIO_PIN_15
#define ENC_R_CS_GPIO_Port GPIOB
#define MTR_R_INH2_Pin GPIO_PIN_6
#define MTR_R_INH2_GPIO_Port GPIOC
#define MTR_R_INH2C7_Pin GPIO_PIN_7
#define MTR_R_INH2C7_GPIO_Port GPIOC
#define MTR_R_INH3_Pin GPIO_PIN_8
#define MTR_R_INH3_GPIO_Port GPIOC
#define MTR_INLx_Pin GPIO_PIN_9
#define MTR_INLx_GPIO_Port GPIOC
#define MTR_L_INH1_Pin GPIO_PIN_8
#define MTR_L_INH1_GPIO_Port GPIOA
#define MTR_L_INH2_Pin GPIO_PIN_9
#define MTR_L_INH2_GPIO_Port GPIOA
#define MTR_L_INH3_Pin GPIO_PIN_10
#define MTR_L_INH3_GPIO_Port GPIOA
#define MTR_R_CS_Pin GPIO_PIN_11
#define MTR_R_CS_GPIO_Port GPIOA
#define MTR_L_CS_Pin GPIO_PIN_12
#define MTR_L_CS_GPIO_Port GPIOA
#define ENC_L_CS_Pin GPIO_PIN_15
#define ENC_L_CS_GPIO_Port GPIOA
#define TOF_INT3_Pin GPIO_PIN_10
#define TOF_INT3_GPIO_Port GPIOC
#define TOF_INT3_EXTI_IRQn EXTI10_IRQn
#define TOF_INT2_Pin GPIO_PIN_11
#define TOF_INT2_GPIO_Port GPIOC
#define TOF_INT2_EXTI_IRQn EXTI11_IRQn
#define TOF_INT1_Pin GPIO_PIN_12
#define TOF_INT1_GPIO_Port GPIOC
#define TOF_INT1_EXTI_IRQn EXTI12_IRQn
#define TOF_INT0_Pin GPIO_PIN_2
#define TOF_INT0_GPIO_Port GPIOD
#define TOF_INT0_EXTI_IRQn EXTI2_IRQn
#define IMU_SCL_Pin GPIO_PIN_6
#define IMU_SCL_GPIO_Port GPIOB
#define IMU_SDA_Pin GPIO_PIN_7
#define IMU_SDA_GPIO_Port GPIOB
#define LCD_BK_Pin GPIO_PIN_8
#define LCD_BK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
