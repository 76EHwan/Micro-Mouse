/*
 * switch.h
 *
 * Location: Motion/Inc/switch.h
 * Author: Joonho Gwon
 * Refactored by: Gemini (HAL Version)
 */

#ifndef MOTION_INC_SWITCH_H_
#define MOTION_INC_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"  // HAL Driver 및 GPIO Pin 정의 참조
#include <stdint.h>
#include <stdbool.h>

/* Enums & Typedefs ----------------------------------------------------------*/

/**
 * @brief 스위치 입력 이벤트 정의 (Bitmask)
 * @note  두 버튼이 동시에 눌리면 OR 연산됨 (SW_EVENT_1 | SW_EVENT_2)
 */
typedef enum {
    SW_EVENT_NONE = 0x00,
    SW_EVENT_1    = 0x01, // (1 << 0)
    SW_EVENT_2    = 0x02, // (1 << 1)
    SW_EVENT_BOTH = 0x03  // (SW_EVENT_1 | SW_EVENT_2)
} SwitchEvent_t;

/* User Settings -------------------------------------------------------------*/
/* * main.h에 정의된 Pin/Port 라벨과 매핑
 */
#define SW1_PORT    MTR_L_nFAULT_GPIO_Port
#define SW1_PIN     MTR_L_nFAULT_Pin
#define SW2_PORT    MTR_R_nFAULT_GPIO_Port
#define SW2_PIN     MTR_R_nFAULT_Pin

/* Function Prototypes -------------------------------------------------------*/

/**
 * @brief  스위치 상태를 읽어옵니다. (Debouncing 및 동시 입력 보정 적용됨)
 * @retval SwitchEvent_t (uint8_t)
 */
uint8_t Switch_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTION_INC_SWITCH_H_ */
