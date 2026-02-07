/*
 * foc.c
 *
 * Created on: Nov 10, 2025
 * Updated for Dual Motor FOC
 */
#include "foc.h"
#include "math.h"
#include "tim.h" // htim1, htim8

// FOC 인스턴스 정의
FOC_Handle_t focL;
FOC_Handle_t focR;

// --- Helper Functions ---

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
    if (pid->integral > pid->out_max) pid->integral = pid->out_max;
    else if (pid->integral < pid->out_min) pid->integral = pid->out_min;

    float out = pid->Kp * err + pid->integral;

    // Output Saturation
    if (out > pid->out_max) out = pid->out_max;
    else if (out < pid->out_min) out = pid->out_min;

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
    if (Ub < Vmin) Vmin = Ub;
    if (Uc < Vmin) Vmin = Uc;
    if (Ub > Vmax) Vmax = Ub;
    if (Uc > Vmax) Vmax = Uc;

    // 3. VCOM(Zero Sequence Voltage) 계산
    // 파형을 전압 범위의 정가운데로 몰아주는 오프셋
    float Vcom = -0.5f * (Vmax + Vmin);

    // 4. Duty 계산 (VCOM을 더해서 SVPWM 구현)
    // (Ua + Vcom)은 -VBUS/2 ~ +VBUS/2 범위이므로,
    // VBUS/2를 더해 0 ~ VBUS 범위로 올린 뒤 비율을 계산합니다.
    Ta = (Ua + Vcom + VBUS/2.0f) / VBUS * PWM_PERIOD;
    Tb = (Ub + Vcom + VBUS/2.0f) / VBUS * PWM_PERIOD;
    Tc = (Uc + Vcom + VBUS/2.0f) / VBUS * PWM_PERIOD;

    // 5. Saturation (0 ~ PWM_PERIOD 제한)
    if(Ta < 0) Ta = 0; else if(Ta > PWM_PERIOD) Ta = PWM_PERIOD;
    if(Tb < 0) Tb = 0; else if(Tb > PWM_PERIOD) Tb = PWM_PERIOD;
    if(Tc < 0) Tc = 0; else if(Tc > PWM_PERIOD) Tc = PWM_PERIOD;

    // 6. Timer CCR 설정
    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->u_tim_channel, (uint32_t)Ta);
    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->v_tim_channel, (uint32_t)Tb);
    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->w_tim_channel, (uint32_t)Tc);
}
// --- Public Functions ---

void FOC_Init(FOC_Handle_t *hfoc, TIM_HandleTypeDef *htim, MT6701_Data_t *enc,
		uint16_t u_tim_channel, uint16_t v_tim_channel, uint16_t w_tim_channel) {
    // Hardware Linking
    hfoc->htim_pwm = htim;
    hfoc->u_tim_channel = u_tim_channel;
    hfoc->v_tim_channel = v_tim_channel;
    hfoc->w_tim_channel = w_tim_channel;

    hfoc->encoder = enc;
    hfoc->pole_pairs = POLE_PAIRS;
    hfoc->dir = 1;

    // PID Init
    hfoc->pid_d.Kp = 1.0f;  // 튜닝 필요
    hfoc->pid_d.Ki = 0.05f; // 튜닝 필요
    hfoc->pid_d.out_max = VBUS * 0.9f;
    hfoc->pid_d.out_min = -VBUS * 0.9f;

    hfoc->pid_q.Kp = 1.0f;  // 튜닝 필요
    hfoc->pid_q.Ki = 0.05f; // 튜닝 필요
    hfoc->pid_q.out_max = VBUS * 0.9f;
    hfoc->pid_q.out_min = -VBUS * 0.9f;
}

void FOC_Start(FOC_Handle_t *hfoc){
    HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->u_tim_channel);
    HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->v_tim_channel);
    HAL_TIM_PWM_Start(hfoc->htim_pwm, hfoc->w_tim_channel);

    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->u_tim_channel, 0);
    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->v_tim_channel, 0);
    __HAL_TIM_SET_COMPARE(hfoc->htim_pwm, hfoc->w_tim_channel, 0);
}

