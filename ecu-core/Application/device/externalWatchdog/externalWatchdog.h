/*
 * externalWatchdog.h
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_
#define DEVICE_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_

typedef enum
{
  EXTWATCHDOG_STATUS_OK     = 0x00U
} ExternalWatchdog_Status_T;

/**
 * @brief Initialize the external Watchdog Timer.
 */
ExternalWatchdog_Status_T ExternalWatchdog_Init(void);

/**
 * @brief Trigger the external watchdog timer to avoid reset.
 * Call this periodically.
 */
ExternalWatchdog_Status_T ExternalWatchdog_Trigger(void);

#endif /* DEVICE_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_ */
