/*
 * drive.h
 *
 *  Created on: Nov 11, 2025
 *      Author: kth59
 */

#ifndef INC_DRIVE_H_
#define INC_DRIVE_H_

typedef struct {
	float left;
	float right;
} Motor_Speed_t;

typedef struct {
	float accel;
	float decel;
	float current;
	float target;
	Motor_Speed_t ref;
	Motor_Speed_t feedback;
} Speed_t;

extern Speed_t mps;

void Drive_First();


#endif /* INC_DRIVE_H_ */
