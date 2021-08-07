/*
 * watchdogTrigger.c
 *
 *  Created on: Jul 10, 2021
 *      Author: Liam Flaherty
 */

#include "watchdogTrigger.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

#include "time/tasktimer/tasktimer.h"
#include "time/externalWatchdog/externalWatchdog.h"
#include "lib/logging/logging.h"

// ------------------- Private data -------------------
static Logging_T* log;

#define WDG_TRIGGER_STACK_SIZE 2000
static StaticTask_t taskBuffer;
static StackType_t taskStack[WDG_TRIGGER_STACK_SIZE];

// Task data
static TaskHandle_t wdgTaskHandle;

// ------------------- Private methods -------------------
static void WatchdogTrigger_TaskMain(void* pvParameters)
{
  logPrintS(log, "WatchdogTrigger_TaskMain begin\n", LOGGING_DEFAULT_BUFF_LEN);

  const TickType_t blockTime = 1 / portTICK_PERIOD_MS; // 1ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process
      // trigger the external watchdog to reset timer
      ExternalWatchdog_Trigger();
    }

  }
}

// ------------------- Public methods -------------------
WatchdogTrigger_Status_T WatchdogTrigger_Init(Logging_T* logger)
{
  log = logger;
  logPrintS(log, "WatchdogTrigger_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // create main task
  wdgTaskHandle = xTaskCreateStatic(
      WatchdogTrigger_TaskMain,
      "WatchdogTriggerTask",
      WDG_TRIGGER_STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      taskStack,
      &taskBuffer);

  // Register the task for timer notifications every 10ms
  uint16_t timerDivider = 10 * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&wdgTaskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return WATCHDOGTRIGGER_STATUS_ERROR;
  }

  logPrintS(log, "WatchdogTrigger_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return WATCHDOGTRIGGER_STATUS_OK;
}
