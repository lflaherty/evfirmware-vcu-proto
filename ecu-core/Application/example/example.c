/*
 * example.c
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#include "example.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "comm/can/can.h"
#include "io/adc/adc.h"

// ------------------- Private data -------------------
static unsigned int count = 0;

// GPIO pins
#define LED_STATUS_Pin GPIO_PIN_12
#define LED_STATUS_GPIO_Port GPIOB


// ------------------- Private methods -------------------
static void Example_TaskMain(void* pvParameters)
{
  while (1) {
    TickType_t ticks = 1000 / portTICK_PERIOD_MS;
    vTaskDelay(ticks ? ticks : 1);

    HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);

    // setup CAN data
    uint8_t TxData[8];
    TxData[0] = (count >> 8) & 0xFF;
    TxData[1] = count & 0xFF;
    TxData[2] = 0x68;
    TxData[3] = 0x04;
    TxData[4] = 0x20;
    TxData[5] = 0xAF;

    /* Start the Transmission process */
    CAN_SendMessage(CAN1, 0x5A1, TxData, 8);

    count++;
    uint16_t voltage = (330 * ADC_Get(ADC1_CHANNEL3)) / 4096;
    printf("Count %d\tADC %d - %dV (x100)\n", count, ADC_Get(ADC1_CHANNEL3), voltage);
  }
}

static void Example_canCallback(const CAN_DataFrame_T* data)
{
  printf("CAN received from %lx: ", data->canId);
  size_t i;
  for (i = 0; i < data->dlc; ++i)
  {
    printf(" %x", data->data[i]);
  }
  printf("\n");
}

// ------------------- Public methods -------------------
Example_Status_T Example_Init(void)
{
  // Register to receive messages from CAN1
  CAN_RegisterCallback(CAN1, Example_canCallback);

  // create main task
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;
  xReturned = xTaskCreate(
      Example_TaskMain,
      "MAIN",
      128,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      &xHandle);

  if (xReturned != pdPASS)
  {
    printf("Failed to create main task\n");
    return EXAMPLE_STATUS_ERROR;
  }

  return EXAMPLE_STATUS_OK;
}
