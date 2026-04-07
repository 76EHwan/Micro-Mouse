/*
 * foc.c
 *
 * Created on: Nov 10, 2025
 * Updated for Dual Motor FOC
 */
#include "foc.h"
#include "math.h"
#include "adc.h"
#include "tim.h" // htim1, htim8

#define clamp(v, min, max) ((v) < (min) ? (min) : ((v) > (max) ? (max) : (v)))

// FOC 인스턴스 정의
FOC_Handle_t focL;
FOC_Handle_t focR;

// --- Helper Functions ---

// ADC DMA 결과 업데이트
void Calc_SOX_Current(FOC_Handle_t *hfoc, volatile uint16_t *pData){
	hfoc->vBus = (float)*(pData + 0) * ADC_REF_VOLT * ADC_RES_INV;
	hfoc->adc_raw_u = *(pData + 1);
	hfoc->adc_raw_v = *(pData + 2);
}

// 0~2PI 범위로 각도 정규화
static inline float normalize_angle(float angle) {
	float a = fmodf(angle, 2.0f * M_PI);
	return (a < 0.0f) ? (a + 2.0f * M_PI) : a;
}

// PI 제어기 업데이트
float PI_Update(PI_Controller *pid, float ref, float feedback) {
	float err = ref - feedback;
	pid->integral += pid->Ki * err;

	// Anti-windup
	if (pid->integral > pid->out_max)
		pid->integral = pid->out_max;
	else if (pid->integral < pid->out_min)
		pid->integral = pid->out_min;

	float out = pid->Kp * err + pid->integral;

	// Output Saturation
	if (out > pid->out_max)
		out = pid->out_max;
	else if (out < pid->out_min)
		out = pid->out_min;

	return out;
}

// SVPWM 생성 및 타이머 레지스터 설정
/* --- SVPWM generation --- */
void SVPWM_Generate(FOC_Handle_t *hfoc, float Valpha, float Vbeta) {
	float Ta, Tb, Tc;

	// 1. 역 Clarke 변환 (2축 -> 3축)
	float Ua = Valpha;
	float Ub = -0.5f * Valpha + SQRT3_INV * Vbeta;
	float Uc = -0.5f * Valpha - SQRT3_INV * Vbeta;

	// 2. Min, Max 값 찾기
	float Vmin = Ua, Vmax = Ua;
	if (Ub < Vmin)
		Vmin = Ub;
	if (Uc < Vmin)
		Vmin = Uc;
	if (Ub > Vmax)
		Vmax = Ub;
	if (Uc > Vmax)
		Vmax = Uc;

	// 3. VCOM(Zero Sequence Voltage) 계산
	// 파형을 전압 범위의 정가운데로 몰아주는 오프셋
	float Vcom = -0.5f * (Vmax + Vmin);

	// 4. Duty 계산 (VCOM을 더해서 SVPWM 구현)
	// (Ua + Vcom)은 -VBUS/2 ~ +VBUS/2 범위이므로,
	// VBUS/2를 더해 0 ~ VBUS 범위로 올린 뒤 비율을 계산합니다.
	Ta = (Ua + Vcom + hfoc->vBus) / hfoc->vBus * PWM_PERIOD_HALF;
	Tb = (Ub + Vcom + hfoc->vBus) / hfoc->vBus * PWM_PERIOD_HALF;
	Tc = (Uc + Vcom + hfoc->vBus) / hfoc->vBus * PWM_PERIOD_HALF;

	// 5. Saturation (0 ~ PWM_PERIOD 제한)
	Ta = clamp(Ta, 0, PWM_PERIOD);
	Tb = clamp(Tb, 0, PWM_PERIOD);
	Tc = clamp(Tc, 0, PWM_PERIOD);

	// 6. Timer CCR 설정
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->u_tim_channel, (uint32_t )Ta);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->v_tim_channel, (uint32_t )Tb);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->w_tim_channel, (uint32_t )Tc);
}
// --- Public Functions ---

void FOC_Init(FOC_Handle_t *hfoc, TIM_HandleTypeDef *htim, MT6701_Data_t *enc,
		DRV8316C_Handle_t *hdrv, uint16_t u_tim_channel, uint16_t v_tim_channel,
		uint16_t w_tim_channel) {
	// Hardware Linking
	hfoc->htim_pwm = htim;
	hfoc->u_tim_channel = u_tim_channel;
	hfoc->v_tim_channel = v_tim_channel;
	hfoc->w_tim_channel = w_tim_channel;

	hfoc->encoder = enc;
	hfoc->hdrv = hdrv;

	hfoc->pole_pairs = POLE_PAIRS;
	hfoc->dir = 1;

	// PID Init

	hfoc->pid_d.Kp = FOC_CURRENT_KP;  // 튜닝 필요
	hfoc->pid_d.Ki = FOC_CURRENT_KI; // 튜닝 필요

	hfoc->pid_q.Kp = FOC_CURRENT_KP;  // 튜닝 필요
	hfoc->pid_q.Ki = FOC_CURRENT_KI; // 튜닝 필요
	hfoc->pid_q.out_max = VBUS * 0.9f;
	hfoc->pid_q.out_min = -VBUS * 0.9f;
}

