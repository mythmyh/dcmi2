#include "stm32f4xx_hal.h"
#ifndef _OV2640_H
#define _OV2640_H

//for not open-drain bus
/*
 * SIOC: PB0
 * SIOD: PB1
 * VSYNC: PB7
 * HREF: PA4
 * PCLK: PA6
 * XCLK: PA8  //24MHz, optional to use
 * D7: PC6
 * D6: PC7
 * D5: PE0
 * D4: PE1
 * D3: PE4
 * D2: PB6
 * D1: PE5
 * D0: PE6
 * RESET: PD10
 * PWDN: PD11
 *
 *
 *
 * SIOC: PD6
 * SIOD: PD7
 * VSYNC: PB7
 * HREF: PA4
 * PCLK: PA6
 * XCLK: PA8  //24MHz, optional to use
 * D7: PE6
 * D6: PE5
 * D5: PB6
 * D4: PC11
 * D3: PC9
 * D2: PC8
 * D1: PC7
 * D0: PC6
 * RESET: PG15
 * PWDN: PD3
 *
 *
 */
#define SCCB_SCL_L    		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_RESET)
#define SCCB_SCL_H    		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_SET)
#define SCCB_SDA_L    		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_RESET)
#define SCCB_SDA_H    		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_SET)

#define SCCB_READ_SDA    	HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7)
#define SCCB_ID_W   	    0X60  			//OV2640 ID for Write
#define SCCB_ID_R   	    0X61  			//OV2640 ID for Read

#define OV2640_PWDN           HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET)
#define OV2640_PWUP           HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET)
#define OV2640_RST  	      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_15,GPIO_PIN_RESET)
#define OV2640_RUN  	      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_15,GPIO_PIN_SET)
#define OV2640_VSYNC 	      HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)
#define OV2640_HREF  	      HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)
#define OV2640_PCLK  	      HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)


void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
uint8_t SCCB_WR_Byte(uint8_t data);
uint8_t SCCB_RD_Byte(void);
uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data);
uint8_t SCCB_RD_Reg(uint8_t reg);
uint32_t tickdelay;

void SCCB_SDA_IN(void);
void SCCB_SDA_OUT(void);

#define ticknumber 12*10

void SCCB_Rst(void);


/***********************************/
void OV2640_Auto_Exposure(uint8_t level);
void OV2640_Light_Mode(uint8_t mode);
void OV2640_Color_Saturation(uint8_t sat);
void OV2640_Brightness(uint8_t bright);
void OV2640_Contrast(uint8_t contrast);
void OV2640_Special_Effects(uint8_t eft);
void OV2640_Color_Bar(uint8_t sw);
void OV2640_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);
uint8_t OV2640_OutSize_Set(uint16_t width,uint16_t height);
uint8_t OV2640_ImageWin_Set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height);
uint8_t OV2640_ImageSize_Set(uint16_t width,uint16_t height);
void OV2640_RGB565_Mode(void);
void OV2640_JPEG_Mode(void);		//JPEG模式

void OV2640_UXGA_Init(void);
   
#endif
