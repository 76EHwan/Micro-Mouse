/*
 * motor.c
 *
 *  Created on: Feb 10, 2026
 *      Author: kth59
 */

#include "motor.h"
#include "adc.h"
#include "main.h"

#include "foc.h"
#include "mt6701.h"
#include "st7789.h"

#define MIN3(a,b,c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

void Simple_6_step_Control(FOC_Handle_t *foc) {
//	uint8_t ctrl2_val = 0;
//	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);
//	if (ctrl2_val != 0x34) {
//		TRIG_TOGGLE;
//		FOC_Stop(foc);
//		DRV8316C_UnlockRegister(foc->hdrv);
//		DRV8316C_ApplyDefaultConfig(foc->hdrv);
//		DRV8316C_LockRegister(foc->hdrv);
//		FOC_Start(foc);
//	}
//	DRV8316C_ReadRegister(foc->hdrv, DRV_REG_CTRL_2, &ctrl2_val);
//	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X", ctrl2_val);

	static uint16_t step = 1;
	uint16_t pwm_val = foc->htim_pwm->Instance->ARR; // 힘을 좀 더 강하게 (약 64%)
	float modulation_factor = 0.6;

//	LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "Step:%3d PWM:%4d", step,
//			pwm_val);

	// 2. 강제 3상 스텝 구동 (0.5초마다 이동)

	switch (step) {
	case 0:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;
	case 1:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;
	case 2:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, 0);
		break;
	case 3:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel,
				pwm_val * modulation_factor);
		break;
	case 4:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel,
				pwm_val * modulation_factor);
		break;
	case 5:
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel,
				pwm_val * modulation_factor);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, 0);
		__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel,
				pwm_val * modulation_factor);
		break;
	}

	step = (step + 1) % 6;
}

void Simple_SVPWM_Control(FOC_Handle_t *foc, float_t step) {
	uint16_t arr = foc->htim_pwm->Instance->ARR;

	// SVPWM에서는 최대 1.154 (2/√3) 까지 선형 변조가 가능하여 전압 이용률이 약 15% 상승합니다.
	float modulation_factor = 0.45f;

	uint16_t PWM_MAX = (uint16_t) (arr * 0.95f);  // ✅ 상한 클램핑 기준

	float rad_u = step * 3.141592f / 180.0f;
	float rad_v = fmodf((step + 120.0f), 360.0f) * 3.141592f / 180.0f;
	float rad_w = fmodf((step + 240.0f), 360.0f) * 3.141592f / 180.0f;

	// 1. 각 상의 기본 Sine 값 계산 (DC offset 제외, -0.5 ~ +0.5 범위)
	float u_val = 0.5f * sinf(rad_u) * modulation_factor;
	float v_val = 0.5f * sinf(rad_v) * modulation_factor;
	float w_val = 0.5f * sinf(rad_w) * modulation_factor;

	// 2. Min-Max Zero Sequence (공통 모드 전압) 계산 - SVPWM의 핵심
	float max_val = fmaxf(u_val, fmaxf(v_val, w_val));
	float min_val = fminf(u_val, fminf(v_val, w_val));
	float zero_seq = -(max_val + min_val) / 2.0f;

	// 3. Zero Sequence를 더하고 0.5(50% Duty) 오프셋을 적용하여 최종 PWM 계산
	uint16_t pwm_u = (uint16_t) (arr * (0.5f + u_val + zero_seq));
	uint16_t pwm_v = (uint16_t) (arr * (0.5f + v_val + zero_seq));
	uint16_t pwm_w = (uint16_t) (arr * (0.5f + w_val + zero_seq));

	// ✅ 상한 클램핑 (기존 로직 유지)
	pwm_u = (pwm_u > PWM_MAX) ? PWM_MAX : pwm_u;
	pwm_v = (pwm_v > PWM_MAX) ? PWM_MAX : pwm_v;
	pwm_w = (pwm_w > PWM_MAX) ? PWM_MAX : pwm_w;

	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->u_tim_channel, pwm_u);
	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->v_tim_channel, pwm_v);
	__HAL_TIM_SET_COMPARE(foc->htim_pwm, foc->w_tim_channel, pwm_w);
}

void Motor_Start() {
	HAL_TIM_Base_Start_IT(&htim7);
	ADC2_Start();
	ADC1_Start();

	FOC_Start(&focR);
	__HAL_TIM_SET_COUNTER(&htim8, PWM_PERIOD);

	FOC_Start(&focL);
}

void Calc_PI_Iqref(FOC_Handle_t *hfoc) {

}
