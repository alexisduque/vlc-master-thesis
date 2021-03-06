/**
  ******************************************************************************
  * @file    Src/stm32l0xx_it.c
  * @date    05/04/2015 10:49:21
  * @author  Alexis Duque
  * @version V0.0.1
  * @brief   Interrupt Service routine
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "stm32l0xx_it.h"
#include "main.h"

#define BITRATE_570KHZ 32000000
#define BITRATE_285KHZ 32000000

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

static __IO uint32_t sysTickCounter;
void EXTI4_15_IRQHandler(void);
extern UART_HandleTypeDef huart2;

/******************************************************************************/
/*            Cortex-M0+ Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
    TimeTick_Decrement();
    HAL_IncTick();
}

void SysTick_Init(void)
{
    /****************************************
     *SystemFrequency/1000      1ms         *
     *SystemFrequency/100000    10us        *
     *SystemFrequency/1000000   1us         *
     *****************************************/
    while (SysTick_Config(SystemCoreClock / 32000000) != 0)

    {
    } // One SysTick interrupt now equals 1us

}


/******************************************************************************/
/*                 STM32L0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l0xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to DMA stream
  *         used for USART data transmission
  */
void USART2_IRQHandler(void)
{
    /* USER CODE BEGIN USART2_IRQn 0 */

    /* USER CODE END USART2_IRQn 0 */
    HAL_UART_IRQHandler(&huart2);
    /* USER CODE BEGIN USART2_IRQn 1 */

    /* USER CODE END USART2_IRQn 1 */
}

void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}

/**
 * This method needs to be called in the SysTick_Handler
 */
void TimeTick_Decrement(void)
{
    if (sysTickCounter != 0x00)
    {
        sysTickCounter--;
    }
}

void delay_nus(uint32_t n)
{
    sysTickCounter = n;
    while (sysTickCounter != 0)
    {
    }
}

void delay_1ms(void)
{
    sysTickCounter = 1000;
    while (sysTickCounter != 0)
    {
    }
}

void delay_nms(uint32_t n)
{
    while (n--)
    {
        delay_1ms();
    }
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/***************************************************************END OF FILE****/
