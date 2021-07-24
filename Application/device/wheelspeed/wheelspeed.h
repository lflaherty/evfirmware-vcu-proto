/*
 * wheelspeed.h
 *
 *  Created on: 7 May 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_WHEELSPEED_WHEELSPEED_H_
#define DEVICE_WHEELSPEED_WHEELSPEED_H_

#include "lib/logging/logging.h"

typedef enum
{
  WHEELSPEED_STATUS_OK     = 0x00U,
  WHEELSPEED_STATUS_ERROR  = 0x01U
} WheelSpeed_Status_T;

/**
 * @brief Initialize the process
 * @param logger Pointer to logging settings
 */
WheelSpeed_Status_T WheelSpeed_Init(Logging_T* logger);


#endif /* DEVICE_WHEELSPEED_WHEELSPEED_H_ */
