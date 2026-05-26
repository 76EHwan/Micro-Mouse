/*
 * drv8316crq1.h
 *
 * Created on: Nov 10, 2025
 * Author: kth59
 */

#ifndef DEVICES_INC_DRV8316CRQ1_H_
#define DEVICES_INC_DRV8316CRQ1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "spi.h"
#include "tim.h"

#define DRV8316C_SPI &hspi2
#define DRV8316C_L_TIM &htim1
#define DRV8316C_R_TIM &htim8

/* Macros for manual nCS pin control */
#define DRV8316C_CS_LOW(hdrv)     HAL_GPIO_WritePin((hdrv)->nCS_Port, (hdrv)->nCS_Pin, GPIO_PIN_RESET)
#define DRV8316C_CS_HIGH(hdrv)    HAL_GPIO_WritePin((hdrv)->nCS_Port, (hdrv)->nCS_Pin, GPIO_PIN_SET)

/* Macros for manual nSLEEP pin control */
#define DRV8316C_WAKEUP			  HAL_GPIO_WritePin(MTR_nSLEEP_GPIO_Port, MTR_nSLEEP_Pin, GPIO_PIN_SET)
#define DRV8316C_SLEEP			  HAL_GPIO_WritePin(MTR_nSLEEP_GPIO_Port, MTR_nSLEEP_Pin, GPIO_PIN_RESET)

/* Macros for manual DRVOFF pin control */
#define DRV8316C_DRVOFF_LOW(hdrv)     HAL_GPIO_WritePin((hdrv)->DRVOFF_Port, (hdrv)->DRVOFF_Pin, GPIO_PIN_RESET)
#define DRV8316C_DRVOFF_HIGH(hdrv)    HAL_GPIO_WritePin((hdrv)->DRVOFF_Port, (hdrv)->DRVOFF_Pin, GPIO_PIN_SET)

/*=======================================================================*/
/* SPI Frame Definition */
/*=======================================================================*/
// 16-bit Frame: [W(B15)] | [A5-A0(B14-B9)] | [P(B8)] | [D7-D0(B7-B0)]
#define DRV_SPI_READ_MASK       (0x8000)  // B15 = 1 (Read)
#define DRV_SPI_WRITE_MASK      (0x0000)  // B15 = 0 (Write)
#define DRV_SPI_ADDR_SHIFT      (9)       // Address 6-bit (B14-B9)
#define DRV_SPI_ADDR_MASK       (0x3F << DRV_SPI_ADDR_SHIFT) // 6-bit mask
#define DRV_SPI_PARITY_BIT      (1 << 8)  // Parity bit (B8)
#define DRV_SPI_DATA_MASK       (0x00FF)  // Data 8-bit (B7-B0)

/*=======================================================================*/
/* DRV8316CR Register Address Map */
/*=======================================================================*/
// Status Registers (Read-Only)
#define DRV_REG_IC_STATUS       (0x00) // IC Status Register
#define DRV_REG_STATUS_1        (0x01) // Status Register 1
#define DRV_REG_STATUS_2        (0x02) // Status Register 2

// Control Registers (Read/Write)
#define DRV_REG_CTRL_1          (0x03) // Control Register 1 (REG_LOCK)
#define DRV_REG_CTRL_2          (0x04) // Control Register 2 (PWM_MODE, SLEW, CLR_FLT)
#define DRV_REG_CTRL_3          (0x05) // Control Register 3 (OVP_EN, OTW_REP)
#define DRV_REG_CTRL_4          (0x06) // Control Register 4 (OCP_MODE, OCP_LVL)
#define DRV_REG_CTRL_5          (0x07) // Control Register 5 (CSA_GAIN, EN_ASR, EN_AAR)
#define DRV_REG_CTRL_6          (0x08) // Control Register 6 (BUCK_SEL, BUCK_DIS)
#define DRV_REG_CTRL_10         (0x0C) // Control Register 10 (DLYCMP_EN)

/*=======================================================================*/
/* DRV8316CR Status Register Bit-Fields (Read-Only)                      */
/*=======================================================================*/

/* IC Status Register (Offset = 0h) */
#define DRV_IC_STATUS_BK_FLT    (1 << 6) // Buck Regulator Fault
#define DRV_IC_STATUS_SPI_FLT   (1 << 5) // SPI Fault
#define DRV_IC_STATUS_OCP       (1 << 4) // Over Current Protection Status
#define DRV_IC_STATUS_NPOR      (1 << 3) // Supply Power On Reset (1=OK, 0=Reset detected)
#define DRV_IC_STATUS_OVP       (1 << 2) // Supply Overvoltage Protection Status
#define DRV_IC_STATUS_OT        (1 << 1) // Overtemperature Fault Status
#define DRV_IC_STATUS_FAULT     (1 << 0) // Device Fault (1 if any other fault is active)

