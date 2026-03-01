/*
 * sensor.h
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "main.h"

#define SENSOR_TIM TIM6

extern uint16_t sensor_L;
extern uint16_t sensor_CL;
extern uint16_t sensor_CR;
extern uint16_t sensor_R;

void Sensor_Start();
void Sensor_Get_Dist(uint8_t i);

#endif /* SENSOR_H_ */
