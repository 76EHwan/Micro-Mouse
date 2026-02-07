/*
 * sensor.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#include "sensor.h"
#include "adc.h"

#include "foc.h"

#define CURRENT_CONV_FACTOR   0.001342773f
#define CURRENT_OFFSET_RAW    2048.0f

void ADC1_Start() {
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, adc1_buffer, 7);
}

void ADC2_Start() {
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc2, adc2_buffer, 4);
}

void Calc_DRV8316C_Current() {
	focL.adc_raw_u = (uint16_t) adc1_buffer[1];
	focL.adc_raw_v = (uint16_t) adc1_buffer[2];
	focL.adc_raw_w = (uint16_t) adc1_buffer[3];
	focR.adc_raw_u = (uint16_t) adc1_buffer[4];
	focR.adc_raw_v = (uint16_t) adc1_buffer[5];
	focR.adc_raw_w = (uint16_t) adc1_buffer[6];

}
