/*
 * DRV8316C.c
 *
 * Created on: Nov 10, 2025
 * Author: kth59
 */
/*
 * DRV8316C.c
 *
 * Modified for 16-bit SPI Mode
 */

/*
 * DRV8316C.c
 *
 * Modified for 16-bit SPI Mode with CORRECT Bit Packing
 * [중요] 칩 스펙에 맞춰 비트 위치를 강제로 재정의했습니다.
 */

#include "drv8316crq1.h"
#include "st7789.h"

// --- [CRITICAL] DRV8316C SPI Bit Definitions ---
// 헤더 파일의 정의가 틀렸을 수 있으므로 여기서 직접 정의합니다.
#define DRV_RW_READ_BIT     (1 << 15)
#define DRV_ADDR_SHIFT      9           // 주소는 9비트 밀어야 함 (Bit 14-9)
#define DRV_PARITY_BIT      (1 << 8)    // 패리티는 Bit 8에 위치
#define DRV_DATA_MASK       0xFF        // 데이터는 하위 8비트

DRV8316C_Handle_t DRV8316C_L;
DRV8316C_Handle_t DRV8316C_R;

/*=======================================================================*/
/* Internal Helper Functions                                             */
/*=======================================================================*/

/**
 * @brief  Calculates the even parity bit.
 */
static uint8_t DRV8316C_CalculateEvenParity(uint16_t data) {
	uint8_t one_count = 0;
	// 패리티 비트 위치(Bit 8)를 제외하고 1의 개수 카운트
	data &= ~DRV_PARITY_BIT;

	for (int i = 0; i < 16; i++) {
		if ((data >> i) & 0x01) {
			one_count++;
		}
	}
	// 홀수 개면 1을 반환 (전체를 짝수로 맞춤)
	return (one_count % 2);
}

/**
 * @brief  Internal helper for SPI Tx/Rx (Native 16-bit)
 */
static HAL_StatusTypeDef DRV8316C_SPI_TxRx(DRV8316C_Handle_t *hdrv,
		uint16_t *pTxData, uint16_t *pRxData) {
	HAL_StatusTypeDef status;

	DRV8316C_CS_LOW(hdrv);

	// 16-bit 모드: Size = 1 (16비트 한 덩어리 전송)
	status = HAL_SPI_TransmitReceive(hdrv->hspi, (uint8_t*) pTxData,
			(uint8_t*) pRxData, 1, 100);

	DRV8316C_CS_HIGH(hdrv);

	return status;
}

/*=======================================================================*/
/* Public Function Implementations                                       */
/*=======================================================================*/

void DRV8316C_Init(DRV8316C_Handle_t *hdrv, SPI_HandleTypeDef *hspi,
		GPIO_TypeDef *nCS_Port, uint16_t nCS_Pin, GPIO_TypeDef *nFAULT_Port,
		uint16_t nFAULT_Pin, GPIO_TypeDef *DRVOFF_Port, uint16_t DRVOFF_Pin,
		TIM_HandleTypeDef *htim, uint32_t u_channel, uint32_t v_channel,
		uint32_t w_channel) {
	hdrv->hspi = hspi;
	hdrv->nCS_Port = nCS_Port;
	hdrv->nCS_Pin = nCS_Pin;

	hdrv->nFAULT_Port = nFAULT_Port;
	hdrv->nFAULT_Pin = nFAULT_Pin;

	hdrv->DRVOFF_Port = DRVOFF_Port;
	hdrv->DRVOFF_Pin = DRVOFF_Pin;

	hdrv->htim = htim;
	hdrv->U_CHANNEL = u_channel;
	hdrv->V_CHANNEL = v_channel;
	hdrv->W_CHANNEL = w_channel;

	DRV8316C_CS_HIGH(hdrv);
}

/**
 * @brief  Writes 16-bit frame with CORRECT Bit Packing
 */
