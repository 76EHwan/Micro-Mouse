/*
 * init.h
 *
 *  Created on: Jan 31, 2026
 *      Author: kth59
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

#define SENSOR_IS_IR
//#define SENSOR_IS_TOF

#if defined(SENSOR_IS_IR) && defined(SENSOR_IS_TOF)
    #error "Error: OPTION_A and OPTION_B cannot be defined at the same time!"
#endif

void MX_User_Init();
void IMU_Start();
#endif /* INC_INIT_H_ */
