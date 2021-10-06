/*
 * deviceMapping.h
 *
 *  Created on: 8 Aug. 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_
#define VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include "io/adc/adc.h" /* Needed for ADC_Channel_T */

/*
 * Include main for the auto-generated pin names
 */
#include "main.h"

/*
 * The ADC channels are read in the "rank" order defined in main.c
 * when the HAL library configures the ADC peripheral
 */
#define MAPPING_ADC_NUM_CHANNELS  ((uint16_t) 5U)
#define MAPPING_ADC1_CHANNEL0     ((ADC_Channel_T) 0U)
#define MAPPING_ADC1_CHANNEL1     ((ADC_Channel_T) 1U)
#define MAPPING_ADC1_CHANNEL2     ((ADC_Channel_T) 2U)
#define MAPPING_ADC1_CHANNEL3     ((ADC_Channel_T) 3U)
#define MAPPING_ADC1_CHANNEL4     ((ADC_Channel_T) 4U)

/*
 * Getters for device handles
 */

TIM_HandleTypeDef* Mapping_GetTaskTimer(void);
ADC_HandleTypeDef* Mapping_GetADC(void);
CAN_HandleTypeDef* Mapping_GetCAN1(void);
UART_HandleTypeDef* Mapping_GetUART1(void);
RTC_HandleTypeDef* Mapping_GetRTC(void);

#endif /* VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_ */
