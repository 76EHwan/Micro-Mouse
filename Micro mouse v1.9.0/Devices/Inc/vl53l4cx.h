/*
 * vl53l4cx.h
 *
 *  Created on: Jan 17, 2026
 *      Author: kth59
 */

#ifndef INC_VL53L4CX_H_
#define INC_VL53L4CX_H_

#include "VL53Lx_api.h"

extern VL53LX_Dev_t dev;
extern VL53LX_DEV Dev;
extern volatile int IntCount;

extern uint8_t byteData;
extern uint16_t wordData;
extern VL53LX_MultiRangingData_t MultiRangingData;
extern VL53LX_MultiRangingData_t *pMultiRangingData;
extern uint8_t NewDataReady;
extern int no_of_object_found;

void VL53L4CX_Init();

#endif /* INC_VL53L4CX_H_ */
