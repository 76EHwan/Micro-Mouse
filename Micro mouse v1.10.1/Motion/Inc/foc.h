/*
 * foc.h
 *
 * Created on: Nov 10, 2025
 * Updated for Dual Motor FOC
 */

#ifndef FOC_H_
#define FOC_H_

#include "main.h"
#include "mt6701.h"
#include "drv8316crq1.h"

// --- Constants ---
#define PWM_PERIOD      1000    // Timer Period (ARR + 1), 예: 1000 (TIM1/8 설정에 맞춤)
#define VBUS            7.4f    // Battery Voltage
#define SQRT3_INV       0.577350269f // 1/sqrt(3)
#define POLE_PAIRS      1       // Pole pairs (데이터시트 확인 필요)

// DRV8316 Current Sense Gain (Default 0.6 V/A)
#define CSA_GAIN        0.6f
#define ADC_REF_VOLT    3.3f
#define ADC_RES         4096.0f

// --- PI Controller Struct ---
typedef struct {
    float Kp;
    float Ki;
    float integral;
    float out_max;
    float out_min;
} PI_Controller;

// --- FOC Handle Struct ---
typedef struct {
    // --- Hardware Links ---
    TIM_HandleTypeDef *htim_pwm;  // PWM Timer Handle (TIM1 or TIM8)
    uint16_t u_tim_channel;
    uint16_t v_tim_channel;
    uint16_t w_tim_channel;

    MT6701_Data_t *encoder;       // Encoder Handle
    DRV8316C_Handle_t *hdrv;

    // --- Configuration ---
    float pole_pairs;             // 극쌍수
    float zero_offset_angle;      // 전기각 0점 오프셋 (Calibration 필요)
    int8_t dir;                   // 회전 방향 (1 or -1)

    // --- State Variables ---
    float theta_e;                // 전기각 (Electrical Angle, rad)
    float Iu, Iv, Iw;             // 상전류 (A)
    float Id, Iq;                 // d-q축 전류
    float I_alpha, I_beta;        // alpha-beta 전류

    // --- Inputs (References) ---
    float Id_ref;                 // d축 전류 지령 (보통 0)
    float Iq_ref;                 // q축 전류 지령 (토크)

    // --- Controllers ---
    PI_Controller pid_d;
    PI_Controller pid_q;

    // --- ADC Offsets (Calibration) ---
    float offset_iu_adc;
    float offset_iv_adc;
    float offset_iw_adc;

    // --- ADC Raw Values (from DMA buffer) ---
    int16_t adc_raw_u;
    int16_t adc_raw_v;
    int16_t adc_raw_w;

    PI_Controller pid_speed;
} FOC_Handle_t;

// --- Global Instances ---
extern FOC_Handle_t focL;
extern FOC_Handle_t focR;

// --- Functions ---
void FOC_Init(FOC_Handle_t *hfoc, TIM_HandleTypeDef *htim, MT6701_Data_t *enc, DRV8316C_Handle_t *hdrv,
		uint16_t u_tim_channel, uint16_t v_tim_channel, uint16_t w_tim_channel);
void FOC_Start(FOC_Handle_t *hfoc);
void FOC_Stop(FOC_Handle_t *hfoc);
void FOC_Calibrate_ADC_Offset(FOC_Handle_t *hfoc);
void FOC_Set_Torque(FOC_Handle_t *hfoc, float iq_target);
void FOC_Update(FOC_Handle_t *hfoc); // Call this in Timer Interrupt (10kHz+)

#endif /* FOC_H_ */
