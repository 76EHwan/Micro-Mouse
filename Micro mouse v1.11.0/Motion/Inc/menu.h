/*
 * menu.h
 * Location: Motion/Inc/menu.h
 */

#ifndef MOTION_INC_MENU_H_
#define MOTION_INC_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "adc.h"
#include "st7789.h"

extern uint8_t battery_percent;

void Menu_Init(void);
void Menu_Loop(void);


void Show_Battery(void);

void Show_IMU(void);

void Show_Current(void);

void Show_ToF(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTION_INC_MENU_H_ */
