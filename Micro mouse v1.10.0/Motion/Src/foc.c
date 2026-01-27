/*
 * foc.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */
#include "main.h"
#include "math.h"
#include "tim.h"
#include "foc.h"

float Iu, Iv, Iw;
float Id, Iq;
float Id_ref = 0.0f, Iq_ref = 0.5f;  // torque command
float theta_e = 0.0f;                // rotor electrical angle

PI_Controller pid_d = { .Kp = 0.5f, .Ki = 0.01f, .out_max = 1.0f };
PI_Controller pid_q = { .Kp = 0.5f, .Ki = 0.01f, .out_max = 1.0f };

float PI_Update(PI_Controller *pid, float ref, float feedback) {
    float err = ref - feedback;
    pid->integral += pid->Ki * err;
    if (pid->integral > pid->out_max) pid->integral = pid->out_max;
    if (pid->integral < -pid->out_max) pid->integral = -pid->out_max;
    float out = pid->Kp * err + pid->integral;
    if (out > pid->out_max) out = pid->out_max;
    if (out < -pid->out_max) out = -pid->out_max;
    return out;
}

/* --- Clarke transform --- */
static inline void Clarke(float Iu, float Iv, float *Ialpha, float *Ibeta) {
    *Ialpha = Iu;
    *Ibeta  = (Iu + 2 * Iv) * SQRT3_INV;
}

/* --- Park transform --- */
static inline void Park(float Ialpha, float Ibeta, float theta, float *Id, float *Iq) {
    float sin_t = sinf(theta);
    float cos_t = cosf(theta);
    *Id = Ialpha * cos_t + Ibeta * sin_t;
    *Iq = -Ialpha * sin_t + Ibeta * cos_t;
}

/* --- Inverse Park --- */
static inline void InvPark(float Vd, float Vq, float theta, float *Valpha, float *Vbeta) {
    float sin_t = sinf(theta);
    float cos_t = cosf(theta);
    *Valpha = Vd * cos_t - Vq * sin_t;
    *Vbeta  = Vd * sin_t + Vq * cos_t;
}

/* --- SVPWM generation --- */
void SVPWM_Generate(float Valpha, float Vbeta) {
    float Ta, Tb, Tc;
    float Vref = (2.0f / VBUS) * PWM_PERIOD;

    Ta = (Valpha + VBUS / 2) * Vref;
    Tb = ((-0.5f * Valpha) + (SQRT3_INV * Vbeta) + VBUS / 2) * Vref;
    Tc = ((-0.5f * Valpha) - (SQRT3_INV * Vbeta) + VBUS / 2) * Vref;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Ta);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Tb);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Tc);
}

/* --- Main FOC loop --- */
void FOC_Loop(void) {
    float Ialpha, Ibeta;
    float Vd, Vq;
    float Valpha, Vbeta;

    // 1. ADC로 전류 측정 완료 후 값 읽기
    // Iu, Iv는 ADC 변환 값 → 전류 변환

    // 2. Clarke & Park
    Clarke(Iu, Iv, &Ialpha, &Ibeta);
    Park(Ialpha, Ibeta, theta_e, &Id, &Iq);

    // 3. PI 컨트롤러
    Vd = PI_Update(&pid_d, Id_ref, Id);
    Vq = PI_Update(&pid_q, Iq_ref, Iq);

    // 4. 역 Park 변환
    InvPark(Vd, Vq, theta_e, &Valpha, &Vbeta);

    // 5. SVPWM
    SVPWM_Generate(Valpha, Vbeta);
}
