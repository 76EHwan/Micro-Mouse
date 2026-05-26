/*
 * switch.c
 *
 * Location: Motion/Src/switch.c
 * Author: Joonho Gwon
 * Refactored by: Gemini (HAL Version)
 */

#include "switch.h" // 헤더 파일명 변경 반영

/* Private Enums -------------------------------------------------------------*/

/**
 * @brief 내부 Debouncing FSM 상태 정의
 */
typedef enum {
    SW_STATE_LONG_OFF  = 0,
    SW_STATE_SHORT_ON,     // 눌림 과도기
    SW_STATE_LONG_ON,      // 눌림 확정
    SW_STATE_SHORT_OFF     // 뗌 과도기
} SwitchFsmState_t;


/* Private Defines -----------------------------------------------------------*/
#define TIME_SHORT  40  // 40ms Debouncing time
#define TIME_LONG   80  // 80ms Repeat rate


/* Private Structs -----------------------------------------------------------*/
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pinMask;
    int32_t timer;
    uint32_t prevTick;
    SwitchFsmState_t state;
} ButtonState_t;


/* Private Variables ---------------------------------------------------------*/
static bool isInitialized = false;
static ButtonState_t sw1, sw2;


/* Private Functions ---------------------------------------------------------*/

// 내부 함수명도 Switch_... 로 통일
static void Switch_Init_ButtonState(ButtonState_t *State, GPIO_TypeDef *GPIOx, uint16_t PinMask) {
    State->port = GPIOx;
    State->pinMask = PinMask;
    State->timer = 0;
    State->prevTick = HAL_GetTick();
    State->state = SW_STATE_LONG_OFF;
}

static uint8_t Switch_State_Machine(ButtonState_t *State) {
    // Active Low (눌림 = 0)
    bool currentPushed = (HAL_GPIO_ReadPin(State->port, State->pinMask) == GPIO_PIN_RESET);
    bool pushEvent = false;

    uint32_t currTick = HAL_GetTick();
    uint32_t timeDelta = currTick - State->prevTick;

    switch (State->state) {

        case SW_STATE_LONG_OFF:
            if (currentPushed) {
                State->state = SW_STATE_SHORT_ON;
                State->timer = TIME_SHORT;
            }
            break;

        case SW_STATE_SHORT_ON:
            if (State->timer <= (int32_t)timeDelta) {
                pushEvent = true;
                State->state = SW_STATE_LONG_ON;
                State->timer = TIME_LONG;
            } else {
                State->timer -= (int32_t)timeDelta;
            }
            break;

        case SW_STATE_LONG_ON:
            if (!currentPushed) {
                State->state = SW_STATE_SHORT_OFF;
                State->timer = TIME_SHORT;
            }
            else {
                // Repeat Logic
                if (State->timer <= (int32_t)timeDelta) {
                    pushEvent = true;
                    State->timer = TIME_LONG;
                } else {
                    State->timer -= (int32_t)timeDelta;
                }
            }
            break;

        case SW_STATE_SHORT_OFF:
            if (State->timer <= (int32_t)timeDelta) {
                State->state = SW_STATE_LONG_OFF;
            } else {
                State->timer -= (int32_t)timeDelta;
            }
            break;
    }

    State->prevTick = currTick;
    return pushEvent;
}

/* Public Functions ----------------------------------------------------------*/

/**
 * @brief  메인 루프에서 호출되는 스위치 읽기 함수
 */
uint8_t Switch_Read(void) {
    // 1. 초기화
    if (!isInitialized) {
        isInitialized = true;
        Switch_Init_ButtonState(&sw1, SW1_PORT, SW1_PIN);
        Switch_Init_ButtonState(&sw2, SW2_PORT, SW2_PIN);
    }

    // 2. 각 버튼 상태 머신 실행
    uint8_t sw1PushEvent = Switch_State_Machine(&sw1);
    uint8_t sw2PushEvent = Switch_State_Machine(&sw2);

    /* 3. 동시 입력 보정 로직
     * 한쪽은 이미 켜졌는데(LONG_ON), 다른 쪽이 켜지려는 중(SHORT_ON)이라면
     * 늦은 쪽을 강제로 켜서 동기화시킴
     */
    if ((sw1.state == SW_STATE_LONG_ON) && (sw2.state == SW_STATE_SHORT_ON)) {
        sw2PushEvent = true;
        sw2.state = SW_STATE_LONG_ON;
        sw2.timer = TIME_LONG;
    }

    if ((sw2.state == SW_STATE_LONG_ON) && (sw1.state == SW_STATE_SHORT_ON)) {
        sw1PushEvent = true;
        sw1.state = SW_STATE_LONG_ON;
        sw1.timer = TIME_LONG;
    }

    // 4. 결과값 반환
    uint8_t buttonPushEvent = SW_EVENT_NONE;

    if (sw1PushEvent) buttonPushEvent |= SW_EVENT_1;
    if (sw2PushEvent) buttonPushEvent |= SW_EVENT_2;

    return buttonPushEvent;
}
