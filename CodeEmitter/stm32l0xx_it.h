/**
  ******************************************************************************
  * @file    Inc/stm32l0xx_it.h
  * @date    05/04/2015 10:49:21
  * @author  Alexis Duque
  * @version V0.0.1
  * @brief   This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L0xx_IT_H
#define __STM32L0xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USART2_IRQHandler(void);
void SysTick_Handler(void);

void SysTick_Init(void);
void TimeTick_Decrement(void);
void delay_nus(uint32_t n);
void delay_1ms(void);
void delay_nms(uint32_t n);


#ifdef __cplusplus
}
#endif

#endif /* __STM32L0xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
