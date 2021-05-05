/*
 * adc.h
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#ifndef IO_ADC_ADC_H_
#define IO_ADC_ADC_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef enum
{
  ADC_STATUS_OK         = 0x00U,
  ADC_STATUS_ERROR_DMA  = 0x01U
} ADC_Status_T;

typedef enum
{
  ADC1_CHANNEL0   = 0x00U,
  ADC1_CHANNEL1   = 0x01U,
  ADC1_CHANNEL2   = 0x02U,
  ADC1_CHANNEL3   = 0x03U,
  ADC1_CHANNEL4   = 0x04U,
} ADC_Channel_T;

/**
 * @brief Initialize ADC driver interface
 */
ADC_Status_T ADC_Init(void);

/**
 * @brief Configure ADC device
 * This should be called from main. Main will retain ownership of handle ptr.
 * @param handle Handle for ADC device
 *
 * @return Return status. ADC_STATUS_OK for success. See ADC_Status_T for more.
 */
ADC_Status_T ADC_Config(ADC_HandleTypeDef* handle);

/**
 * @brief Gets the latest reading from the given ADC channel
 *
 * @returns Raw ADC reading (12-bit)
 */
uint16_t ADC_Get(const ADC_Channel_T channel);


#endif /* IO_ADC_ADC_H_ */