/* Status Register 1 (Offset = 1h) */
#define DRV_STATUS1_OTW         (1 << 7) // Overtemperature Warning
#define DRV_STATUS1_OTS         (1 << 6) // Overtemperature Shutdown
#define DRV_STATUS1_OCP_HC      (1 << 5) // OCP High-side C
#define DRV_STATUS1_OCP_LC      (1 << 4) // OCP Low-side C
#define DRV_STATUS1_OCP_HB      (1 << 3) // OCP High-side B
#define DRV_STATUS1_OCP_LB      (1 << 2) // OCP Low-side B
#define DRV_STATUS1_OCP_HA      (1 << 1) // OCP High-side A
#define DRV_STATUS1_OCP_LA      (1 << 0) // OCP Low-side A

/* Status Register 2 (Offset = 2h) */
#define DRV_STATUS2_OTP_ERR     (1 << 6) // OTP Error
#define DRV_STATUS2_BUCK_OCP    (1 << 5) // Buck Regulator OCP
#define DRV_STATUS2_BUCK_UV     (1 << 4) // Buck Regulator Undervoltage
#define DRV_STATUS2_VCP_UV      (1 << 3) // Charge Pump Undervoltage
#define DRV_STATUS2_SPI_PARITY  (1 << 2) // SPI Parity Error
#define DRV_STATUS2_SPI_SCLK_FLT (1 << 1) // SPI Clock Framing Error
#define DRV_STATUS2_SPI_ADDR_FLT (1 << 0) // SPI Address Error

/*=======================================================================*/
/* DRV8316CR Control Register Bit-Fields (R/W)                           */
/*=======================================================================*/

/* Control Register 2 (Offset = 4h) */
#define DRV_CTRL2_SDO_MODE_OD   (0 << 5) // SDO Open Drain
#define DRV_CTRL2_SDO_MODE_PP   (1 << 5) // SDO Push Pull
#define DRV_CTRL2_SLEW_25V_us   (0 << 3) // 25 V/us
#define DRV_CTRL2_SLEW_50V_us   (1 << 3) // 50 V/us
#define DRV_CTRL2_SLEW_125V_us  (2 << 3) // 125 V/us
#define DRV_CTRL2_SLEW_200V_us  (3 << 3) // 200 V/us
#define DRV_CTRL2_PWM_MODE_6X   (0 << 1) // 6x PWM Mode
#define DRV_CTRL2_PWM_MODE_6X_CL (1 << 1) // 6x PWM Mode w/ Current Limit
#define DRV_CTRL2_PWM_MODE_3X   (2 << 1) // 3x PWM Mode
#define DRV_CTRL2_PWM_MODE_3X_CL (3 << 1) // 3x PWM Mode w/ Current Limit
#define DRV_CTRL2_CLR_FLT_BIT   (1 << 0) // Write 1 to clear faults

/* Control Register 3 (Offset = 5h) */
#define DRV_CTRL3_PWM_100_DUTY_40KHZ  (1 << 4) // Overtemperature Warning Reporting
#define DRV_CTRL3_OVP_SEL_22V   (1 << 3) // Overtemperature Warning Reporting
#define DRV_CTRL3_OVP_EN        (1 << 2) // OVP Enable
#define DRV_CTRL3_SPI_FLT_REP   (1 << 1) // Overtemperature Warning Reporting
#define DRV_CTRL3_OTW_REP       (1 << 0) // Overtemperature Warning Reporting


/* Control Register 4 (Offset = 6h) */
#define DRV_CTRL4_OCP_RETRY_5ms (0 << 3) // 5 ms OCP retry
#define DRV_CTRL4_OCP_RETRY_500ms (1 << 3) // 500 ms OCP retry
#define DRV_CTRL4_OCP_LVL_16A   (0 << 2) // OCP 16A
#define DRV_CTRL4_OCP_LVL_24A   (1 << 2) // OCP 24A
#define DRV_CTRL4_OCP_MODE_LATCH (0 << 0) // OCP Latched fault
#define DRV_CTRL4_OCP_MODE_RETRY (1 << 0) // OCP Auto Retry
#define DRV_CTRL4_OCP_MODE_REPORT (2 << 0) // OCP Report Only
#define DRV_CTRL4_OCP_MODE_DISABLED (3 << 0) // OCP Disabled
#define DRV_CTRL4_OCP_DEG_0_6us (1 << 4)  // OCP Deglitch time (default is higher)

