/*
 * mt6701.h
 *
 *  Created on: Jan 24, 2026
 *      Author: kth59
 */

#ifndef INC_MT6701_H_
#define INC_MT6701_H_

#include "main.h"

typedef struct {
    uint16_t raw_angle; // 14-bit raw value
    float angle_deg;    // 변환된 각도 (0~360도)
    uint8_t status;     // 상태 비트
    uint8_t crc;        // CRC 값
    GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
} MT6701_Data_t;

extern MT6701_Data_t encDataL;
extern MT6701_Data_t encDataR;

void MT6701_ReadSSI(MT6701_Data_t *encData);

#endif /* INC_MT6701_H_ */
