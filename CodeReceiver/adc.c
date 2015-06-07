/**
  ******************************************************************************
  * File Name          : Src/adc.c
  * Date               : 05/04/2015 10:49:20
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "adc.h"
#include "math.h"
#include "4b6b.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define THRESHOLD 0
#define DETECTION_THRESHOLD 300
#define BIT_LENGTH 3
#define DMA_BUFFER_SIZE 512
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef AdcHandle;
DMA_HandleTypeDef DmaHandle;
UART_HandleTypeDef huart2;

FIFO_t AdcFIFO;
uint8_t AdcBuffer[BUFFER_SIZE];

uint32_t resultDMA[DMA_BUFFER_SIZE];
uint32_t tmp_resultDMA[DMA_BUFFER_SIZE];
uint8_t sampledDMA[DMA_BUFFER_SIZE/2];
uint32_t adcResultDMA;

uint8_t bitBuffer = 0x00;
int counter = 0;
long offset = 0;
unsigned char bit = 0;
float average, sum, sumTemp, variance, minVal, maxVal, noise;

uint32_t ADCVoltageValue = 0;
uint8_t ADCVoltageValue8 = 0;

char * val;
char * valBit;
char * valFiltered;

uint8_t buff = 0x00;
uint8_t * buff_ptr = &buff;

/* Private function prototypes -----------------------------------------------*/
double meanArray(uint32_t * adcArray, long arraySize);
long sumArray(uint32_t * adcArray, long arraySize);

