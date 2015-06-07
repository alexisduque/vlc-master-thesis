/**
  ******************************************************************************
  * File Name          : Src/i2c.c
  * Date               : 05/04/2015 10:49:20
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "adc.h"
#include "main.h"

/* Private macro -------------------------------------------------------------*/
#define I2C_ADDRESS             0x50
#define MASTER_REQ_READ         0x01
#define MASTER_REQ_WRITE        0x02
#define MASTER_REQ_STOP         0x03
#define I2C_TIMING_100KHZ       0x00A03D53
#define I2C_TIMING_400KHZ       0x00500615
#define I2C_TIMING_32_400KHZ    0x00B0122A
#define I2C_TIMING_32_SFAST     0x0040060D
#define SLAVE_TX_ENABLED        0x01
#define SLAVE_TX_DISABLED       0x00

/* Private variables ---------------------------------------------------------*/
uint8_t slaveTxEnabled = SLAVE_TX_DISABLED;
/* I2C handler declaration */
I2C_HandleTypeDef I2CxHandle;

/* Buffer used for transmission */
uint8_t aTxBuffer[TXBUFFERSIZE];
uint16_t aTxSize = TXBUFFERSIZE;
/* Buffer used for reception */
uint8_t aRxBuffer[RXBUFFERSIZE];
uint16_t aRxSize = RXBUFFERSIZE;
uint16_t fifo_size = 0;
uint8_t bTransferRequest = 0;
HAL_StatusTypeDef read = 0;

/* Private function prototypes -----------------------------------------------*/
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);

/* Private functions ---------------------------------------------------------*/

/* I2C1 init function */
void MX_I2C1_Init(void)
{

    I2Cx_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = I2Cx_SCL_AF;

    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SDA_AF;

    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

    /* NVIC for I2C1 */
    HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2Cx_IRQn);

    I2CxHandle.Instance = I2C1;
    I2CxHandle.Init.Timing =  0x00500615;
    I2CxHandle.Init.OwnAddress1 = I2C_ADDRESS;
    I2CxHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2CxHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    I2CxHandle.Init.OwnAddress2 = 0;
    I2CxHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2CxHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    I2CxHandle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
    if(HAL_I2C_Init(&I2CxHandle) != HAL_OK)
    {
        Error_Handler();
    }

    /**Configure Analogue filter
    */
    HAL_I2CEx_AnalogFilter_Config(&I2CxHandle, I2C_ANALOGFILTER_ENABLED);
    Flush_Buffer((uint8_t*)aTxBuffer, aTxSize);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *I2CxHandle)
{

}

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *I2CxHandle)
{
    I2Cx_FORCE_RESET();
    I2Cx_RELEASE_RESET();

    HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
    HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(I2C1_IRQn);
}

void ARA_I2C_Listen(void)
{
    HAL_I2C_Slave_Transmit_IT(&I2CxHandle, (uint8_t*)aTxBuffer, aTxSize);
}

void HAL_I2C_SlaveTxCpltCallback (I2C_HandleTypeDef *I2CxHandle)
{
    Flush_Buffer((uint8_t*)aTxBuffer, aTxSize);
    RES_t res = FIFO_read(&AdcFIFO, &aTxBuffer, aTxSize);
    if (res != RES_OK) {
        printf("Empty Buffer");
    }
}

void HAL_I2C_SlaveRxCpltCallback (I2C_HandleTypeDef *I2CxHandle)
{
    if(aRxBuffer[0] == MASTER_REQ_READ)
    {
        slaveTxEnabled = SLAVE_TX_ENABLED;
    }
    else if(aRxBuffer[0] == MASTER_REQ_STOP)
    {
        slaveTxEnabled = SLAVE_TX_DISABLED;
    }
    Flush_Buffer((uint8_t*)aRxBuffer, aRxSize);
}

/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
    while(BufferLength--)
    {
        *pBuffer = 1;
        pBuffer++;
    }
}

/**
  * @brief  I2C error callbacks
  * @param  I2cHandle: I2C handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2CxHandle)
{
    /* Turn On LED2 */
    Error_Handler();
}

/**
  * @}
  */

/**
  * @}
  */

/*****************************END OF FILE**************************************/
