/*
 * deviceMapping.c
 *
 *  Created on: 8 Aug. 2021
 *      Author: Liam Flaherty
 */

#include "deviceMapping.h"
#include "stm32f7xx_hal.h"


extern TIM_HandleTypeDef htim2;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;

extern RTC_HandleTypeDef hrtc;

extern UART_HandleTypeDef huart1;

TIM_HandleTypeDef* Mapping_GetTaskTimer(void)
{
  return &htim2;
}

ADC_HandleTypeDef* Mapping_GetADC(void)
{
  return &hadc1;
}


CAN_HandleTypeDef* Mapping_GetCAN1(void)
{
  return &hcan1;
}


UART_HandleTypeDef* Mapping_GetUART1(void)
{
  return &huart1;
}


RTC_HandleTypeDef* Mapping_GetRTC(void)
{
  return &hrtc;
}