HAL_StatusTypeDef DRV8316C_WriteRegister(DRV8316C_Handle_t *hdrv,
		uint8_t regAddr, uint8_t data) {
	uint16_t tx_frame = 0;
	uint16_t rx_frame = 0;

	// 1. 프레임 생성 (Write=0, Address Shift=9, Data)
	// [중요] 기존 코드의 shift 8이나 mask 문제를 해결
	tx_frame = ((uint16_t) (regAddr & 0x3F) << DRV_ADDR_SHIFT)
			| (data & DRV_DATA_MASK);

	// 2. 패리티 계산 및 삽입 (Bit 8)
	if (DRV8316C_CalculateEvenParity(tx_frame)) {
		tx_frame |= DRV_PARITY_BIT;
	}

	// 3. 전송
	return DRV8316C_SPI_TxRx(hdrv, &tx_frame, &rx_frame);
}

/**
 * @brief  Reads 16-bit frame with CORRECT Bit Packing
 */
HAL_StatusTypeDef DRV8316C_ReadRegister(DRV8316C_Handle_t *hdrv,
		uint8_t regAddr, uint8_t *pData) {
	uint16_t tx_frame = 0;
	uint16_t rx_frame = 0;
	HAL_StatusTypeDef status;

	// 1. Read 프레임 생성 (Read=1, Address Shift=9)
	tx_frame = DRV_RW_READ_BIT
			| ((uint16_t) (regAddr & 0x3F) << DRV_ADDR_SHIFT);

	// 2. 패리티 계산 및 삽입
	if (DRV8316C_CalculateEvenParity(tx_frame)) {
		tx_frame |= DRV_PARITY_BIT;
	}

	// 3. 전송 및 수신
	status = DRV8316C_SPI_TxRx(hdrv, &tx_frame, &rx_frame);

	if (status == HAL_OK) {
		// 4. 데이터 추출 (하위 8비트)
		*pData = (uint8_t) (rx_frame & DRV_DATA_MASK);
	}

	return status;
}

// ... (Unlock, Lock, ApplyDefault 등 나머지 함수는 변경 없음) ...
HAL_StatusTypeDef DRV8316C_UnlockRegister(DRV8316C_Handle_t *hdrv) {
	return DRV8316C_WriteRegister(hdrv, 0x3, 0x3);
}

HAL_StatusTypeDef DRV8316C_LockRegister(DRV8316C_Handle_t *hdrv) {
	return DRV8316C_WriteRegister(hdrv, 0x3, 0x6);
}
// (나머지 함수들도 그대로 두시면 됩니다)
HAL_StatusTypeDef DRV8316C_ApplyDefaultConfig(DRV8316C_Handle_t *hdrv) {
	HAL_StatusTypeDef status;
	uint8_t reg_val;

	reg_val = DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
			| DRV_CTRL2_PWM_MODE_3X | DRV_CTRL2_CLR_FLT_BIT;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_2, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = DRV_CTRL3_PWM_100_DUTY_40KHZ | DRV_CTRL3_OVP_SEL_22V
			| DRV_CTRL3_OVP_EN | DRV_CTRL3_SPI_FLT_REP | DRV_CTRL3_OTW_REP;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_3, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = DRV_CTRL4_OCP_MODE_RETRY | DRV_CTRL4_OCP_LVL_16A
			| DRV_CTRL4_OCP_DEG_0_6us;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_4, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = DRV_CTRL5_CSA_GAIN_0_6 | DRV_CTRL5_EN_ASR_BIT
			| DRV_CTRL5_EN_AAR_BIT;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_5, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val =
	DRV_CTRL6_BUCK_PS_DIS | DRV_CTRL6_BUCK_SEL_5V | DRV_CTRL6_BUCK_DIS;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_6, reg_val);

	return status;
}

HAL_StatusTypeDef DRV8316C_ClearFaults(DRV8316C_Handle_t *hdrv) {
//	// (이전 코드와 동일)
//	DRV8316C_UnlockRegister(hdrv);
//	uint8_t reg_val = DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
//			| DRV_CTRL2_PWM_MODE_3X | DRV_CTRL2_CLR_FLT_BIT;
//	return DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_2, reg_val);
//	DRV8316C_LockRegister(hdrv);
	UNUSED(hdrv);
	return HAL_OK;
}