void FOC_Start(FOC_Handle_t *hfoc) {
	HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->u_tim_channel);
	HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->v_tim_channel);
	HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->w_tim_channel);

	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->u_tim_channel, 0);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->v_tim_channel, 0);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->w_tim_channel, 0);
	HAL_GPIO_WritePin(MTR_INLX_GPIO_Port, MTR_INLX_Pin, GPIO_PIN_SET);
}

void FOC_Stop(FOC_Handle_t *hfoc) {
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->u_tim_channel, 0);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->v_tim_channel, 0);
	__HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->w_tim_channel, 0);

	HAL_TIM_PWM_Stop(hfoc->htim_pwm, hfoc->u_tim_channel);
	HAL_TIM_PWM_Stop(hfoc->htim_pwm, hfoc->v_tim_channel);
	HAL_TIM_PWM_Stop(hfoc->htim_pwm, hfoc->w_tim_channel);
	HAL_GPIO_WritePin(MTR_INLX_GPIO_Port, MTR_INLX_Pin, GPIO_PIN_RESET);
}

void FOC_Set_Torque(FOC_Handle_t *hfoc, float iq_target) {
	hfoc->Iq_ref = iq_target;
}

// Main FOC Interrupt Handler (call at 10kHz~20kHz)
void FOC_Update(FOC_Handle_t *hfoc) {
	// 1. 전기각 계산 (Electrical Angle Calculation)
	// MT6701의 motor_elec_angle 사용
	hfoc->theta_e = hfoc->encoder->motor_elec_angle;

	if (hfoc->dir == -1)
		hfoc->theta_e = normalize_angle(2.0f * M_PI - hfoc->theta_e);

	// 2. 전류 측정 및 단위 변환 (ADC -> Ampere)
	// DRV8316: V_so = V_ref/2 + G * I
	// I = (V_adc - V_offset) / G
	float volts_per_count = ADC_REF_VOLT * ADC_RES_INV;
	float V_adc_u = (hfoc->adc_raw_u - 2048) * volts_per_count;
	float V_adc_v = (hfoc->adc_raw_v - 2048) * volts_per_count;

	hfoc->Iu = V_adc_u * CSA_GAIN_INV;
	hfoc->Iv = V_adc_v * CSA_GAIN_INV;

	// 3. Clarke Transform (abc -> alpha,beta)
	hfoc->I_alpha = hfoc->Iu;
	hfoc->I_beta = (2.f * hfoc->Iv + hfoc->Iu) * SQRT3_INV;

	// 4. Park Transform (alpha,beta -> d,q)
	float s = sinf(hfoc->theta_e);
	float c = cosf(hfoc->theta_e);

	hfoc->Id = hfoc->I_alpha * c + hfoc->I_beta * s;
	hfoc->Iq = -hfoc->I_alpha * s + hfoc->I_beta * c;

	// 5. PID Control
	// d축은 자속 제어 (일반적으로 0)
	hfoc->Vd = PI_Update(&hfoc->pid_d, hfoc->Id_ref, hfoc->Id);
	// q축은 토크 제어
	hfoc->Vq = PI_Update(&hfoc->pid_q, hfoc->Iq_ref, hfoc->Iq);

	// 6. Inverse Park Transform (d,q -> alpha,beta)
	hfoc->Valpha = hfoc->Vd * c - hfoc->Vq * s;
	hfoc->Vbeta = hfoc->Vd * s + hfoc->Vq * c;
}

void ADC1_Callback_Handle(){
	Calc_SOX_Current(&focL, adc1_buffer);
	MT6701_ReadSSI(focL.encoder);
	FOC_Update(&focL);
	SVPWM_Generate(&focL, focL.Valpha, focL.Vbeta);
}

void ADC2_Callback_Handle(){
	Calc_SOX_Current(&focR, adc2_buffer);
	MT6701_ReadSSI(focR.encoder);
	FOC_Update(&focR);
	SVPWM_Generate(&focR, focR.Valpha, focR.Vbeta);
}
