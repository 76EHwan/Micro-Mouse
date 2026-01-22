/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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
#include "vl53l4cx.h"
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PH0-OSC_IN(PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT(PH1)   ------> RCC_OSC_OUT
     PA13(JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14(JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LD2_Pin|XSHUT3_Pin|XSHUT2_Pin|MTR_INLx_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_CS_Pin|LCD_DC_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MTR_R_DRVOFF_Pin|MTR_L_DRVOFF_Pin|MTR_nSLEEP_Pin|MTR_R_CS_Pin
                          |MTR_L_CS_Pin|ENC_L_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, XSHUT1_Pin|XSHUT0_Pin|ENC_R_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LD2_Pin LCD_CS_Pin LCD_DC_Pin MTR_INLx_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LCD_CS_Pin|LCD_DC_Pin|MTR_INLx_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : MTR_R_DRVOFF_Pin MTR_L_DRVOFF_Pin MTR_nSLEEP_Pin MTR_R_CS_Pin
                           MTR_L_CS_Pin ENC_L_CS_Pin */
  GPIO_InitStruct.Pin = MTR_R_DRVOFF_Pin|MTR_L_DRVOFF_Pin|MTR_nSLEEP_Pin|MTR_R_CS_Pin
                          |MTR_L_CS_Pin|ENC_L_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MTR_R_nFAULT_Pin MTR_L_nFAULT_Pin */
  GPIO_InitStruct.Pin = MTR_R_nFAULT_Pin|MTR_L_nFAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : XSHUT3_Pin XSHUT2_Pin */
  GPIO_InitStruct.Pin = XSHUT3_Pin|XSHUT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : XSHUT1_Pin XSHUT0_Pin */
  GPIO_InitStruct.Pin = XSHUT1_Pin|XSHUT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_R_CS_Pin */
  GPIO_InitStruct.Pin = ENC_R_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ENC_R_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TOF_INT3_Pin TOF_INT2_Pin TOF_INT1_Pin */
  GPIO_InitStruct.Pin = TOF_INT3_Pin|TOF_INT2_Pin|TOF_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TOF_INT0_Pin */
  GPIO_InitStruct.Pin = TOF_INT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TOF_INT0_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI10_IRQn);

  HAL_NVIC_SetPriority(EXTI11_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI11_IRQn);

  HAL_NVIC_SetPriority(EXTI12_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI12_IRQn);

}

/* USER CODE BEGIN 2 */

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
//	if (GPIO_Pin == TOF_INT0_Pin) {
//		is_vl53lx_ready[0] = 1;
//	}
//	else if (GPIO_Pin == TOF_INT1_Pin) {
//		is_vl53lx_ready[1] = 1;
//	}
//	else if (GPIO_Pin == TOF_INT2_Pin) {
//		is_vl53lx_ready[2] = 1;
//	}
	if (GPIO_Pin == TOF_INT3_Pin) {
		is_vl53lx_ready[3] = 1;
		TRIG_TOGGLE;
	}
}
/* USER CODE END 2 */
