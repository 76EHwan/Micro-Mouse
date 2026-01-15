/*
 * DRV8316C.c
 *
 * Created on: Nov 10, 2025
 * Author: kth59
 */

#include "drv8316crq1.h"
#include "spi.h"

DRV8316C_Handle_t DRV8316C_L;
DRV8316C_Handle_t DRV8316C_R;

/* Macros for manual nCS pin control */
#define DRV8316C_CS_LOW(hdrv)     HAL_GPIO_WritePin((hdrv)->nCS_Port, (hdrv)->nCS_Pin, GPIO_PIN_RESET)
#define DRV8316C_CS_HIGH(hdrv)    HAL_GPIO_WritePin((hdrv)->nCS_Port, (hdrv)->nCS_Pin, GPIO_PIN_SET)

/*=======================================================================*/
/* Internal Helper Functions                                             */
/*=======================================================================*/

/**
 * @brief  Calculates the even parity bit by counting the number of '1's.
 * @note   Datasheet 8.5.1.1: "Parity bit is set such that the SDI input data word has even number of 1s"
 * @param  data: 16-bit SPI frame with parity bit (B8) cleared to 0.
 * @return 1 (if 1s count is odd), 0 (if 1s count is even)
 */
static uint8_t DRV8316C_CalculateEvenParity(uint16_t data) {
	uint8_t one_count = 0;
	// Clear the parity bit (B8) just in case, to count the other 15 bits
	data &= ~DRV_SPI_PARITY_BIT;

	for (int i = 0; i < 16; i++) {
		if ((data >> i) & 0x01) {
			one_count++;
		}
	}

	// If the count of 1s is odd, return 1 to make the total count even.
	return (one_count % 2);
}

/**
 * @brief  Internal helper function to perform SPI transmit/receive (8-bit x 2)
 * @note   Modified to send 2 bytes (8-bit mode) instead of 1 half-word (16-bit mode)
 */
static HAL_StatusTypeDef DRV8316C_SPI_TxRx(DRV8316C_Handle_t *hdrv,
		uint8_t *pTxData, uint8_t *pRxData) {
	HAL_StatusTypeDef status;

	DRV8316C_CS_LOW(hdrv); // Activate Chip Select (LOW)

	// Transmit 2 bytes (Size = 2).
	// HAL_SPI_TransmitReceive handles sending pTxData[0] then pTxData[1].
	status = HAL_SPI_TransmitReceive(hdrv->hspi, pTxData, pRxData, 2,
			100);

	DRV8316C_CS_HIGH(hdrv); // Deactivate Chip Select (HIGH)

	return status;
}

/*=======================================================================*/
/* Public Function Implementations                                       */
/*=======================================================================*/

/**
 * @brief  Initializes the DRV8316C handle.
 */
void DRV8316C_Init(DRV8316C_Handle_t *hdrv, SPI_HandleTypeDef *hspi,
		GPIO_TypeDef *nCS_Port, uint16_t nCS_Pin) {
	hdrv->hspi = hspi;
	hdrv->nCS_Port = nCS_Port;
	hdrv->nCS_Pin = nCS_Pin;

	// Set initial pin states
	DRV8316C_CS_HIGH(hdrv);  // nCS starts inactive (HIGH)
}

/**
 * @brief  Writes 8 bits of data to a specific DRV8316C register.
 */
HAL_StatusTypeDef DRV8316C_WriteRegister(DRV8316C_Handle_t *hdrv,
		uint8_t regAddr, uint8_t data) {
	uint16_t frame_16bit = 0;
	uint8_t tx_buff[2] = { 0 };
	uint8_t rx_buff[2] = { 0 };

	// 1. Construct the 16-bit frame first (R/W=0, Addr, Data)
	frame_16bit = DRV_SPI_WRITE_MASK
			| ((regAddr << DRV_SPI_ADDR_SHIFT) & DRV_SPI_ADDR_MASK)
			| (data & DRV_SPI_DATA_MASK);

	// 2. Calculate and set the even parity bit
	if (DRV8316C_CalculateEvenParity(frame_16bit)) {
		frame_16bit |= DRV_SPI_PARITY_BIT;
	}

	// 3. Split 16-bit frame into two 8-bit bytes (MSB First)
	tx_buff[0] = (uint8_t) ((frame_16bit >> 8) & 0xFF); // Upper byte
	tx_buff[1] = (uint8_t) (frame_16bit & 0xFF);        // Lower byte

	// 4. Transmit 2 bytes
	return DRV8316C_SPI_TxRx(hdrv, tx_buff, rx_buff);
}

/**
 * @brief  Reads 8 bits of data from a specific DRV8316C register.
 */
