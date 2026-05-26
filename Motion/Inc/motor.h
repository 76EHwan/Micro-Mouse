/*
 * motor.h
 *
 *  Created on: Feb 10, 2026
 *      Author: kth59
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "foc.h"

void Simple_6_step_Control(FOC_Handle_t *foc);
void Simple_SVPWM_Control(FOC_Handle_t *foc, float_t step);

void Motor_Start(void);

#endif /* INC_MOTOR_H_ */
