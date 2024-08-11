/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ov2640.h"
//#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE {
	while ((UART4->SR & 0X40) == 0)
		;
	UART4->DR = (uint8_t) ch;
	return ch;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

void DCMI_DMA_MemInc_En(void);
void DCMI_DMA_MemInc_Den(void);
void PY_OV2640_RGB565_CONFIG(void);
uint8_t aRxBuffer=0;
uint8_t TxBuff[99] = {0};

uint8_t ov2640_verh = 0xff, ov2640_verl = 0xff;

HAL_StatusTypeDef dcmi_dma_status = HAL_OK;

uint32_t dcmi_data_buff[16000] = { 0 };
//11328
uint32_t DCMI_RN = 0;  //row number
uint32_t DCMI_CN = 0;  //column number
uint32_t DCMI_RS = 0;  //row start
uint32_t DCMI_CS = 0;  //column start

uint8_t scmd = 0;
uint8_t tx_busy = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
  PY_OV2640_RGB565_CONFIG();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
//		DCMI_DMA_MemInc_En();
//		dcmi_dma_status = HAL_DCMI_Init(&hdcmi);
//
//		for (uint8_t i = 0; i < 10; i++) {
//			HAL_DCMI_DisableCrop(&hdcmi);
//
//			DCMI_RN = 48;
//			DCMI_CN = 236*2;
//
//			DCMI_RS = 48 * i;
//			DCMI_CS = 0;
//
//			HAL_DCMI_ConfigCrop(&hdcmi, DCMI_CS, DCMI_RS, DCMI_CN, DCMI_RN);
//			HAL_Delay(1);
//			HAL_DCMI_EnableCrop(&hdcmi);
//			HAL_Delay(1);
//
//			dcmi_dma_status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT,
//					dcmi_data_buff, DCMI_CN * DCMI_RN / 4);
//
//			while (HAL_DMA_GetState(&hdcmi) == HAL_DMA_STATE_BUSY)
//				;
//
//			HAL_DCMI_Stop(&hdcmi);
//			HAL_UART_Transmit_DMA(&huart4, (uint8_t*) dcmi_data_buff, 22656);
//			HAL_Delay(20000);
//
//
//		}
//
				HAL_Delay(1);
				DCMI_DMA_MemInc_En();
				dcmi_dma_status = HAL_DCMI_Init(&hdcmi);
	    	 for (uint8_t i=0; i<10;i++)
	    	  {
		 		     HAL_DCMI_DisableCrop (&hdcmi);

		 	    	 DCMI_RN = 48;
		 	    	 DCMI_CN = 1280;

		 	    	 DCMI_RS = 48*i;

		 	    	 DCMI_CS = 0;

		 	    	 HAL_DCMI_ConfigCrop (&hdcmi, DCMI_CS, DCMI_RS, DCMI_CN, DCMI_RN);
		 	    	 HAL_Delay(1);
		 	    	 HAL_DCMI_EnableCrop (&hdcmi);
		 	    	 HAL_Delay(1);

		 	    	 dcmi_dma_status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint8_t*)dcmi_data_buff,DCMI_CN*DCMI_RN/4);
	                 while(HAL_DMA_GetState(&hdcmi)==HAL_DMA_STATE_BUSY) ;
	 	 	    	 HAL_DCMI_Stop(&hdcmi);
	 	 	    	 tx_busy = 1;
	 	 	    	HAL_UART_Transmit_DMA(&huart4, (uint8_t *)dcmi_data_buff, 61440);
	 	 	    	while(tx_busy!=0) ;




	    	 }



			    	 HAL_Delay(500000);
















		//HAL_Delay(20000);


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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);
}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMI_Init(void)
{

  /* USER CODE BEGIN DCMI_Init 0 */

  /* USER CODE END DCMI_Init 0 */

  /* USER CODE BEGIN DCMI_Init 1 */

  /* USER CODE END DCMI_Init 1 */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALTERNATE_2_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMI_Init 2 */

  /* USER CODE END DCMI_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PG9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PG15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart ==  &huart4)
	{

		if (aRxBuffer==0x01)
		{
			scmd = 0x02;
			aRxBuffer=0x00;

			HAL_UART_Receive_IT(&huart4, (uint8_t *)&aRxBuffer, 1);
		}
		else
		{

		HAL_UART_Receive_IT(&huart4, (uint8_t *)&aRxBuffer, 1);

		}


	}
      return;

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

	if (huart ==  &huart4)
	{
		tx_busy = 0;
	}

}


void DCMI_DMA_MemInc_En(void) {
	HAL_DMA_DeInit(&hdma_dcmi);

	hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
	if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
		Error_Handler();
	}
}

void DCMI_DMA_MemInc_Den(void) {
	HAL_DMA_DeInit(&hdma_dcmi);
	hdma_dcmi.Init.MemInc = DMA_MINC_DISABLE;
	if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK) {
		Error_Handler();
	}
}

void PY_OV2640_RGB565_CONFIG(void) {
	/*Camera Interface*/
	SCCB_Rst();     //hard reset
	HAL_Delay(100);

	//SCCB_WR_Reg(0xff, 0x01);   //soft reset
	//SCCB_WR_Reg(0x12, 0x80);
	//HAL_Delay(100);

	ov2640_verh = SCCB_RD_Reg(0x1c);
	HAL_Delay(50);
	ov2640_verl = SCCB_RD_Reg(0x1d);
	HAL_Delay(50);

	while ((ov2640_verh == 0xff) || (ov2640_verl == 0xff)) {
		//HAL_UART_Transmit(&huart1, &ov2640_verh, 1, 0xFFFFFF);
		HAL_Delay(500);
		//HAL_UART_Transmit(&huart1, &ov2640_verl, 1, 0xFFFFFF);
		HAL_Delay(500);
	}

	OV2640_UXGA_Init();

	//pix speed adjustment
	SCCB_WR_Reg(0xff, 0x00);
	SCCB_WR_Reg(0xd3, 0x00);
	SCCB_WR_Reg(0XFF, 0X01);
	SCCB_WR_Reg(0X11, 0x01);

	OV2640_RGB565_Mode();

	OV2640_OutSize_Set(640, 480);
	HAL_Delay(200);

}

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
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

