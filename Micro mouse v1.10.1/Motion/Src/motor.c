/*
 * motor.c
 *
 *  Created on: Feb 10, 2026
 *      Author: kth59
 */

#include "motor.h"
#include "main.h"
#include "st7789.h"

#define MIN3(a,b,c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

void Simple_6_step_Control(FOC_Handle_t *foc) {
	uint8_t ctrl2_val = 0;
	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);

//	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X", ctrl2_val);
	if (ctrl2_val != 0x34) {
		FOC_Stop(foc);
		DRV8316C_UnlockRegister(foc->hdrv);
		DRV8316C_ApplyDefaultConfig(foc->hdrv);
		DRV8316C_LockRegister(foc->hdrv);
		FOC_Start(foc);
	}
	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);
	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X", ctrl2_val);

	static uint16_t step = 0;
	uint16_t pwm_val = foc->htim_pwm->Instance->ARR; // 힘을 좀 더 강하게 (약 64%)
	float modulation_factor = 0.16;

// 1. 상태 모니터링
//	uint32_t ccer = foc->htim_pwm->Instance->CCER;
//	uint32_t ccr1 = foc->htim_pwm->Instance->CCR1;

	// CCER이 0이면 안 됩니다! (보통 0x111 같은 값이어야 함)
//	LCD_Printf(0, 1, ST7789_WHITE, ST7789_BLACK, "CCER:%3x CCR:%4d", ccer,
//			ccr1);
//
	LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "Step:%3d PWM:%4d", step,
			pwm_val);

	// 2. 강제 3상 스텝 구동 (0.5초마다 이동)

	float rad_u = step * M_PI / 180.f;
	float rad_v = (step + 120) * M_PI / 180.f;
	float rad_w = (step + 240) * M_PI / 180.f;

	uint16_t pwm_u = pwm_val * ((0.5 + 0.5 * sinf(rad_u) * modulation_factor));
	uint16_t pwm_v = pwm_val * ((0.5 + 0.5 * sinf(rad_v) * modulation_factor));
	uint16_t pwm_w = pwm_val * ((0.5 + 0.5 * sinf(rad_w) * modulation_factor));

	uint16_t min_pwm = MIN3(pwm_u, pwm_v, pwm_w);

	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, pwm_u - min_pwm);
	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, pwm_v - min_pwm);
	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, pwm_w - min_pwm);

	step = (step + 1) % 360;
}
