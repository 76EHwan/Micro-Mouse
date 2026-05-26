/*
 * drive.c
 *
 *  Created on: Nov 11, 2025
 *      Author: kth59
 */

#include "main.h"
#include "drive.h"
#include "foc.h"
#include "sensor.h"

#define dt 0.0005f

#define KP_CENTER 0.1f
#define KD_CENTER 0.1f

#define STRAIGHT 0x0
#define TURN 0x1

uint32_t state;
uint32_t prev_error_center = 0;

Speed_t mps = { .accel = 1.f, .decel = 1.f };

void Drive_TIM7_IRQ() {
	if (mps.current < mps.target) {
		if (mps.target - mps.current < mps.accel * dt)
			mps.current = mps.target;
		else
			mps.current += mps.accel * dt;
	} else if (mps.current > mps.target) {
		if (mps.current - mps.target < mps.decel * dt)
			mps.current = mps.target;
		else
			mps.current -= mps.decel * dt;
	}

	switch(state){
	case STRAIGHT:
		float error_center = 0.f;
		float error_center_deriv = error_center - prev_error_center;
		float error_center_output = KP_CENTER * error_center + KD_CENTER * error_center_deriv;
		prev_error_center = error_center;
		mps.ref.left = mps.current - error_center_output;
		mps.ref.right = mps.current + error_center_output;
		break;
	case TURN:

		break;
	}
}

void Drive_First(){

}

void Test_UVW(){

	for(uint8_t i = 0; i < 3; i++){

	}
}
