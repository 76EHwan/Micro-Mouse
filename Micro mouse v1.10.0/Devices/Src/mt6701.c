#include "mt6701.h"
#include "spi.h"
#include "main.h"
#include <math.h> // floorf 함수 사용

#define ENC_SPI &hspi2

// 기어비 (51 / 9)
#define GEAR_RATIO (51.0f / 9.0f)
// 극쌍수 1
#define POLE_PAIRS 1.0f

// 상수 미리 정의 (연산 속도 최적화)
#define ENC_RES 16384.0f
#define ENC_HALF 8192
#define DEG_PER_TICK (360.0f / ENC_RES) // 0.02197...

/* USER CODE BEGIN 0 */
MT6701_Data_t encDataL = { .cs_port = ENC_L_CS_GPIO_Port, .cs_pin = ENC_L_CS_Pin };
MT6701_Data_t encDataR = { .cs_port = ENC_R_CS_GPIO_Port, .cs_pin = ENC_R_CS_Pin };

// 초기화 함수 (첫 실행 시 튀는 것 방지용)
HAL_StatusTypeDef MT6701_Init(MT6701_Data_t *encData, uint8_t *rxBuffer) {
//    uint8_t rxBuffer[3] = {0,0,0};
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Receive(ENC_SPI, rxBuffer, 3, 10);
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_SET);

    if((((rxBuffer[2] >> 6) | rxBuffer[1]) & 0x03)) return HAL_ERROR;

    uint32_t rawValue = ((uint32_t)rxBuffer[0] << 16) | ((uint32_t)rxBuffer[1] << 8) | rxBuffer[2];
    encData->last_raw_angle = (rawValue >> 10) & 0x3FFF;
    encData->motor_elec_angle = 0.0f;
    return HAL_OK;
}

void MT6701_ReadSSI(MT6701_Data_t *encData) {
    uint8_t rxBuffer[3]; // 초기화 불필요 (속도 ↑)

    // 1. SPI 통신 (빠른 처리를 위해 타임아웃 최소화 or DMA 권장)
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_RESET);
    if (HAL_SPI_Receive(ENC_SPI, rxBuffer, 3, 2) == HAL_OK) { // Timeout 2ms로 단축

        // 2. 비트 연산 (빠름)
        uint32_t rawValue = ((uint32_t)rxBuffer[0] << 16) | ((uint32_t)rxBuffer[1] << 8) | rxBuffer[2];
        uint16_t new_raw = (rawValue >> 10) & 0x3FFF; // 0 ~ 16383

        // 3. 변화량 계산 (Delta)
        // (int32_t) 캐스팅으로 언더플로우 방지 -> 매우 중요!
        int32_t diff = (int32_t)new_raw - (int32_t)encData->last_raw_angle;

        // 4. Wrap-around 처리 (한 바퀴 넘었을 때 보정)
        // 비트 연산이나 삼항 연산자보다 if문 분기가 가독성과 예측성에 좋음
        if (diff < -ENC_HALF) diff += 16384;
        else if (diff > ENC_HALF) diff -= 16384;

        // 5. 각도 변환 및 누적 (FPU 사용)
        // 변화량(diff)에 바로 비율을 곱해서 한 번에 더함 (연산 횟수 최소화)
        // 공식: diff * (360/16384) * 기어비 * 극쌍수
        float angle_step = (float)diff * DEG_PER_TICK * GEAR_RATIO * POLE_PAIRS;
        encData->motor_elec_angle += angle_step;

        // 6. 0~360 범위 유지 (최적화된 방식)
        // if문 반복보다 floorf가 CPU 사이클은 더 먹을 수 있지만,
        // 기어비 때문에 각도가 -5000도, +5000도로 누적되는 걸 수학적으로 완벽하게 막아줌.
        // FPU가 있는 STM32F4/G4/H7 등에서는 매우 빠름.
        encData->motor_elec_angle -= 360.0f * floorf(encData->motor_elec_angle / 360.0f);

        // 7. 데이터 갱신
        encData->last_raw_angle = new_raw;

        // (선택) 디버깅 정보가 필요 없다면 아래 두 줄 삭제 가능
        // encData->status = (rawValue >> 6) & 0x0F;
        // encData->crc = rawValue & 0x3F;
    }
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_SET);
}