HAL_StatusTypeDef DRV8316C_ReadRegister(DRV8316C_Handle_t *hdrv,
		uint8_t regAddr, uint8_t *pData) {
	uint16_t frame_16bit = 0;
	uint8_t tx_buff[2] = { 0 };
	uint8_t rx_buff[2] = { 0 };
	HAL_StatusTypeDef status;

	// 1. Construct the read frame with R/W=1 and address
	frame_16bit = DRV_SPI_READ_MASK
			| ((regAddr << DRV_SPI_ADDR_SHIFT) & DRV_SPI_ADDR_MASK);

	// 2. Calculate and set the even parity bit
	if (DRV8316C_CalculateEvenParity(frame_16bit)) {
		frame_16bit |= DRV_SPI_PARITY_BIT;
	}

	// 3. Split into bytes (MSB First)
	tx_buff[0] = (uint8_t) ((frame_16bit >> 8) & 0xFF);
	tx_buff[1] = (uint8_t) (frame_16bit & 0xFF);

	// 4. Transmit/Receive 2 bytes
	status = DRV8316C_SPI_TxRx(hdrv, tx_buff, rx_buff);

	if (status == HAL_OK) {
		// 5. Reconstruct 16-bit received frame from 2 bytes
		uint16_t rx_frame = ((uint16_t) rx_buff[0] << 8) | rx_buff[1];

		// The lower 8 bits of the received frame contain the data
		*pData = (rx_frame & DRV_SPI_DATA_MASK);
	}

	return status;
}

// ... (나머지 Unlock, Lock, ApplyDefault, Verify 함수는 그대로 유지) ...
// ... (Helper 함수인 DRV8316C_SPI_TxRx가 바뀌었으므로 이 함수들을 호출하는 상위 로직은 수정 불필요) ...
// ... (아래는 참고용으로 변경된 부분이 없는 함수들입니다. 파일에 그대로 두시면 됩니다.) ...

HAL_StatusTypeDef DRV8316C_UnlockRegister(DRV8316C_Handle_t *hdrv) {
	return DRV8316C_WriteRegister(hdrv, 0x3, 0x3);
}

HAL_StatusTypeDef DRV8316C_LockRegister(DRV8316C_Handle_t *hdrv) {
	return DRV8316C_WriteRegister(hdrv, 0x3, 0x6);
}

HAL_StatusTypeDef DRV8316C_ApplyDefaultConfig(DRV8316C_Handle_t *hdrv) {
	// (이전 코드와 동일, 내부에서 WriteRegister를 호출하므로 자동 적용됨)
	HAL_StatusTypeDef status;
	uint8_t reg_val;

	reg_val = DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
			| DRV_CTRL2_PWM_MODE_3X;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_2, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = (1 << 2) | (1 << 0);
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_3, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = DRV_CTRL4_OCP_MODE_RETRY | DRV_CTRL4_OCP_LVL_16A | (1 << 4);
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_4, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = DRV_CTRL5_CSA_GAIN_0_6 | DRV_CTRL5_EN_ASR_BIT
			| DRV_CTRL5_EN_AAR_BIT;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_5, reg_val);
	if (status != HAL_OK)
		return status;

	reg_val = 1;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_6, reg_val);

	return status;
}

HAL_StatusTypeDef DRV8316C_ClearFaults(DRV8316C_Handle_t *hdrv) {
	// (이전 코드와 동일)
	HAL_StatusTypeDef status;
	uint8_t unlock_val = 0x03;
	status = DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_1, unlock_val);
	if (status != HAL_OK)
		return status;

	uint8_t reg_val = DRV_CTRL2_SDO_MODE_PP | DRV_CTRL2_SLEW_125V_us
			| DRV_CTRL2_PWM_MODE_3X | DRV_CTRL2_CLR_FLT_BIT;
	return DRV8316C_WriteRegister(hdrv, DRV_REG_CTRL_2, reg_val);
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

	expected_val = (1 << 2) | (1 << 0);
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_3, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL3;

	expected_val = DRV_CTRL4_OCP_MODE_RETRY | DRV_CTRL4_OCP_LVL_16A | (1 << 4);
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_4, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL4;

	expected_val = DRV_CTRL5_CSA_GAIN_0_6 | DRV_CTRL5_EN_ASR_BIT
			| DRV_CTRL5_EN_AAR_BIT;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_5, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL5;

	expected_val = 1;
	status = DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_6, &read_val);
	if (status != REG_OK || read_val != expected_val)
		return REG_FAULT_CTRL6;

	return REG_OK;
}

void MX_DRV8316C_Init() {
	HAL_GPIO_WritePin(MTR_nSLEEP_GPIO_Port, MTR_nSLEEP_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	SPI1_Config_For_DRV8316();
	HAL_Delay(10);

	DRV8316C_Init(&DRV8316C_L, &hspi1, MTR_L_CS_GPIO_Port, MTR_L_CS_Pin);
	DRV8316C_Init(&DRV8316C_R, &hspi1, MTR_R_CS_GPIO_Port, MTR_R_CS_Pin);

	DRV8316C_UnlockRegister(&DRV8316C_L);
	DRV8316C_UnlockRegister(&DRV8316C_R);

	DRV8316C_ApplyDefaultConfig(&DRV8316C_L);
	DRV8316C_ApplyDefaultConfig(&DRV8316C_R);

	DRV8316C_LockRegister(&DRV8316C_L);
	DRV8316C_LockRegister(&DRV8316C_R);

	if (DRV8316C_VerifyConfig(&DRV8316C_L) != REG_OK) {
	}
	if (DRV8316C_VerifyConfig(&DRV8316C_R) != REG_OK) {
	}

	SPI1_Config_For_ST7735();
}

