/*
 * vl53l4cx.h
 *
 *  Created on: Jan 17, 2026
 *      Author: kth59
 */

#ifndef INC_VL53L4CX_H_
#define INC_VL53L4CX_H_

#include "VL53Lx_api.h"
#include "vl53lx_register_settings.h"

#define VL53L4CX_NUM 4
#define VL53LX_SLAVE_ADDRESS_DEFAULT (VL53LX_EWOK_I2C_DEV_ADDR_DEFAULT << 1)

extern VL53LX_DEV vl53lx;
extern uint8_t is_vl53lx_ready[VL53L4CX_NUM];

extern uint8_t byteData;
extern uint16_t wordData;
extern VL53LX_MultiRangingData_t MultiRangingData[VL53L4CX_NUM];
extern VL53LX_MultiRangingData_t *pMultiRangingData;
extern uint8_t NewDataReady;
extern int no_of_object_found;

HAL_StatusTypeDef VL53L4CX_Init(VL53LX_DEV Dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t new_address, uint8_t i);
HAL_StatusTypeDef MX_VL53L4CX_Init();
void VL53L4CX_Start();

#endif /* INC_VL53L4CX_H_ */
