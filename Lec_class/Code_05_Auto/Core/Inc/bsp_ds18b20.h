#ifndef __BSP_DS18B20_H
#define __BSP_DS18B20_H

#include "stm32f1xx_hal.h"

#define OW_PIN_PORT		    GPIOB
#define OW_PIN				GPIO_PIN_1


#define OW_DIR_OUT() 	    mode_output1()
#define OW_DIR_IN() 	    mode_input1()
#define OW_OUT_LOW() 	    (HAL_GPIO_WritePin(OW_PIN_PORT, OW_PIN, GPIO_PIN_RESET))
#define OW_GET_IN()  	    (HAL_GPIO_ReadPin(OW_PIN_PORT, OW_PIN))
#define OW_SKIP_ROM 		0xCC
#define DS18B20_CONVERT 	0x44
#define DS18B20_READ 		0xBE


void DS18B20_Init(void);
uint16_t ds18b20_read(void);


#endif

