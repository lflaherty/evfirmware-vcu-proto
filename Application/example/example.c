/*
 * example.c
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#include "example.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "comm/can/can.h"
#include "comm/uart/uart.h"
#include "io/adc/adc.h"
#include "time/tasktimer/tasktimer.h"
#include "time/rtc/rtc.h"

// ------------------- Private data -------------------
static Logging_T* log;

static unsigned int count = 0;

#define EX_STACK_SIZE 2000
static StaticTask_t taskBuffer;
static StackType_t taskStack[EX_STACK_SIZE];

// GPIO pins
#define LED_STATUS_Pin GPIO_PIN_12
#define LED_STATUS_GPIO_Port GPIOB

// Task data
static TaskHandle_t exampleTaskHandle;

// TODO pass this as a parameter
extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef hrtc;

// RTC data
static RTC_DateTime_T rtcDateTime;

// ------------------- Private methods -------------------
static void Example_TaskMain(void* pvParameters)
{
  logPrintS(log, "Example_TaskMain complete\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  const TickType_t blockTime = 10 / portTICK_PERIOD_MS; // 10ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process

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

      /* Send something on UART */
      char hellomsg[] = "Hey..;)\n";
      UART_SendMessage(&huart1, (uint8_t*)hellomsg, sizeof(hellomsg)/sizeof(char));

      // Update RTC
      RTC_GetDateTime(&hrtc, &rtcDateTime);
      snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN,
          "%d/%d/%d %d:%d:%d\t",
          rtcDateTime.date.Year,
          rtcDateTime.date.Month,
          rtcDateTime.date.Date,
          rtcDateTime.time.Hours,
          rtcDateTime.time.Minutes,
          rtcDateTime.time.Seconds);
      logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);


      snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "\n");
      logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
      count++;
    }

  }
}

static void Example_canCallback(const CAN_DataFrame_T* data)
{
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];
  snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "CAN received from %lx: ", data->msgId);
  logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);

  size_t i;
  for (i = 0; i < data->dlc; ++i)
  {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, " %x", data->data[i]);
    logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
  }

  snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "\n");
  logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
}

static void Example_uartCallback(const USART_Data_T* data)
{
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];
  snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "UART received byte: %x\n", data->data);
  logPrintS(log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
}

// ------------------- Public methods -------------------
Example_Status_T Example_Init(Logging_T* logger)
{
  log = logger;
  logPrintS(log, "Example_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Register to receive messages from CAN1
  CAN_RegisterCallback(&hcan1, 0x3A1, Example_canCallback);
  UART_RegisterCallback(&huart1, Example_uartCallback);

  // ADC1_PUP
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 0 (RESET) => pull up
  // ADC2_PUP
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);   // 1 (SET) => pull down

  // set RTC
  memset(&rtcDateTime, 0, sizeof(RTC_DateTime_T));
  rtcDateTime.time.Hours = 0x0;
  rtcDateTime.time.Minutes = 0x0;
  rtcDateTime.time.Seconds = 0x0;
  rtcDateTime.time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  rtcDateTime.time.StoreOperation = RTC_STOREOPERATION_RESET;
  rtcDateTime.date.WeekDay = RTC_WEEKDAY_MONDAY;
  rtcDateTime.date.Month = RTC_MONTH_JANUARY;
  rtcDateTime.date.Date = 0x1;
  rtcDateTime.date.Year = 0x0;
  RTC_SetDateTime(&hrtc, &rtcDateTime);

  // create main task
  exampleTaskHandle = xTaskCreateStatic(
      Example_TaskMain,
      "ExampleTask",
      EX_STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      taskStack,
      &taskBuffer);

  // Register the task for timer notifications every 1s
  uint16_t timerDivider = 1000 * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&exampleTaskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return EXAMPLE_STATUS_ERROR;
  }

  logPrintS(log, "Example_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return EXAMPLE_STATUS_OK;
}
