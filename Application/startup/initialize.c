/*
 * initialize.c
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#include "initialize.h"

#include <stdio.h>
#include <stdbool.h>

#include "lib/logging/logging.h"
#include "comm/can/can.h"
#include "comm/uart/uart.h"
#include "io/adc/adc.h"
#include "time/tasktimer/tasktimer.h"
#include "time/externalWatchdog/externalWatchdog.h"
#include "time/rtc/rtc.h"

#include "device/wheelspeed/wheelspeed.h"

#include "vehicleInterface/deviceMapping/deviceMapping.h"
#include "vehicleProcesses/example/example.h"
#include "vehicleProcesses/watchdogTrigger/watchdogTrigger.h"

// ------------------- Private data -------------------
static Logging_T log;

// ------------------- Private prototypes -------------------
static ECU_Init_Status_T ECU_Init_System1(void);  // Init basics for logging
static ECU_Init_Status_T ECU_Init_System2(void);  // Init remaining internal devices
static ECU_Init_Status_T ECU_Init_System3(void);  // Init external devices
static ECU_Init_Status_T ECU_Init_App1(void);     // Init application devices
static ECU_Init_Status_T ECU_Init_App2(void);     // Init application vehicle interface
static ECU_Init_Status_T ECU_Init_App3(void);     // Init application processes

//------------------------------------------------------------------------------
ECU_Init_Status_T ECU_Init(void)
{
  ECU_Init_Status_T ret = ECU_INIT_OK;

  // Initialize components
  ret |= ECU_Init_System1();
  ret |= ECU_Init_System2();
  ret |= ECU_Init_System3();
  ret |= ECU_Init_App1();
  ret |= ECU_Init_App2();
  ret |= ECU_Init_App3();

  if (ret != ECU_INIT_OK) {
    logPrintS(&log, "Failed to initialize\n", LOGGING_DEFAULT_BUFF_LEN);
    return ret;
  }

  logPrintS(&log, "ECU_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System1(void)
{
  // Set up logging
  Logging_Status_T statusLog;
  statusLog = Log_Init(&mLog);
  if (LOGGING_STATUS_OK != statusLog) {
    return ECU_INIT_ERROR;
  }

  statusLog = Log_EnableSWO(&mLog);
  if (LOGGING_STATUS_OK != statusLog) {
    return ECU_INIT_ERROR;
  }

  logPrintS(&log, "###### ECU_Init_System1 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  // UART
  UART_Status_T statusUart;
  statusUart = UART_Init(&log);
  if (UART_STATUS_OK != statusUart) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "UART Initialization error %u\n", UART_STATUS_OK);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  statusUart = UART_Config(Mapping_GetUART1());
  if (UART_STATUS_OK != statusUart) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "UART config error %u\n", statusUart);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // enable serial logging
  log.enableLogToSerial = true;
  log.handleSerial = Mapping_GetUART1();

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System2(void)
{
  logPrintS(&log, "###### ECU_Init_System2 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  // CAN bus
  CAN_Status_T statusCan;
  statusCan = CAN_Init(&log);
  if (CAN_STATUS_OK != statusCan) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "CAN Initialization error %u\n", statusCan);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  statusCan = CAN_Config(Mapping_GetCAN1());
  if (CAN_STATUS_OK != statusCan) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "CAN config error %u\n", statusCan);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // ADC
  ADC_Status_T statusAdc;
  statusAdc = ADC_Init(&log, MAPPING_ADC_NUM_CHANNELS, 16);  // TODO don't use magic number
  if (ADC_STATUS_OK != statusAdc) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "ADC initialization error %u\n", statusAdc);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  statusAdc = ADC_Config(Mapping_GetADC());
  if (ADC_STATUS_OK != statusAdc) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "ADC config error %u\n", statusAdc);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // Timers
  TaskTimer_Status_T statusTaskTimer = TaskTimer_Init(&log, Mapping_GetTaskTimer());
  if (TASKTIMER_STATUS_OK != statusTaskTimer) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "Task Timer initialization error %u\n", statusTaskTimer);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // RTC
  RTC_Status_T rtcStatus = RTC_Init(&log);
  if (RTC_STATUS_OK != rtcStatus) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "RTC initialization error %u", rtcStatus);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System3(void)
{
  logPrintS(&log, "###### ECU_Init_System3 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  // External watchdog
  ExternalWatchdog_Status_T extWdgStatus = ExternalWatchdog_Init(&log, WATCHDOG_MR_GPIO_Port, WATCHDOG_MR_Pin);
  if (EXTWATCHDOG_STATUS_OK != extWdgStatus) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "ExternalWatchdog initialization error %u", extWdgStatus);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  logPrintS(&log, "###### ECU_Init_App1 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  // Wheel speed process
  WheelSpeed_Status_T statusWheelSpeed = WheelSpeed_Init(&log);
  if (WHEELSPEED_STATUS_OK != statusWheelSpeed) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "WheelSpeed process init error %u", statusWheelSpeed);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  logPrintS(&log, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App3(void)
{
  logPrintS(&log, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  // Example process
  Example_Status_T statusEx;
  statusEx = Example_Init(
      &log,
      Mapping_GetCAN1(),
      Mapping_GetUART1(),
      Mapping_GetRTC());
  if (EXAMPLE_STATUS_OK != statusEx) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "Example process init error %u", statusEx);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // Watchdog Trigger
  WatchdogTrigger_Status_T watchdogTriggerStatus = WatchdogTrigger_Init(&log);
  if (WATCHDOGTRIGGER_STATUS_OK != watchdogTriggerStatus) {
    snprintf(logBuffer, LOGGING_DEFAULT_BUFF_LEN, "WatchdogTrigger process init error %u", watchdogTriggerStatus);
    logPrintS(&log, logBuffer, LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

