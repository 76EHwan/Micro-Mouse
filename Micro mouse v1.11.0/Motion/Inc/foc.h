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
#define PWM_PERIOD      (TIM1->ARR)    // Timer Period
#define PWM_PERIOD_HALF	(PWM_PERIOD / 2)
#define VBUS            8.4f    // Battery Voltage
#define SQRT3_INV       0.577350269f // 1/sqrt(3)
#define ANGLE_SCALER	(2 * M_PI / ENC_RES)

// DRV8316 Current Sense Gain (Default 0.6 V/A)
#define CSA_GAIN        0.6f
#define CSA_GAIN_INV	(1.f / CSA_GAIN)
#define ADC_REF_VOLT    3.3f
#define ADC_RES         4096.f
#define ADC_RES_HALF	(ADC_RES / 2.f)
#define ADC_RES_INV		(1.f/ADC_RES)

// --- MOTOR Parameter ---
#define MOTOR_RES 		1.9 // Motor Resister
#define MOTOR_IND 		0.000024f // Motor Inductance
#define CTRL_FREQ_HZ        40000.0f // FOC 제어 인터럽트 주파수 (40kHz)
#define CTRL_TS             (1.0f / CTRL_FREQ_HZ) // 제어 주기 (0.000025s)

// Current Bandwidth
#define CURRENT_BW_HZ       2000.0f         // 목표 차단 주파수 (1000Hz)
#define CURRENT_BW_RAD      (2.0f * 3.1415926f * CURRENT_BW_HZ) // 오메가 c (rad/s)

// Compile-time calculation
#define FOC_CURRENT_KP      (MOTOR_IND * CURRENT_BW_RAD)
#define FOC_CURRENT_KI      (MOTOR_RES * CURRENT_BW_RAD * CTRL_TS)

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
    float vBus;					  // 입력 전압 (V)
    float Iu, Iv;                 // 상전류 (A)
    float Id, Iq;                 // d-q축 전류
    float I_alpha, I_beta;        // alpha-beta 전류

    // --- Inputs (References) ---
    float Id_ref;                 // d축 전류 지령 (보통 0)
    float Iq_ref;                 // q축 전류 지령 (토크)

    // --- Park Transform Variables ---
    float Vd;
    float Vq;

    // --- Clarke Transform Variables ---
    float Valpha;
    float Vbeta;

    // --- Controllers ---
    PI_Controller pid_d;
    PI_Controller pid_q;

    // --- ADC Raw Values (from DMA buffer) ---
    int16_t adc_raw_u;
    int16_t adc_raw_v;

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