/* ADC init function */
void MX_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    /* Peripheral clock enable */
    __ADC1_CLK_ENABLE();
    /* DMA controller clock enable */
    __DMA1_CLK_ENABLE();

    /**ADC GPIO Configuration
    PA0-WKUP     ------> ADC_IN0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/

    DmaHandle.Instance = DMA1_Channel1;
    DmaHandle.Init.Request = DMA_REQUEST_0;
    DmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    DmaHandle.Init.Mode = DMA_CIRCULAR;
    DmaHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_DeInit(&DmaHandle);
    HAL_DMA_Init(&DmaHandle);

    __HAL_LINKDMA(&AdcHandle, DMA_Handle, DmaHandle);

    /* DMA interrupt init */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    ADC_ChannelConfTypeDef sConfig;
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    AdcHandle.Instance = ADC1;
    AdcHandle.Init.OversamplingMode = DISABLE;
    AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV1;
    AdcHandle.Init.Resolution = ADC_RESOLUTION8b;
    AdcHandle.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    AdcHandle.Init.ScanDirection = ADC_SCAN_DIRECTION_UPWARD;
    AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.ContinuousConvMode = ENABLE;
    AdcHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIG_EDGE_NONE;
    AdcHandle.Init.DMAContinuousRequests = ENABLE;
    AdcHandle.Init.EOCSelection = EOC_SINGLE_CONV;
    AdcHandle.Init.Overrun = OVR_DATA_PRESERVED;
    AdcHandle.Init.LowPowerAutoWait = DISABLE;
    AdcHandle.Init.LowPowerFrequencyMode = DISABLE;
    AdcHandle.Init.LowPowerAutoOff = DISABLE;

    HAL_ADC_Init(&AdcHandle);
    HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);

    sConfig.Channel = ADC_CHANNEL_0;
    HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);

    /**Configure the analog watchdog
    */
    AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
    AnalogWDGConfig.Channel = ADC_CHANNEL_0;
    AnalogWDGConfig.ITMode = ENABLE;
    AnalogWDGConfig.HighThreshold = 0;
    AnalogWDGConfig.LowThreshold = 0;
    HAL_ADC_AnalogWDGConfig(&AdcHandle, &AnalogWDGConfig);

    HAL_ADC_Start_DMA(&AdcHandle, &resultDMA, DMA_BUFFER_SIZE);

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* AdcHandle)
{
    // Nothing to do there;
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* AdcHandle)
{

    if(AdcHandle->Instance==ADC1)
    {
        /* Peripheral clock disable */
        __ADC1_CLK_DISABLE();

        /**ADC GPIO Configuration
        PA0-WKUP     ------> ADC_IN0
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(AdcHandle->DMA_Handle);

    }
}

uint8_t ARA_ADC_Threashold(uint32_t voltage)
{
    if (voltage > 400) return 1;
    return (voltage > (minVal + DETECTION_THRESHOLD)) ? 1 : 0;
}

void compute_stat(uint32_t * buffer, int bufferSize)
{
    int i = 0;
    sum = 0;
    average =0;
    variance = 0;
    minVal = 0;
    maxVal = 0;
    sumTemp = 0;
    for (i = 0; i < bufferSize; i++)
    {
        sum += buffer[i];
        minVal = buffer[i] < minVal ? buffer[i] : minVal;
        maxVal = buffer[i] > maxVal ? buffer[i] : maxVal;
    }
    average = sum / (float)bufferSize;
    /*  Compute  variance  and standard deviation  */
    for (i = 0; i < bufferSize; i++)
    {
        sumTemp += powf((float)(buffer[i] - average), (float)2);
    }
    variance = sumTemp / (float)bufferSize;

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
    int i,y, preamb = 0;
    int succsiveOne = 0;
    int sample = 0;
    uint8_t filtered = 0;
    memcpy(&tmp_resultDMA, &resultDMA, DMA_BUFFER_SIZE * 4);

    compute_stat(tmp_resultDMA, DMA_BUFFER_SIZE);
    noise = variance < 1000 ? average : noise;
    for (i = 0; i < DMA_BUFFER_SIZE; i++)
    {
        sample++;
        ADCVoltageValue = tmp_resultDMA[i] / 16;

        int ADCVoltageValueUART =  tmp_resultDMA[i];
        val = itoa(ADCVoltageValueUART);
        #ifdef UART
        HAL_UART_Transmit(&huart2, (uint8_t*)val, strlen(val), 10);
        HAL_UART_Transmit(&huart2, (uint8_t*)&",", 2, 10);
        #endif
        uint8_t bit = 0;
        bit = ARA_ADC_Threashold(ADCVoltageValueUART);
        ADCVoltageValue8 = (uint8_t) ADCVoltageValue;

        /* USELESS
        uint8_t filtered = 0;
        filtered = ((ADCVoltageValueUART - average) > 0) ? (ADCVoltageValueUART - average) : 0;
        valFiltered = itoa(filtered);
        HAL_UART_Transmit(&huart2, (uint8_t*)valFiltered, strlen(valFiltered), 10);
        HAL_UART_Transmit(&huart2, (uint8_t*)&",", 2, 10);
        */

        valFiltered = bit == 0 ? itoa(1) : itoa(10);

        if (sample == (BIT_LENGTH))
        {
            sample = 0;
            if (bit == 0)
            {
                valFiltered = itoa(-1);
            }
            else
            {
                succsiveOne = succsiveOne++;
                valFiltered = 1;
            }
            sampledDMA[y] = bit;
            appendBit(bit, buff_ptr);
            if (*buff_ptr == CODE_4B6B_PRE2 && preamb == 0)
            {
                preamb = 1;
            }
            else if(*buff_ptr == CODE_4B6B_PRE2 && preamb == 1)
            {
                preamb = 0;
            }
            if (preamb == 1)
            {
                if (bit == 1)
                {
                    bitBuffer |= (1u << (7 - counter));
                }
                counter++;
                if (counter > 7)
                {
                    FIFO_write_trample(&AdcFIFO, &bitBuffer, 1);
                    counter = 0;
                    bitBuffer = 0x00;
                }

            }
            y = y++;
            #ifdef UART
            HAL_UART_Transmit(&huart2, (uint8_t*)valFiltered, strlen(valFiltered), 10);
            HAL_UART_Transmit(&huart2, (uint8_t*)&",", 2, 10);
            #endif
        }
        else
        {
            valFiltered = itoa(0);
            #ifdef UART
            HAL_UART_Transmit(&huart2, (uint8_t*)valFiltered, strlen(valFiltered), 10);
            HAL_UART_Transmit(&huart2, (uint8_t*)&",", 2, 10);
            #endif
        }

        valBit = itoa(bit);
        #ifdef UART
        HAL_UART_Transmit(&huart2, (uint8_t*)valBit, strlen(valBit), 10);
        HAL_UART_Transmit(&huart2, (uint8_t*)&"\r", 4, 10);
        #endif


        /* DISABLE FOR TEST
        bit = ARA_ADC_Threashold(ADCVoltageValue);
        if (bit == 1)
        {
            bitBuffer |= (1u << (7 - counter));
        }
        counter++;
        if (counter > 7)
        {
            FIFO_write_trample(&AdcFIFO, &bitBuffer, 1);
            counter = 0;
            bitBuffer = 0x00;
        }
        */
    }

//HAL_UART_Transmit(&huart2, (uint8_t*)&"\r", 4, 10);
}

/**
  * @}
  */

/**
  * @}
  */

/*****************************END OF FILE**************************************/
