/*
 * initialize.c
 *
 *  Created on: 28 Nov 2020
 *      Author: Liam Flaherty
 */

#include "initialize.h"

#include <stdio.h>
#include "example/example.h"
#include "comm/can/can.h"
#include "io/adc/adc.h"

// externs for handles declared in main
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;


//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System(void)
{
  printf("\n");
  printf("Initializing system components\n");

  // CAN bus
  CAN_Status_T statusCan;
  statusCan = CAN_Init();
  if (statusCan != CAN_STATUS_OK) {
    printf("CAN initialization error %u", statusCan);
    return ECU_INIT_ERROR;
  }

  statusCan = CAN_Config(&hcan1);
  if (statusCan != CAN_STATUS_OK) {
    printf("CAN config error %u", statusCan);
    return ECU_INIT_ERROR;
  }

  // ADC
  ADC_Status_T statusAdc;
  statusAdc = ADC_Init();
  if (statusAdc != ADC_STATUS_OK) {
    printf("ADC initialization error %u", statusAdc);
    return ECU_INIT_ERROR;
  }

  statusAdc = ADC_Config(&hadc1);
  if (statusAdc != ADC_STATUS_OK) {
    printf("ADC config error %u", statusAdc);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_Application(void)
{
  // Example process
  Example_Status_T status;
  status = Example_Init();
  if (status != EXAMPLE_STATUS_OK) {
    printf("Example process init error %u", status);
    return ECU_INIT_ERROR;
  }

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
