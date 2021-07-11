/*
 * externalWatchdog.c
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#include "externalWatchdog.h"

#include <stdio.h>

#include "stm32f7xx_hal.h"
#include "main.h"

ExternalWatchdog_Status_T ExternalWatchdog_Init(void)
{
  printf("ExternalWatchdog_Init begin\n");

  // set pin state
  HAL_GPIO_WritePin(WATCHDOG_MR_GPIO_Port, WATCHDOG_MR_Pin, GPIO_PIN_RESET);

  printf("ExternalWatchdog_Init complete\n");
  return EXTWATCHDOG_STATUS_OK;
}

ExternalWatchdog_Status_T ExternalWatchdog_Trigger(void)
{
  HAL_GPIO_TogglePin(WATCHDOG_MR_GPIO_Port, WATCHDOG_MR_Pin);
  return EXTWATCHDOG_STATUS_OK;
}
