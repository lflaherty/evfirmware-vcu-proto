/*
 * initialize.c
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#include "initialize.h"
#include "main.h"

#include <stdio.h>
#include "vehicleProcesses/watchdogTrigger/watchdogTrigger.h"
#include "example/example.h"
#include "device/wheelspeed/wheelspeed.h"
#include "comm/can/can.h"
#include "comm/uart/uart.h"
//#include "comm/spi/spi.h" // TODO remove
#include "io/adc/adc.h"
//#include "io/ad5592r/ad5592r.h"
#include "time/tasktimer/tasktimer.h"
#include "time/externalWatchdog/externalWatchdog.h"

// externs for handles declared in main
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;

//extern SPI_HandleTypeDef hspi4;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart1;

// SPI Data for AD5592R
// TODO remove
//#define AD5592R_SPI_CS_GPIO_Port GPIOE
//#define AD5592R_SPI_CS_Pin GPIO_PIN_4


//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System(void)
{
  printf("ECU_Init_System begin\n");

  // CAN bus
  CAN_Status_T statusCan;
  statusCan = CAN_Init();
  if (CAN_STATUS_OK != statusCan) {
    printf("CAN Initialization error %u\n", statusCan);
    return ECU_INIT_ERROR;
  }

  statusCan = CAN_Config(&hcan1);
  if (CAN_STATUS_OK != statusCan) {
    printf("CAN config error %u\n", statusCan);
    return ECU_INIT_ERROR;
  }

//  // SPI bus
//  SPI_Status_T statusSpi = SPI_Init();
//  if (SPI_STATUS_OK != statusSpi) {
//    printf("SPI Initialization error %u\n", statusSpi);
//    return ECU_INIT_ERROR;
//  }

  UART_Status_T statusUart;
  statusUart = UART_Init();
  if (UART_STATUS_OK != statusUart) {
    printf("UART Initialization error %u\n", UART_STATUS_OK);
    return ECU_INIT_ERROR;
  }

  statusUart = UART_Config(&huart1);
  if (UART_STATUS_OK != statusUart) {
    printf("UART config error %u\n", statusUart);
    return ECU_INIT_ERROR;
  }

  // ADC
  ADC_Status_T statusAdc;
  statusAdc = ADC_Init();
  if (ADC_STATUS_OK != statusAdc) {
    printf("ADC initialization error %u\n", statusAdc);
    return ECU_INIT_ERROR;
  }

  statusAdc = ADC_Config(&hadc1);
  if (ADC_STATUS_OK != statusAdc) {
    printf("ADC config error %u\n", statusAdc);
    return ECU_INIT_ERROR;
  }

  // TODO remove AD5592R and SPI
//  // AD5592R
//  AD5592R_Status_T statusAD5592R;
//  statusAD5592R = AD5592R_Init(&hspi4, AD5592R_SPI_CS_GPIO_Port, AD5592R_SPI_CS_Pin);
//  if (AD5592R_STATUS_OK != statusAD5592R) {
//    printf("AD5592R initialization error %u\n", statusAD5592R);
//    return ECU_INIT_ERROR;
//  }
//
//  // TODO Setup channels
//  statusAD5592R = AD5592R_ConfigChannel(AD5592R_IO0, AD5592R_MODE_AOUT, AD5592R_PULLDOWN_ENABLED);
//  if (AD5592R_STATUS_OK != statusAD5592R) {
//    printf("AD5592R config IO0 error %u\n", statusAD5592R);
//    return ECU_INIT_ERROR;
//  }
//  statusAD5592R = AD5592R_ConfigChannel(AD5592R_IO1, AD5592R_MODE_AIN, AD5592R_PULLDOWN_ENABLED);
//  if (AD5592R_STATUS_OK != statusAD5592R) {
//    printf("AD5592R config IO1 error %u\n", statusAD5592R);
//    return ECU_INIT_ERROR;
//  }

  // Timers
  TaskTimer_Status_T statusTaskTimer = TaskTimer_Init(&htim2);
  if (TASKTIMER_STATUS_OK != statusTaskTimer) {
    printf("Task Timer initialization error %u\n", statusTaskTimer);
    return ECU_INIT_ERROR;
  }

  // External watchdog
  ExternalWatchdog_Status_T extWdgStatus = ExternalWatchdog_Init(WATCHDOG_MR_GPIO_Port, WATCHDOG_MR_Pin);
  if (EXTWATCHDOG_STATUS_OK != extWdgStatus) {
    printf("ExternalWatchdog process init error %u", extWdgStatus);
    return ECU_INIT_ERROR;
  }


  printf("ECU_Init_System complete\n\n");
  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_Application(void)
{
  printf("ECU_Init_Application begin\n");

  // Wheel speed process
  WheelSpeed_Status_T statusWheelSpeed = WheelSpeed_Init();
  if (WHEELSPEED_STATUS_OK != statusWheelSpeed) {
    printf("WheelSpeed process init error %u", statusWheelSpeed);
    return ECU_INIT_ERROR;
  }

  // Example process
  Example_Status_T statusEx;
  statusEx = Example_Init();
  if (EXAMPLE_STATUS_OK != statusEx) {
    printf("Example process init error %u", statusEx);
    return ECU_INIT_ERROR;
  }

  // Watchdog Trigger
  WatchdogTrigger_Status_T watchdogTriggerStatus = WatchdogTrigger_Init();
  if (WATCHDOGTRIGGER_STATUS_OK != watchdogTriggerStatus) {
    printf("WatchdogTrigger process init error %u", watchdogTriggerStatus);
    return ECU_INIT_ERROR;
  }

  printf("ECU_Init_Application complete\n\n");
  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
ECU_Init_Status_T ECU_Init(void)
{
  ECU_Init_Status_T ret;

  // Initialize System components
  ret = ECU_Init_System();
  if (ret != ECU_INIT_OK) {
    return ret;
  }

  // Initialize application components
  ret = ECU_Init_Application();
  if (ret != ECU_INIT_OK) {
    return ret;
  }

  return ECU_INIT_OK;
}
