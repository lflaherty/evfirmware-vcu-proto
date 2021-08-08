/*
 * example.h
 *
 * Example "Application" layer process
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#ifndef EXAMPLE_EXAMPLE_H_
#define EXAMPLE_EXAMPLE_H_

#include "lib/logging/logging.h"

typedef enum
{
  EXAMPLE_STATUS_OK     = 0x00U,
  EXAMPLE_STATUS_ERROR  = 0x01U
} Example_Status_T;

/**
 * @brief Initialize the process
 * @param logger Pointer to system logger
 * @param hcan Handle for CAN bus to use
 * @param huart Handle for UART device to use
 * @param hrtc Handle for RTC
 */
Example_Status_T Example_Init(
    Logging_T* logger,
    CAN_HandleTypeDef* hcan,
    UART_HandleTypeDef* huart,
    RTC_HandleTypeDef* hrtc);

#endif /* EXAMPLE_EXAMPLE_H_ */
