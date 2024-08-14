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
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ov2640.h"
#define HEIGHT 480
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define SRAM_ADDR_BEGIN 0x68000000UL
#define SRAM_ADDR_HALF 0x68080000UL
#define SRAM_ADDR_END 0x680FFFFFUL
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
static  uint32_t testsram[160000]  __attribute__((section(".sram")));


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

extern ApplicationTypeDef Appli_state;
extern USBH_HandleTypeDef hUsbHostFS;
extern char USBHPath[4];
FATFS otgupan;
FIL myFile;

uint32_t testsram2[16000]={0};
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

//uint32_t dcmi_data_buff[16000] = { 0 };
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

SRAM_HandleTypeDef hsram3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_UART4_Init(void);
static void MX_FSMC_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#pragma pack(2)

typedef struct tagBITMAPFILEHEADER3{
	WORD bfType;//位图文件的类型，在Windows中，此字段的值�?�为‘BM�?????(1-2字节�?????
	DWORD bfSize;//位图文件的大小，以字节为单位�?????3-6字节，低位在前）
	WORD bfReserved1;//位图文件保留字，必须�?????0(7-8字节�?????
	WORD bfReserved2;//位图文件保留字，必须�?????0(9-10字节�?????
	DWORD bfOffBits;//位图数据的起始位置，以相对于位图�?????11-14字节，低位在前）
	//文件头的偏移量表示，以字节为单位
} BitMapFileHeader;	//BITMAPFILEHEADER;
#pragma pack()
typedef struct tagBITMAPINFOHEADER3{
	DWORD biSize;//本结构所占用字节数（15-18字节�?????
	LONG biWidth;//位图的宽度，以像素为单位�?????19-22字节�?????
	LONG biHeight;//位图的高度，以像素为单位�?????23-26字节�?????
	WORD biPlanes;//目标设备的级别，必须�?????1(27-28字节�?????
	WORD biBitCount;//每个像素�?????�?????的位数，必须�?????1（双色），（29-30字节�?????
	//4(16色）�?????8(256色）16(高彩�?????)�?????24（真彩色）之�?????
	DWORD biCompression;//位图压缩类型，必须是0（不压缩），�?????31-34字节�?????
	//1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之�?????
	DWORD biSizeImage;//位图的大�?????(其中包含了为了补齐行数是4的�?�数而添加的空字�?????)，以字节为单位（35-38字节�?????
	LONG biXPelsPerMeter;//位图水平分辨率，像素数（39-42字节�?????
	LONG biYPelsPerMeter;//位图垂直分辨率，像素数（43-46字节)
	DWORD biClrUsed;//位图实际使用的颜色表中的颜色数（47-50字节�?????
	DWORD biClrImportant;//位图显示过程中重要的颜色数（51-54字节�?????
} BitMapInfoHeader;	//BITMAPINFOHEADER;
typedef struct tagRGBQUAD2{
	BYTE rgbBlue;//蓝色的亮度（值范围为0-255)
	BYTE rgbGreen;//绿色的亮度（值范围为0-255)
	BYTE rgbRed;//红色的亮度（值范围为0-255)
	BYTE rgbReserved;//保留，必须为0
} RgbQuad2;	//RGBQUAD;

int Rgb565ConvertBmp(uint8_t* buf,int width,int height,FIL * fp)
{

	BitMapFileHeader bmfHdr; //定义文件�?????
	BitMapInfoHeader bmiHdr; //定义信息�?????
	RgbQuad2 bmiClr[3]; //定义调色�?????

	bmiHdr.biSize = sizeof(BitMapInfoHeader);
	bmiHdr.biWidth = width;//指定图像的宽度，单位是像�?????
	bmiHdr.biHeight = height;//指定图像的高度，单位是像�?????
	bmiHdr.biPlanes = 1;//目标设备的级别，必须�?????1
	bmiHdr.biBitCount = 16;//表示用到颜色时用到的位数 16位表示高彩色�?????
	bmiHdr.biCompression = 3L;//BI_RGB仅有RGB555格式
	bmiHdr.biSizeImage = (width * height * 2);//指定实际位图�?????占字节数
	bmiHdr.biXPelsPerMeter = 0;//水平分辨率，单位长度内的像素�?????
	bmiHdr.biYPelsPerMeter = 0;//垂直分辨率，单位长度内的像素�?????
	bmiHdr.biClrUsed = 0;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用�?????有调色板项）
	bmiHdr.biClrImportant = 0;//说明对图象显示有重要影响的颜色索引的数目�?????0表示�?????有颜色都重要

	//RGB565格式掩码
	bmiClr[0].rgbBlue = 0;
	bmiClr[0].rgbGreen = 0xF8;
	bmiClr[0].rgbRed = 0;
	bmiClr[0].rgbReserved = 0;

	bmiClr[1].rgbBlue = 0xE0;
	bmiClr[1].rgbGreen = 0x07;
	bmiClr[1].rgbRed = 0;
	bmiClr[1].rgbReserved = 0;

	bmiClr[2].rgbBlue = 0x1F;
	bmiClr[2].rgbGreen = 0;
	bmiClr[2].rgbRed = 0;
	bmiClr[2].rgbReserved = 0;


	bmfHdr.bfType = (WORD)0x4D42;//文件类型�?????0x4D42也就是字�?????'BM'
	bmfHdr.bfSize = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + sizeof(RgbQuad2) * 3 + bmiHdr.biSizeImage);//文件大小
	bmfHdr.bfReserved1 = 0;//保留，必须为0
	bmfHdr.bfReserved2 = 0;//保留，必须为0
	bmfHdr.bfOffBits = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader)+ sizeof(RgbQuad2) * 3);//实际图像数据偏移�?????
	uint32_t byteswritten;



	f_write(fp,&bmfHdr,  sizeof(BitMapFileHeader),(void *)&byteswritten);
	f_write(fp,&bmiHdr, sizeof(BitMapInfoHeader),(void *)&byteswritten);
	f_write(fp,&bmiClr, 3*sizeof(RgbQuad2),(void *)&byteswritten);
	for(int i=0; i<height; i++){
		f_write(fp,buf+(width*(height-i-1)*2), 2*width,(void *)&byteswritten);
	}

    f_close( fp );

	 return 0;
}

