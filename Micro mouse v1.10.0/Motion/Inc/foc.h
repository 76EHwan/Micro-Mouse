/*
 * foc.h
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#ifndef FOC_H_
#define FOC_H_

#include "math.h"

#define PWM_PERIOD 	1024
#define VBUS		7.4f
#define SQRT3_INV	(1.f / M_SQRT3)
#define PWM_FREQUENCY	20000

typedef struct {
    float Kp;
    float Ki;
    float integral;
    float out_max;
} PI_Controller;

float PI_Update(PI_Controller *pid, float ref, float feedback);
void SVPWM_Generate(float Valpha, float Vbeta);
void FOC_Loop(void);

#endif /* FOC_H_ */