DRV8316C_REG_Typedef DRV8316C_VerifyConfig(DRV8316C_Handle_t *hdrv) {
	// (이전 코드와 동일, 내부에서 ReadRegister 호출하므로 자동 적용됨)
	DRV8316C_REG_Typedef status;
	uint8_t read_val = 0;
	uint8_t expected_val = 0;

	expected_val = DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
			| DRV_CTRL2_PWM_MODE_3X;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_2, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL2;

	expected_val = DRV_CTRL3_PWM_100_DUTY_40KHZ | DRV_CTRL3_OVP_SEL_22V
			| DRV_CTRL3_OVP_EN | DRV_CTRL3_SPI_FLT_REP | DRV_CTRL3_OTW_REP;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_3, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL3;

	expected_val = DRV_CTRL4_OCP_MODE_RETRY | DRV_CTRL4_OCP_LVL_16A
			| DRV_CTRL4_OCP_DEG_0_6us;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_4, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL4;

	expected_val = DRV_CTRL5_CSA_GAIN_0_6 | DRV_CTRL5_EN_ASR_BIT
			| DRV_CTRL5_EN_AAR_BIT;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_5, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL5;

	expected_val = DRV_CTRL6_BUCK_PS_DIS | DRV_CTRL6_BUCK_SEL_5V
			| DRV_CTRL6_BUCK_DIS;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_6, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL6;

	return REG_OK;
}

void Test_DRV8316C_Read_Status(DRV8316C_Handle_t *hdrv) {
	uint8_t status;
	LCD_Printf(0, 1, ST7789_WHITE, ST7789_BLACK, " nFAULT: %d",
			HAL_GPIO_ReadPin(hdrv->nFAULT_Port, hdrv->nFAULT_Pin));
	DRV8316C_ReadRegister(hdrv, DRV_REG_IC_STATUS, &status);
	LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "IC STATUS: %02X (%02X)",
			status, 0x00);
	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_1, &status);
	LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "STATUS 1: %02X (%02X)",
			status, 0x00);
	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_2, &status);
	LCD_Printf(0, 4, ST7789_WHITE, ST7789_BLACK, "STATUS 2: %02X (%02X)",
			status, 0x80);
	while (1) {
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_1, &status);
		LCD_Printf(0, 5, ST7789_WHITE, ST7789_BLACK, "CTRL 1: %02X (06)",
				status);
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_2, &status);
		LCD_Printf(0, 6, ST7789_WHITE, ST7789_BLACK, "CTRL 2: %02X (%02X)",
				status,
				DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
						| DRV_CTRL2_PWM_MODE_3X);
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_3, &status);
		LCD_Printf(0, 7, ST7789_WHITE, ST7789_BLACK, "CTRL 3: %02X (%02X)",
				status,
				DRV_CTRL3_PWM_100_DUTY_40KHZ | DRV_CTRL3_OVP_SEL_22V
						| DRV_CTRL3_OVP_EN | DRV_CTRL3_SPI_FLT_REP
						| DRV_CTRL3_OTW_REP);
		HAL_Delay(2000);
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_4, &status);
		LCD_Printf(0, 5, ST7789_WHITE, ST7789_BLACK, "CTRL 4: %02X (%02X)",
				status,
				DRV_CTRL4_OCP_MODE_RETRY | DRV_CTRL4_OCP_LVL_16A
						| DRV_CTRL4_OCP_DEG_0_6us);
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_5, &status);
		LCD_Printf(0, 6, ST7789_WHITE, ST7789_BLACK, "CTRL 5: %02X (%02X)",
				status,
				DRV_CTRL5_CSA_GAIN_0_6 | DRV_CTRL5_EN_ASR_BIT
						| DRV_CTRL5_EN_AAR_BIT);
		DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_6, &status);
		LCD_Printf(0, 7, ST7789_WHITE, ST7789_BLACK, "CTRL 6: %02X (%02X)",
				status,
				DRV_CTRL6_BUCK_PS_DIS | DRV_CTRL6_BUCK_SEL_5V
						| DRV_CTRL6_BUCK_DIS);
		HAL_Delay(2000);
	}
}

