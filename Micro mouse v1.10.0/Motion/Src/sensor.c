/*
 * sensor.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#include "sensor.h"
#include "adc.h"

void ADC_Battery_Start(){
	HAL_ADC_Start_DMA(&hadc2, &raw_vbattery, 1);
}
