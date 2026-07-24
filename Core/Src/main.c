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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>
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
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DS18B20_PORT DS18B20_GPIO_Port
#define DS18B20_PIN  DS18B20_Pin

static void DWT_Delay_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void DelayUs(volatile uint32_t us)
{
	uint32_t start = DWT->CYCCNT;
	uint32_t cycles = us * (HAL_RCC_GetHCLKFreq() / 1000000);
	while ((DWT->CYCCNT - start) < cycles);
}

static uint8_t OneWire_Reset(void)
{
	uint8_t presence = 0;

	//OneWire_SetOutput();

	DS18B20_PORT->BSRR = (uint32_t)DS18B20_PIN << 16U;

	//HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
	DelayUs(480);

	//OneWire_SetInput();
	//HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
	DS18B20_PORT->BSRR = DS18B20_PIN;
	DelayUs(80);

	if ((DS18B20_PORT->IDR & DS18B20_PIN)== 0)
	{
		presence = 1;

	}
	DelayUs(400);
	return presence;

	//presence = (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET) ? 1 : 0;
	//DelayUs(410);
	//return presence;
}

static void OneWire_WriteBit(uint8_t bit)
{
	//OneWire_SetOutput();
	//HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
	DS18B20_PORT->BSRR = (uint32_t)DS18B20_PIN << 16U;

	if (bit)
	{
		DelayUs(6);
		//OneWire_SetInput();
		DS18B20_PORT->BSRR = DS18B20_PIN;
		DelayUs(64);

	}
	else
	{
		DelayUs(60);
		//OneWire_SetInput();
		DS18B20_PORT->BSRR = DS18B20_PIN;
		DelayUs(10);
	}
}

static uint8_t OneWire_ReadBit(void)
{
	uint8_t bit = 0;

	//OneWire_SetOutput();

	//HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
	DS18B20_PORT->BSRR = (uint32_t)DS18B20_PIN << 16U;
	DelayUs(2);

	//OneWire_SetInput();

	//HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
	DS18B20_PORT->BSRR = DS18B20_PIN;
	DelayUs(13);

	if((DS18B20_PORT->IDR & DS18B20_PIN) != 0)
	{
		bit = 1;

	}

	DelayUs(45);
	return bit;

}

static void OneWire_WriteByte(uint8_t byte)
{
	for (uint8_t i=0; i<8; i++)
	{
		OneWire_WriteBit(byte & 0x01);
		byte >>=1;
	}
}

static uint8_t OneWire_ReadByte(void)
{
	uint8_t byte = 0;
	for(uint8_t i=0; i<8; i++)
	{
		byte >>= 1;
		if(OneWire_ReadBit())
			byte |= 0x80;

	}
	return byte;
}

static uint8_t DS18B20_StartConversion(void)
{
	/*if((DS18B20_PORT->IDR & DS18B20_PIN)== 0)
	{
	return 0;
	}*/

	if (!OneWire_Reset())
		return 0;

	OneWire_WriteByte(0xCC);
	OneWire_WriteByte(0x44);
	return 1;

}
static float DS18B20_ReadTemperature(uint8_t *raw_byte0)
{
	uint8_t temp_lsb =0;
	uint8_t temp_msb =0;

	if (!OneWire_Reset())
		return -999.0f;

	OneWire_WriteByte(0xCC);
	OneWire_WriteByte(0xBE);
	//DelayUs(10);

	/*if(!OneWire_Reset())
		return -999.0f;

	OneWire_WriteByte(0xCC);
	OneWire_WriteByte(0x44);
	OneWire_WriteByte(0xCC);
	OneWire_WriteByte(0xBE);*/

	temp_lsb = OneWire_ReadByte();
	temp_msb = OneWire_ReadByte();

	if(raw_byte0 != NULL){
		*raw_byte0 = temp_lsb;
	}

	int16_t raw_temp = (temp_msb << 8) | temp_lsb;
	float temperature = raw_temp / 16.0f;

	return temperature;

}
//static void OneWire_SetOutput(void)
//{
	/*GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin= DS18B20_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);*/
	//DS18B20_PORT->MODER &= ~(0x3 << (1*2));
	//DS18B20_PORT->MODER |=  (0x1 <<(1*2));


//}


//static void OneWire_SetInput(void)
//{
	/*GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DS18B20_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);*/
	//DS18B20_PORT->MODER &= ~(0x3 << (1*2));

//}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */



  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  DWT_Delay_Init();

  HAL_Delay(2000);
  char clk_msg[80];
  sprintf(clk_msg, "BMS Project Initialized. HCLK: %lu Hz\r\n", HAL_RCC_GetHCLKFreq());
  CDC_Transmit_FS((uint8_t*)clk_msg, strlen(clk_msg));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*uint32_t raw_idr = GPIOA->IDR;
	  char debug_txt[60];

	  // Print the exact binary state of the first few pins of Port A
	   sprintf(debug_txt, "Port A IDR Register Raw Bits: 0x%08LX\r\n", raw_idr);
	   CDC_Transmit_FS((uint8_t*)debug_txt, strlen(debug_txt));

	   //mirror to led
	   if ((raw_idr & GPIO_PIN_1) !=0)
	   {
		   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // LED ON
	   }
	   else
	   {
		   HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);   // LED OFF
	   }

	   HAL_Delay(3000);*/

	  /*if ((GPIOA->IDR & GPIO_PIN_1) !=0)
	  {
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	  }
	  else
	  {
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  }

	  HAL_Delay(50);*/
	  //Toggle LED every cycle to see execution status
	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	  //1. Process Analog Readings(ADC)
	  HAL_ADC_Start(&hadc1);
	  if (HAL_ADC_PollForConversion(&hadc1, 10)==HAL_OK)
	  {
		  uint32_t raw_adc = HAL_ADC_GetValue(&hadc1);
		  float adc_voltage = (raw_adc / 4095.0f) * 3.3f;

		  float temperature = -999.0f;
		  uint8_t test_byte = 0x00;

		  //2. Process Digital 1-wire temperature sensor
		  uint8_t conv_started = DS18B20_StartConversion();

		  char msg[150];
		  if(conv_started)
		  {
			  HAL_Delay(1000); // DS18B20 needs time to compute temperature
			  temperature = DS18B20_ReadTemperature(&test_byte);
			  sprintf(msg, "ADC: %.3fV | Temp: %.2fC | RawByte0: 0x%02X\r\n", adc_voltage, temperature, test_byte);

		  }
		  else
		  {
			  sprintf(msg, "ADC: %.3fV | Sensor Status: DISCONNECTED / RESET FAILED\r\n", adc_voltage);
		   }

		  //3. Telemetry output over USB virtual COM port
		  CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
	  }
	  HAL_ADC_Stop(&hadc1);
	  //uint8_t presence = OneWire_Reset();
	  //char debug_msg[50];
	  //sprintf(debug_msg, "Presence Pulse: %d\r\n", presence);
	  //CDC_Transmit_FS((uint8_t*)debug_msg, strlen(debug_msg));

	  //safe place delay prevents flooding tera term
	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DS18B20_GPIO_Port, DS18B20_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DS18B20_Pin */
  GPIO_InitStruct.Pin = DS18B20_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(DS18B20_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
 /* GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);*/

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
