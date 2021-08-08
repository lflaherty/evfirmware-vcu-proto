/*
 * deviceMapping.h
 *
 *  Created on: 8 Aug. 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_
#define VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_

#include "stm32f7xx_hal.h"

/*
 * Include main for the auto-generated pin names
 */
#include "main.h"

/*
 * Getters for device handles
 */

TIM_HandleTypeDef* Mapping_GetTaskTimer(void);
ADC_HandleTypeDef* Mapping_GetADC(void);
CAN_HandleTypeDef* Mapping_GetCAN1(void);
UART_HandleTypeDef* Mapping_GetUART1(void);
RTC_HandleTypeDef* Mapping_GetRTC(void);

#endif /* VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_ */
