/*
 * adc.c
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#include "adc.h"

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ADC_NUM_CHANNELS 5  /* Number of channels used */
#define ADC_NUM_SAMPLES 1   /* Number of samples to take per channel */
#define ADC_BUF_LEN (ADC_NUM_CHANNELS * ADC_NUM_SAMPLES)

/**
 * Used by DMA to store data
 */
static volatile uint16_t adcDMABuf[ADC_BUF_LEN];

/**
 * Used to store final ADC results
 */
static volatile uint16_t adcData[ADC_NUM_CHANNELS];


// ------------------- Public methods -------------------
ADC_Status_T ADC_Init(void)
{
  printf("ADC_Init begin\n");
  // Initialize buffers to 0 (can't use memset due to volatile)
  size_t i;
  for (i = 0; i < ADC_BUF_LEN; ++i) {
    adcDMABuf[i] = 0;
  }
  for (i = 0; i < ADC_NUM_CHANNELS; ++i) {
    adcData[i] = 0;
  }

  printf("ADC_Init complete\n");
  return ADC_STATUS_OK;
}

//------------------------------------------------------------------------------
// TODO rename to ADC_Start
ADC_Status_T ADC_Config(ADC_HandleTypeDef* handle)
{
  printf("ADC_Config begin\n");
  // TODO: this only works for ADC1 (with multiple channels), expand to ADCx

  // just need to start DMA
  if (HAL_ADC_Start_DMA(handle, (uint32_t*)adcDMABuf, ADC_BUF_LEN) != HAL_OK) {
    return ADC_STATUS_ERROR_DMA;
  }

  printf("ADC_Config complete\n");
  return ADC_STATUS_OK;
}

//------------------------------------------------------------------------------
uint16_t ADC_Get(const ADC_Channel_T channel)
{
  return adcData[channel];
}


// ------------------- Interrupts -------------------
/**
 * @brief Called when first half of buffer is filled
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  // Empty
}

/**
 * @brief Called when buffer is completely filled
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  // TODO: average results collected in this window?

  // copy results into final array
  size_t i;
  for (i = 0; i < ADC_NUM_CHANNELS; ++i) {
    adcData[i] = adcDMABuf[i] & 0xFFF;  // 12 bit, it should be this anyway, but make sure
  }
}