/* Control Register 5 (Offset = 7h) */
#define DRV_CTRL5_EN_AAR_BIT    (1 << 3) // Active Asynchronous Rectification
#define DRV_CTRL5_EN_ASR_BIT    (1 << 2) // Active Synchronous Rectification
#define DRV_CTRL5_CSA_GAIN_0_15 (0 << 0) // 0.15 V/A
#define DRV_CTRL5_CSA_GAIN_0_3  (1 << 0) // 0.3 V/A
#define DRV_CTRL5_CSA_GAIN_0_6  (2 << 0) // 0.6 V/A
#define DRV_CTRL5_CSA_GAIN_1_2  (3 << 0) // 1.2 V/A

/* Control Register 6 (Offset = 8h) */
#define DRV_CTRL6_BUCK_PS_DIS   (1 << 4) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_CL       (1 << 3) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_SEL_3V3  (0 << 1) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_SEL_5V   (1 << 1) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_SEL_4V   (2 << 1) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_SEL_5V7  (3 << 1) // Buck Regulator Disable
#define DRV_CTRL6_BUCK_DIS      (1 << 0) // Buck Regulator Disable


/*=======================================================================*/
/* Driver Handle Structure                                               */
/*=======================================================================*/
typedef enum {
	REG_OK,
	REG_FAULT_CTRL1,
	REG_FAULT_CTRL2,
	REG_FAULT_CTRL3,
	REG_FAULT_CTRL4,
	REG_FAULT_CTRL5,
	REG_FAULT_CTRL6,
	REG_FAULT_CTRL10
} DRV8316C_REG_Typedef;

typedef struct
{
    SPI_HandleTypeDef* hspi;       // Pointer to the SPI peripheral handle
    GPIO_TypeDef* nCS_Port;   // GPIO Port for nCS (Chip Select) pin
    uint16_t           nCS_Pin;    // nCS pin number
    GPIO_TypeDef* nSLEEP_Port; // GPIO Port for nSLEEP pin
    uint16_t           nSLEEP_Pin;   // nSLEEP pin number

    GPIO_TypeDef* DRVOFF_Port;
    uint16_t DRVOFF_Pin;

    GPIO_TypeDef* nFAULT_Port;
    uint16_t nFAULT_Pin;

} DRV8316C_Handle_t;

extern DRV8316C_Handle_t DRV8316C_L;
extern DRV8316C_Handle_t DRV8316C_R;

/*=======================================================================*/
/* Function Prototypes                                                   */
/*=======================================================================*/

/**
 * @brief  Initializes the DRV8316C handle.
 */

void DRV8316C_Init(DRV8316C_Handle_t *hdrv, SPI_HandleTypeDef *hspi, GPIO_TypeDef *nCS_Port, uint16_t nCS_Pin,
		 GPIO_TypeDef *nFAULT_Port, uint16_t nFAULT_Pin,  GPIO_TypeDef *DRVOFF_Port, uint16_t DRVOFF_Pin);

/**
 * @brief  Writes 8 bits of data to a specific DRV8316C register.
 */
HAL_StatusTypeDef DRV8316C_WriteRegister(DRV8316C_Handle_t* hdrv, uint8_t regAddr, uint8_t data);

/**
 * @brief  Reads 8 bits of data from a specific DRV8316C register.
 */
HAL_StatusTypeDef DRV8316C_ReadRegister(DRV8316C_Handle_t* hdrv, uint8_t regAddr, uint8_t* pData);

/**
 * @brief  Activates the DRV8316C driver (sets nSLEEP pin High).
 */
HAL_StatusTypeDef DRV8316C_UnlockRegister(DRV8316C_Handle_t* hdrv);

/**
 * @brief  Deactivates the DRV8316C driver (sets nSLEEP pin Low).
 */
HAL_StatusTypeDef DRV8316C_LockRegister(DRV8316C_Handle_t* hdrv);

/**
 * @brief  Applies a common default configuration to the DRV8316C.
 */
HAL_StatusTypeDef DRV8316C_ApplyDefaultConfig(DRV8316C_Handle_t* hdrv);

/**
 * @brief  Clears all fault flags (using CLR_FLT bit in CTRL_2).
 */
HAL_StatusTypeDef DRV8316C_ClearFaults(DRV8316C_Handle_t* hdrv);

/**
 * @brief  Verifies if the default configuration matches the actual register values.
 */
DRV8316C_REG_Typedef DRV8316C_VerifyConfig(DRV8316C_Handle_t *hdrv);

/**
 * @brief  Initializes global DRV8316 instances and SPI configuration.
 */
void MX_DRV8316C_Init(void);

void Test_DRV8316C_Read_Status(DRV8316C_Handle_t *hdrv);

#ifdef __cplusplus
}
#endif

#endif /* DEVICES_INC_DRV8316CRQ1_H_ */
