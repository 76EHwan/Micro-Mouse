/*
 * bldc_open_loop.c
 *
 * Description: Open Loop SVPWM Control for DRV8316C
 * (Corrected for Voltage Scaling & Startup Torque)
 */

#include "main.h"
#include "math.h"
#include "tim.h"
#include "drive.h"
#include "bldc_open_loop.h"

/* --- [1] 모터 튜닝 파라미터 (여기를 잘 맞춰야 돕니다!) --- */
// 1. 사용하는 모터의 극쌍 수 (대부분의 드론/마우스 모터는 7입니다. 1 아님!)
#define POLE_PAIRS      1.0f

// 2. 배터리 전압 (실제 전압과 비슷하게 설정)
#define VBUS_VOLTAGE    5.f

// 3. 타이머 주기 (tim.c의 Counter Period 값)
#define PWM_PERIOD      9999.0f

// 4. 모터 제한 설정
#define MAX_VOLTAGE     (VBUS_VOLTAGE * 0.9f) // 최대 출력 전압 (포화 방지)

// 5. 오픈루프 기동 설정 (중요)
#define RAMP_RATE       1000.0f    // 가속도 (RPM/s) - 너무 빠르면 탈조남
#define MIN_START_V     2.5f       // 기동 전압 (V) - 안 돌면 3.0f~4.0f로 올리세요!
#define V_PER_RPM       0.005f     // 속도 비례 전압 (V/RPM) - 고속에서 힘 부족하면 올리세요

/* --- 상수 및 변수 --- */
#define TWO_PI          6.28318530718f
#define DT              0.0005f    // 제어 루프 주기 (TIM7 2kHz = 0.5ms 가정)

typedef struct {
	TIM_HandleTypeDef *htim;
	float current_rpm;            // 현재 실제 출력중인 RPM
	float theta_e;                // 전기각 (Electrical Angle)
	float voltage_mag;            // 인가 전압 크기
} BLDC_Motor_t;

BLDC_Motor_t Motor_L = { .htim = &htim1, .theta_e = 0.0f, .current_rpm = 0.0f };
BLDC_Motor_t Motor_R = { .htim = &htim8, .theta_e = 0.0f, .current_rpm = 0.0f };

/* --- SVPWM 생성 함수 (핵심 수정됨) --- */
static void SVPWM_Generate(TIM_HandleTypeDef *htim, float Valpha, float Vbeta) {
	// [수정 포인트] 2.0f -> 1.0f (전압 스케일링 정상화)
	float Vref = (1.0f / VBUS_VOLTAGE) * PWM_PERIOD;

	// 역 Clarke 변환
	float Va = Valpha;
	float Vb = -0.5f * Valpha + 0.8660254f * Vbeta;
	float Vc = -0.5f * Valpha - 0.8660254f * Vbeta;

	// Center Aligned SVPWM (Min-Max 주입)
	float V_max = Va;
	if (Vb > V_max)
		V_max = Vb;
	if (Vc > V_max)
		V_max = Vc;

	float V_min = Va;
	if (Vb < V_min)
		V_min = Vb;
	if (Vc < V_min)
		V_min = Vc;

	float V_com = (V_max + V_min) * 0.5f;

	// 실제 PWM 듀티 계산 (V_com을 빼서 중심을 맞춤 + 50% 오프셋)
	// Va * Vref가 PWM Count 단위가 됨.
	float Ta = ((Va - V_com) * Vref) + (PWM_PERIOD / 2.0f);
	float Tb = ((Vb - V_com) * Vref) + (PWM_PERIOD / 2.0f);
	float Tc = ((Vc - V_com) * Vref) + (PWM_PERIOD / 2.0f);

	// 값 제한 (Saturation)
	if (Ta > PWM_PERIOD)
		Ta = PWM_PERIOD;
	else if (Ta < 0)
		Ta = 0;
	if (Tb > PWM_PERIOD)
		Tb = PWM_PERIOD;
	else if (Tb < 0)
		Tb = 0;
	if (Tc > PWM_PERIOD)
		Tc = PWM_PERIOD;
	else if (Tc < 0)
		Tc = 0;

	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (uint32_t )Ta);
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (uint32_t )Tb);
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (uint32_t )Tc);
}

/* --- 모터 업데이트 루프 --- */
static void BLDC_Update_Loop(BLDC_Motor_t *motor, float target_rpm) {
	float Valpha, Vbeta;

	// 1. 속도 램프 (급격한 변화 방지)
	float rpm_diff = target_rpm - motor->current_rpm;
	float max_step = RAMP_RATE * DT;

	if (rpm_diff > max_step)
		motor->current_rpm += max_step;
	else if (rpm_diff < -max_step)
		motor->current_rpm -= max_step;
	else
		motor->current_rpm = target_rpm;

	// 2. 각도 적분 (속도 -> 위치)
	// 전기각 주파수 = (RPM / 60) * PolePairs
	float freq_e = (motor->current_rpm / 60.0f) * POLE_PAIRS;
	motor->theta_e += freq_e * TWO_PI * DT;

	// 각도 정규화 (0 ~ 2PI)
	if (motor->theta_e > TWO_PI)
		motor->theta_e -= TWO_PI;
	else if (motor->theta_e < 0.0f)
		motor->theta_e += TWO_PI;

	// 3. V/f 제어 (전압 결정)
	// 목표 속도가 0에 가까우면 전압 차단 (정지)
	if (fabs(motor->current_rpm) < 1.0f) { // 1 RPM 미만은 정지
		motor->voltage_mag = 0.0f;
	} else {
		// 기본 마찰 극복 전압 + 속도 비례 전압
		motor->voltage_mag = MIN_START_V
				+ (fabs(motor->current_rpm) * V_PER_RPM);

		// 전압 제한
		if (motor->voltage_mag > MAX_VOLTAGE)
			motor->voltage_mag = MAX_VOLTAGE;
	}

	// 4. 회전 전압 벡터 생성
	// RPM이 음수면 각도는 반대로 돌지만, Valpha/Vbeta 계산은 동일 (freq_e 부호로 처리됨)
	// 다만, Open-Loop에서는 '방향'을 명확히 하기 위해 절대값 전압을 쓰고 각도만 돌리는게 안전함.

	Valpha = motor->voltage_mag * cosf(motor->theta_e);
	Vbeta = motor->voltage_mag * sinf(motor->theta_e);

	// 5. 하드웨어 출력
	SVPWM_Generate(motor->htim, Valpha, Vbeta);
}

/* --- 메인 호출 함수 --- */
void Motor_Control_Loop(void) {
	// drive.h의 목표 속도 사용
	BLDC_Update_Loop(&Motor_L, mps.ref.left);
	BLDC_Update_Loop(&Motor_R, mps.ref.right);
}

/* --- 하드웨어 시작 함수 --- */
void Motor_Start(void) {
	// 제어 루프 타이머 (TIM7)
//	HAL_TIM_Base_Start_IT(&htim1);

	// PWM 타이머 (TIM1, TIM8)
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
}

void Motor_Stop(void) {
//	HAL_TIM_Base_Stop_IT(&htim7);

	// PWM 타이머 (TIM1, TIM8)
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_3);
}
