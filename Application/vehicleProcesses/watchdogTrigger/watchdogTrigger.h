/*
 * watchdogTrigger.h
 *
 * Continuously resets the watchdog timers.
 *
 *  Created on: Jul 10, 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEPROCESSES_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_
#define VEHICLEPROCESSES_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_

#include "lib/logging/logging.h"

typedef enum
{
  WATCHDOGTRIGGER_STATUS_OK     = 0x00U,
  WATCHDOGTRIGGER_STATUS_ERROR  = 0x01U
} WatchdogTrigger_Status_T;

/**
 * @brief Initialize the process
 */
WatchdogTrigger_Status_T WatchdogTrigger_Init(Logging_T* logger);

#endif /* VEHICLEPROCESSES_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_ */