static FRESULT ETX_MSC_ProcessUsbDevice(char * filename,uint8_t * data)
{
  FATFS     UsbDiskFatFs;                                 /* File system object for USB disk logical drive */
  char      UsbDiskPath[4] = {0};                         /* USB Host logical drive path */
  FIL       file;                                         /* File object */
  FRESULT   res;                                          /* FatFs function common result code */
  DWORD     fre_clust;                                    /* Freee Cluster */

  do
  {
    /* Register the file system object to the FatFs module */
    res = f_mount( &UsbDiskFatFs, (TCHAR const*)UsbDiskPath, 0 );
    if( res != FR_OK )
    {
      /* FatFs Init Error */
      break;
    }

    /* Check the Free Space */
    FATFS *fatFs = &UsbDiskFatFs;
    f_getfree("", &fre_clust, &fatFs);


    /* Create a new text file with write access */
    res = f_open( &file, filename, ( FA_CREATE_ALWAYS | FA_WRITE ) );
    if( res != FR_OK )
    {
      /* File Open Error */
      break;
    }

    /* Write the data to the text file */
    Rgb565ConvertBmp(data,640,480,&file);




    /* Close the file */
    f_close(&file);

    if(res != FR_OK)
    {
      /* File Read Error */
      break;
    }

    /* Print the data */

  } while ( 0 );

  /* Unmount the device */
  f_mount(NULL, UsbDiskPath, 0);

  /* Unlink the USB disk driver */
  FATFS_UnLinkDriver(UsbDiskPath);

  return res;
}

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
  MX_FSMC_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */
  PY_OV2640_RGB565_CONFIG();
for(int ts=0;ts<160000;ts++)testsram[ts]=ts;
while(APPLICATION_READY!=Appli_state)
MX_USB_HOST_Process();
char filename[6];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

				HAL_Delay(1);
				DCMI_DMA_MemInc_En();
	    	 for (uint8_t i=0; i<5;i++)
	    	  {

		 		     HAL_DCMI_DisableCrop (&hdcmi);
		 	    	 DCMI_RN = HEIGHT;
		 	    	 DCMI_CN = 1280;
		 	    	 DCMI_RS = i*HEIGHT;
		 	    	 int offset=1280*HEIGHT*i;
		 	    	 DCMI_CS = 0;
		 	    	 HAL_DCMI_ConfigCrop (&hdcmi, DCMI_CS, DCMI_RS, DCMI_CN, DCMI_RN);
		 	    	 HAL_Delay(1);
		 	    	 HAL_DCMI_EnableCrop (&hdcmi);
		 	    	 HAL_Delay(1);
		 	    	 dcmi_dma_status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (testsram+offset),DCMI_CN*DCMI_RN/4);
	                 while(HAL_DMA_GetState(&hdcmi)==HAL_DMA_STATE_BUSY){} ;
	 	 	    	 HAL_DCMI_Stop(&hdcmi);
	    	 	 	   HAL_Delay(1000);

		                 sprintf(filename,"%d.bmp",i);

	    	 	 	 ETX_MSC_ProcessUsbDevice(filename,(uint8_t*)testsram);
	    	 	 	HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_9);
	    	 }






	    	 HAL_Delay(5000000);

//		for(int i=0;i<15;i+=1){
//
//			printf("read data %d=%lu,testsram  %p\n",i,testsram[i],&testsram[i]);
//
//
//	    	 HAL_Delay(2000);
//
//		}



















		//HAL_Delay(20000);


    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

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
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
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
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

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

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
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

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM3 memory initialization sequence
  */
  hsram3.Instance = FSMC_NORSRAM_DEVICE;
  hsram3.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram3.Init */
  hsram3.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram3.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram3.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram3.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram3.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram3.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram3.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram3.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram3.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram3.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram3.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram3.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram3.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram3.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 0;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 4;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram3, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
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

