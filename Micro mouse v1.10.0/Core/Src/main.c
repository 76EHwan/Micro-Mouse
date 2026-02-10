/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "flash.h"
#include "gpdma.h"
#include "i2c.h"
#include "icache.h"
#include "lptim.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "init.h"
#include "menu.h"
#include "error.h"
#include "sensor.h"
#include "vl53l4cx.h"
#include "drv8316crq1.h"
#include "foc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint32_t us) {
	// 현재 사이클 카운트 저장
	uint32_t startTick = DWT->CYCCNT;

	// 필요한 사이클 수 계산 (us * (HCLK / 1,000,000))
	// SystemCoreClock은 현재 MCU의 주파수(Hz)를 담고 있습니다.
	uint32_t delayTicks = us * (SystemCoreClock / 1000000);

	// 목표 사이클에 도달할 때까지 대기
	while ((DWT->CYCCNT - startTick) < delayTicks)
		;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* Configure the peripherals common clocks */
	PeriphCommonClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_GPDMA1_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();
	MX_FLASH_Init();
	MX_I2C2_Init();
	MX_ICACHE_Init();
	MX_LPTIM1_Init();
	MX_SPI1_Init();
	MX_SPI2_Init();
	MX_TIM1_Init();
	MX_TIM4_Init();
	MX_TIM8_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM5_Init();
	MX_TIM6_Init();
	MX_TIM7_Init();
	MX_TIM15_Init();
	MX_TIM12_Init();
	MX_SPI3_Init();
	/* USER CODE BEGIN 2 */
	TRIG_ON;

	MX_User_Init();
	TRIG_OFF;
	LCD_Printf(0, 0, ST7789_WHITE, ST7789_BLACK, "Hello world!");
	HAL_Delay(1000);
	HAL_GPIO_WritePin(MTR_L_DRVOFF_GPIO_Port, MTR_L_DRVOFF_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MTR_R_DRVOFF_GPIO_Port, MTR_R_DRVOFF_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	ADC1_Start();
	ADC2_Start();
	IMU_Start();
	FOC_Start(&focL);
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
//		Test_DRV8316C_Read_Status(&DRV8316C_R);
		uint8_t ctrl2_val = 0;
		// 레지스터 읽기 (READ 함수가 정상 동작한다고 가정)
		DRV8316C_ReadRegister(&DRV8316C_L, DRV_REG_CTRL_2, &ctrl2_val);

		// 3x PWM 모드라면 비트 0:1이 '01' 또는 '10'이어야 함.
		// 0x00(00b) = 6x PWM Mode (현재 증상의 원인)
		LCD_Printf(0, 3, ST7789_WHITE, ST7789_BLACK, "CTRL2: 0x%02X",
				ctrl2_val);
		if (ctrl2_val != 0x34) {
			HAL_GPIO_WritePin(MTR_INLx_GPIO_Port, MTR_INLx_Pin, GPIO_PIN_RESET);
			DRV8316C_UnlockRegister(&DRV8316C_L);
			DRV8316C_ApplyDefaultConfig(&DRV8316C_L);
			DRV8316C_LockRegister(&DRV8316C_L);
			HAL_GPIO_WritePin(MTR_INLx_GPIO_Port, MTR_INLx_Pin, GPIO_PIN_SET);
		}
		DRV8316C_ReadRegister(&DRV8316C_L, DRV_REG_IC_STATUS, &ctrl2_val);
		LCD_Printf(0, 4, ST7789_WHITE, ST7789_BLACK, "IC: 0x%02X", ctrl2_val);
		DRV8316C_ReadRegister(&DRV8316C_L, DRV_REG_STATUS_1, &ctrl2_val);
		LCD_Printf(0, 5, ST7789_WHITE, ST7789_BLACK, "ST2: 0x%02X", ctrl2_val);
		DRV8316C_ReadRegister(&DRV8316C_L, DRV_REG_STATUS_2, &ctrl2_val);
		LCD_Printf(0, 6, ST7789_WHITE, ST7789_BLACK, "ST2: 0x%02X", ctrl2_val);
		static uint8_t step = 0;
		uint16_t pwm_val = htim8.Instance->ARR / 20; // 힘을 좀 더 강하게 (약 64%)

		// 1. 상태 모니터링
		int fault = HAL_GPIO_ReadPin(MTR_L_nFAULT_GPIO_Port, MTR_R_nFAULT_Pin);

		LCD_Printf(0, 0, ST7789_WHITE, ST7789_BLACK, "MTR Check");
		LCD_Printf(0, 1, ST7789_WHITE, ST7789_BLACK, "FAULT:%d", fault);
		LCD_Printf(0, 2, ST7789_WHITE, ST7789_BLACK, "Step:%d PWM:%d", step,
				pwm_val);

		// 2. 강제 3상 스텝 구동 (0.5초마다 이동)
		switch (step) {
		case 0: // [Step 1] 0°: U-High (V, W는 Low로 전류가 빠져나감)
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			break;

		case 1: // [Step 2] 60°: U-High, V-High
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			break;

		case 2: // [Step 3] 120°: V-High
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			break;

		case 3: // [Step 4] 180°: V-High, W-High
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_val);
			break;

		case 4: // [Step 5] 240°: W-High
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_val);
			break;

		case 5: // [Step 6] 300°: W-High, U-High
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_val);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_val);
			break;
		}

		step = (step + 1) % 6;
		TRIG_TOGGLE;
		HAL_Delay(100);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV2;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 100;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 3;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}

	/** Configure the programming delay
	 */
	__HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void) {
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

	/** Initializes the peripherals clock
	 */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER
			| RCC_PERIPHCLK_ADCDAC;
	PeriphClkInitStruct.PLL2.PLL2Source = RCC_PLL2_SOURCE_HSE;
	PeriphClkInitStruct.PLL2.PLL2M = 5;
	PeriphClkInitStruct.PLL2.PLL2N = 64;
	PeriphClkInitStruct.PLL2.PLL2P = 2;
	PeriphClkInitStruct.PLL2.PLL2Q = 2;
	PeriphClkInitStruct.PLL2.PLL2R = 5;
	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2_VCIRANGE_2;
	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2_VCORANGE_WIDE;
	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	PeriphClkInitStruct.PLL2.PLL2ClockOut = RCC_PLL2_DIVR;
	PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
	PeriphClkInitStruct.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_PLL2R;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
//	__disable_irq();
	ST7789_FillScreen(ST7789_RED);
	LCD_Printf(0, 0, ST7789_BLACK, ST7789_RED, error_log);
	while (1) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(100);
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
