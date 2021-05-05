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
#include "io/ad5592r/ad5592r.h"

// externs for handles declared in main
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;

extern SPI_HandleTypeDef hspi4;

// SPI Data for AD5592R
#define AD5592R_SPI_CS_GPIO_Port GPIOE
#define AD5592R_SPI_CS_Pin GPIO_PIN_4


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

  // AD5592R
  AD5592R_Status_T statusAD5592R;
  statusAD5592R = AD5592R_Init(&hspi4, AD5592R_SPI_CS_GPIO_Port, AD5592R_SPI_CS_Pin);
  if (AD5592R_STATUS_OK != statusAD5592R) {
    printf("AD5592R initialization error %u\n", statusAD5592R);
    return ECU_INIT_ERROR;
  }

  // TODO Setup channels
  statusAD5592R = AD5592R_ConfigChannel(AD5592R_IO0, AD5592R_MODE_AOUT, AD5592R_PULLDOWN_ENABLED);
  if (AD5592R_STATUS_OK != statusAD5592R) {
    printf("AD5592R config IO0 error %u\n", statusAD5592R);
    return ECU_INIT_ERROR;
  }
  statusAD5592R = AD5592R_ConfigChannel(AD5592R_IO1, AD5592R_MODE_AIN, AD5592R_PULLDOWN_ENABLED);
  if (AD5592R_STATUS_OK != statusAD5592R) {
    printf("AD5592R config IO1 error %u\n", statusAD5592R);
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
