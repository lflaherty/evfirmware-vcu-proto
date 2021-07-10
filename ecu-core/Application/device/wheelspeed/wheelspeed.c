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

// ------------------- Private data -------------------
#define STACK_SIZE 128
static StaticTask_t taskBuffer;
static StackType_t taskStack[STACK_SIZE];

// Task data
static TaskHandle_t wheelSpeedTaskHandle;
// ------------------- Private methods -------------------
static void WheelSpeed_TaskMain(void* pvParameters)
{
  printf("WheelSpeed_TaskMain begin\n");

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
WheelSpeed_Status_T WheelSpeed_Init(void)
{
  printf("WheelSpeed_Init begin\n");

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

  printf("WheelSpeed_Init complete\n");
  return WHEELSPEED_STATUS_OK;
}
