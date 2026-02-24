/*
 * motor.c
 *
 *  Created on: Feb 10, 2026
 *      Author: kth59
 */

#include "motor.h"
#include "main.h"
#include "st7789.h"

void Simple_6_step_Control(FOC_Handle_t *foc) {
	uint8_t ctrl2_val = 0;
	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);

	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X", ctrl2_val);
	if (ctrl2_val != 0x34) {
		HAL_GPIO_WritePin(MTR_INLx_GPIO_Port, MTR_INLx_Pin, GPIO_PIN_RESET);
		DRV8316C_UnlockRegister(foc->hdrv);
		DRV8316C_ApplyDefaultConfig(foc->hdrv);
		DRV8316C_LockRegister(foc->hdrv);
		HAL_GPIO_WritePin(MTR_INLx_GPIO_Port, MTR_INLx_Pin, GPIO_PIN_SET);
	}
	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);
	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X", ctrl2_val);

	static uint8_t step = 1;
	uint16_t pwm_val = foc->htim_pwm->Instance->ARR / 3 * 2; // 힘을 좀 더 강하게 (약 64%)

	// 1. 상태 모니터링
	int fault = HAL_GPIO_ReadPin(foc->hdrv->nFAULT_Port, foc->hdrv->nFAULT_Pin);

	LCD_Printf(0, 0, ST7789_WHITE, ST7789_BLACK, "MTR Check");
	LCD_Printf(0, 1, ST7789_WHITE, ST7789_BLACK, "FAULT:%d", fault);
	LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "Step:%d PWM:%d", step,
			pwm_val);

	// 2. 강제 3상 스텝 구동 (0.5초마다 이동)
	switch (step) {
	case 0: // [Step 1] 0°: U-High (V, W는 Low로 전류가 빠져나감)
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;

	case 1: // [Step 2] 60°: U-High, V-High
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;

	case 2: // [Step 3] 120°: V-High
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;

	case 3: // [Step 4] 180°: V-High, W-High
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, pwm_val);
		break;

	case 4: // [Step 5] 240°: W-High
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, pwm_val);
		break;

	case 5: // [Step 6] 300°: W-High, U-High
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, pwm_val);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, pwm_val);
		break;
	}

//	step = (step + 1) % 6;
}
