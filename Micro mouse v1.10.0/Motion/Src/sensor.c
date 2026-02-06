/*
 * sensor.c
 *
 *  Created on: Nov 10, 2025
 *      Author: kth59
 */

#include "sensor.h"
#include "adc.h"

#include "drv8316crq1.h"

#define CURRENT_CONV_FACTOR   0.001342773f
#define CURRENT_OFFSET_RAW    2048.0f

void ADC1_Start() {
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, adc1_buffer, 7);
}

__STATIC_INLINE float Convert_DRV8316C_ADC_To_Current(uint32_t adc_data) {
	int32_t voltage_measured = adc_data - CURRENT_OFFSET_RAW;
	return voltage_measured * CURRENT_CONV_FACTOR;
}

void Calc_DRV8316C_Current() {
	DRV8316C_L.u_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[1]);
	DRV8316C_L.v_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[2]);
	DRV8316C_L.w_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[3]);
	DRV8316C_R.u_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[4]);
	DRV8316C_R.v_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[5]);
	DRV8316C_R.w_current = Convert_DRV8316C_ADC_To_Current(adc1_buffer[6]);
}
