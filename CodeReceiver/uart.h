/**
  ******************************************************************************
  * @file    Inc/uart.h
  * @author  Alexis Duque
  * @version V0.0.1
  * @date    30-March-2015
  * @brief   Main ADC conversion with buffering
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern UART_HandleTypeDef huart2;

void MX_UART2_UART_Init(void);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
char *itoa(int value);

#endif /* __UART_H */

/*****************************END OF FILE**************************************/