// 모터가 정지해 있을 때(0A)의 ADC 값을 읽어 오프셋으로 저장
void FOC_Calibrate_ADC_Offset(FOC_Handle_t *hfoc) {
    float sum_u = 0, sum_v = 0, sum_w = 0;
    int samples = 100;

    for(int i=0; i<samples; i++) {
        // 실제로는 여기서 ADC 값을 새로 읽어와야 함 (HAL_ADC_PollForConversion 등)
        // 현재는 외부 DMA 버퍼가 갱신된다고 가정
        sum_u += hfoc->adc_raw_u;
        sum_v += hfoc->adc_raw_v;
        sum_w += hfoc->adc_raw_w;
        HAL_Delay(1);
    }

    hfoc->offset_iu_adc = sum_u / samples;
    hfoc->offset_iv_adc = sum_v / samples;
    hfoc->offset_iw_adc = sum_w / samples;
}

void FOC_Set_Torque(FOC_Handle_t *hfoc, float iq_target) {
    hfoc->Iq_ref = iq_target;
}

// Main FOC Interrupt Handler (call at 10kHz~20kHz)
void FOC_Update(FOC_Handle_t *hfoc) {
    // 1. 전기각 계산 (Electrical Angle Calculation)
    // MT6701의 raw_angle(0~16383) 사용
    uint16_t raw = hfoc->encoder->raw_angle;
    // 기구적 각도(rad) 변환
    float theta_m = (raw / 16384.0f) * 2.0f * M_PI;
    // 전기각 변환
    hfoc->theta_e = normalize_angle(hfoc->pole_pairs * theta_m - hfoc->zero_offset_angle);

    if(hfoc->dir == -1) hfoc->theta_e = normalize_angle(2.0f * M_PI - hfoc->theta_e);

    // 2. 전류 측정 및 단위 변환 (ADC -> Ampere)
    // DRV8316: V_so = V_ref/2 + G * I
    // I = (V_adc - V_offset) / G
    float volts_per_count = ADC_REF_VOLT / ADC_RES;
    float V_adc_u = (hfoc->adc_raw_u - hfoc->offset_iu_adc) * volts_per_count;
    float V_adc_v = (hfoc->adc_raw_v - hfoc->offset_iv_adc) * volts_per_count;
    float V_adc_w = (hfoc->adc_raw_w - hfoc->offset_iw_adc) * volts_per_count; // 2션트 사용시 계산으로 대체 가능

    hfoc->Iu = V_adc_u / CSA_GAIN;
    hfoc->Iv = V_adc_v / CSA_GAIN;
    hfoc->Iw = V_adc_w / CSA_GAIN;

    // 3. Clarke Transform (abc -> alpha,beta)
    hfoc->I_alpha = hfoc->Iu;
    hfoc->I_beta  = (hfoc->Iv - hfoc->Iw) * SQRT3_INV;

    // 4. Park Transform (alpha,beta -> d,q)
    float s = sinf(hfoc->theta_e);
    float c = cosf(hfoc->theta_e);

    hfoc->Id =  hfoc->I_alpha * c + hfoc->I_beta * s;
    hfoc->Iq = -hfoc->I_alpha * s + hfoc->I_beta * c;

    // 5. PID Control
    // d축은 자속 제어 (일반적으로 0)
    float Vd = PI_Update(&hfoc->pid_d, hfoc->Id_ref, hfoc->Id);
    // q축은 토크 제어
    float Vq = PI_Update(&hfoc->pid_q, hfoc->Iq_ref, hfoc->Iq);

    // 6. Inverse Park Transform (d,q -> alpha,beta)
    float Valpha = Vd * c - Vq * s;
    float Vbeta  = Vd * s + Vq * c;

    // 7. SVPWM Output
    SVPWM_Generate(hfoc, Valpha, Vbeta);
}
