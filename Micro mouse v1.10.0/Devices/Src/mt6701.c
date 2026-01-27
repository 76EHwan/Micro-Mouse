/*
 * mt6701.c
 *
 *  Created on: Jan 24, 2026
 *      Author: kth59
 */

#include "mt6701.h"
#include "spi.h"
#include "main.h"

#define ENC_SPI &hspi2

/* USER CODE BEGIN 0 */
MT6701_Data_t encDataL = {
		.cs_port = ENC_L_CS_GPIO_Port, .cs_pin = ENC_L_CS_Pin
};
MT6701_Data_t encDataR = {
		.cs_port = ENC_R_CS_GPIO_Port, .cs_pin = ENC_R_CS_Pin
};


void MT6701_ReadSSI(MT6701_Data_t *encData) {
    uint8_t rxBuffer[3] = {0, 0, 0}; // 24비트(3바이트) 수신 버퍼
    uint32_t rawValue = 0;

    // 1. CS Low (통신 시작)
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_RESET);

    // t_clk_fe 딜레이 (필요시 약 1~2us 대기, H5는 빠르므로 짧은 루프 필요할 수 있음)
    // for(int i=0; i<10; i++) __NOP();

    // 2. SPI 데이터 수신 (3바이트)
    // Timeout은 상황에 맞게 조절 (여기선 10ms)
    if (HAL_SPI_Receive(ENC_SPI, rxBuffer, 3, 10) == HAL_OK) {

        // 3. 3바이트를 하나의 32비트 정수로 합침 (Big Endian 기준)
        rawValue = ((uint32_t)rxBuffer[0] << 16) | ((uint32_t)rxBuffer[1] << 8) | rxBuffer[2];

        // 4. 데이터 파싱 (MT6701 SSI 포맷: 24bit = Angle[14] + Status[4] + CRC[6])

        // 상위 14비트 추출 (전체 24비트 중 상위 14비트)
        encData->raw_angle = (rawValue >> 10) & 0x3FFF;

        // 각도 변환 (0 ~ 360도)
        encData->angle_deg = (float)encData->raw_angle * 360.0f / 8192.0f - 360.f;

        // 상태 비트 (중간 4비트)
        encData->status = (rawValue >> 6) & 0x0F;

        // CRC (하위 6비트) - 필요시 검증 로직 추가
        encData->crc = rawValue & 0x3F;
    }

    // 5. CS High (통신 종료)
    HAL_GPIO_WritePin(encData->cs_port, encData->cs_pin, GPIO_PIN_SET);

    // t_mono 딜레이 (다음 읽기 전까지 최소 대기 시간, 보통 수 us)
    // for(int i=0; i<50; i++) __NOP();
}
/* USER CODE END 0 */
