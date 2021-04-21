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
#include "comm/spi/spi.h"
#include "io/adc/adc.h"

// externs for handles declared in main
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;



//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System(void)
{
  printf("ECU_Init_System begin\n");

  // CAN bus
  CAN_Status_T statusCan;
  statusCan = CAN_Init();
  if (statusCan != CAN_STATUS_OK) {
    printf("CAN Initialization error %u\n", statusCan);
    return ECU_INIT_ERROR;
  }

  statusCan = CAN_Config(&hcan1);
  if (statusCan != CAN_STATUS_OK) {
    printf("CAN config error %u\n", statusCan);
    return ECU_INIT_ERROR;
  }

  // SPI bus
  SPI_Status_T statusSpi = SPI_Init();
  if (statusSpi != SPI_STATUS_OK) {
    printf("SPI Initialization error %u\n", statusSpi);
    return ECU_INIT_ERROR;
  }

  // ADC
  ADC_Status_T statusAdc;
  statusAdc = ADC_Init();
  if (statusAdc != ADC_STATUS_OK) {
    printf("ADC initialization error %u\n", statusAdc);
    return ECU_INIT_ERROR;
  }

  statusAdc = ADC_Config(&hadc1);
  if (statusAdc != ADC_STATUS_OK) {
    printf("ADC config error %u\n", statusAdc);
    return ECU_INIT_ERROR;
  }

  printf("ECU_Init_System complete\n\n");
  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_Application(void)
{
  printf("ECU_Init_Application begin\n");
  // Example process
  Example_Status_T statusEx;
  statusEx = Example_Init();
  if (statusEx != EXAMPLE_STATUS_OK) {
    printf("Example process init error %u", statusEx);
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
