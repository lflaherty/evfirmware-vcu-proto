/*
 * wheelspeed.c
 *
 *  Created on: 7 May 2021
 *      Author: Liam Flaherty
 */

#include "wheelspeed.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "main.h" /* Fetch auto-generated GPIO names */

#include "time/tasktimer/tasktimer.h"
#include "lib/logging/logging.h"

// ------------------- Private data -------------------
static Logging_T* log;

#define STACK_SIZE 2000
static StaticTask_t taskBuffer;
static StackType_t taskStack[STACK_SIZE];

// Task data
static TaskHandle_t wheelSpeedTaskHandle;
// ------------------- Private methods -------------------
static void WheelSpeed_TaskMain(void* pvParameters)
{
  logPrintS(log, "WheelSpeed_TaskMain begin\n", LOGGING_DEFAULT_BUFF_LEN);

  const TickType_t blockTime = 10 / portTICK_PERIOD_MS; // 10ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process

      // Just toggle this pin really quickly
      HAL_GPIO_TogglePin(SPEED_TEST_GPO_GPIO_Port, SPEED_TEST_GPO_Pin);
    }

  }
}


// ------------------- Public methods -------------------
WheelSpeed_Status_T WheelSpeed_Init(Logging_T* logger)
{
  log = logger;
  logPrintS(log, "WheelSpeed_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // create main task
  wheelSpeedTaskHandle = xTaskCreateStatic(
      WheelSpeed_TaskMain,
      "WheelSpeedTask",
      STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      taskStack,
      &taskBuffer);

  // Register the task for timer notifications every 1ms
  uint16_t timerDivider = 1 * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&wheelSpeedTaskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return WHEELSPEED_STATUS_ERROR;
  }

  logPrintS(log, "WheelSpeed_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return WHEELSPEED_STATUS_OK;
}
