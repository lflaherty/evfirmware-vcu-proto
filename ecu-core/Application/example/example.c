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

#define EX_STACK_SIZE 128
static StaticTask_t taskBuffer;
static StackType_t taskStack[EX_STACK_SIZE];

// GPIO pins
#define LED_STATUS_Pin GPIO_PIN_12
#define LED_STATUS_GPIO_Port GPIOB


// TODO pass this as a parameter
extern CAN_HandleTypeDef hcan1;


// ------------------- Private methods -------------------
static void Example_TaskMain(void* pvParameters)
{
  printf("Example_TaskMain begin\n");
  while (1) {
    TickType_t ticks = 500 / portTICK_PERIOD_MS;
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
    CAN_SendMessage(&hcan1, 0x5A1, TxData, 8);

    count++;
    uint16_t voltage = (330 * ADC_Get(ADC1_CHANNEL3)) / 4096;
    printf("Count %d\tADC %d - %dV (x100)\n", count, ADC_Get(ADC1_CHANNEL3), voltage);

    // Send all the ADCs out on the CAN bus
    uint16_t adc0 = ADC_Get(ADC1_CHANNEL0);
    uint16_t adc1 = ADC_Get(ADC1_CHANNEL1);
    uint16_t adc2 = ADC_Get(ADC1_CHANNEL2);
    uint16_t adc3 = ADC_Get(ADC1_CHANNEL3);
    uint16_t adc4 = ADC_Get(ADC1_CHANNEL4);

    uint8_t canMsg1[8] = {0};
    canMsg1[0] = adc0 & 0xFF;
    canMsg1[1] = (adc0 >> 8) &0xFF;
    canMsg1[2] = adc1 & 0xFF;
    canMsg1[3] = (adc1 >> 8) &0xFF;
    canMsg1[4] = adc2 & 0xFF;
    canMsg1[5] = (adc2 >> 8) &0xFF;
    canMsg1[6] = adc3 & 0xFF;
    canMsg1[7] = (adc3 >> 8) &0xFF;

    uint8_t canMsg2[8] = {0};
    canMsg2[0] = adc4 & 0xFF;
    canMsg2[1] = (adc4 >> 8) & 0xFF;

    CAN_SendMessage(&hcan1, 0x100, canMsg1, 8);
    CAN_SendMessage(&hcan1, 0x101, canMsg2, 8);
  }
}

static void Example_canCallback(const CAN_DataFrame_T* data)
{
  printf("CAN received from %lx: ", data->msgId);
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
  printf("Example_Init begin\n");
  // Register to receive messages from CAN1
  CAN_RegisterCallback(&hcan1, 0x3A1, Example_canCallback);

  // ADC1_PUP
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // pull up
  // ADC2_PUP
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);   // pull down

  // create main task
  TaskHandle_t xHandle;
  xHandle = xTaskCreateStatic(
      Example_TaskMain,
      "ExampleTask",
      EX_STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      taskStack,
      &taskBuffer);

  printf("Example_Init complete\n");
  return EXAMPLE_STATUS_OK;
}
