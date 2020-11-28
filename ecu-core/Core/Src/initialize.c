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

/*
 * @brief Macro for more easily running an init method and returning an error
 * if the returned status isn't what was desired.
 *
 * Calls a return with ECU_INIT_ERROR.
 *
 * @param methodInvocation run the init method here
 * @param resultOK the value returned from init for a passed output
 * @param errorMsg error test printed if the returned value != resultOK
 */
#define CALL_CHECK(methodInvocation, resultOK, errorMsg) {\
  if ( (methodInvocation) != (resultOK) ) {\
    printf((errorMsg)); \
    return ECU_INIT_ERROR; \
  } \
}

/**
 * @brief Macro for more easily running an init method and returning an error
 * if the returned status isn't what was desired.
 *
 * Displays status output in the printf (accessable via %u/%d).
 *
 * Calls a return with ECU_INIT_ERROR.
 *
 * @param retVar return storage for init method
 * @param methodInvocation run the init method here
 * @param resultOK the value returned from init for a passed output
 * @param errorMsg error test printed if the returned value != resultOK *
 */
#define CALL_CHECK_D(retVar, methodInvocation, resultOK, errorMsg) {\
  retVar = (methodInvocation);\
  if ( retVar != (resultOK) ) {\
    printf((errorMsg), retVar); \
    return ECU_INIT_ERROR; \
  } \
}



//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System(void)
{
  printf("\n");
  printf("Initializing system components\n");

  // CAN bus
  CAN_Status_T statusCan;
  CALL_CHECK_D(statusCan, CAN_Init(), CAN_STATUS_OK, "CAN initialization error %u");
  CALL_CHECK_D(statusCan, CAN_Config(&hcan1), CAN_STATUS_OK, "CAN config error %u");

  // ADC
  ADC_Status_T statusAdc;
  CALL_CHECK_D(statusAdc, ADC_Init(), ADC_STATUS_OK, "ADC initialization error %u");
  CALL_CHECK_D(statusAdc, ADC_Config(&hadc1), ADC_STATUS_OK, "ADC config error %u");

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_Application(void)
{
  // Example process
  Example_Status_T statusEx;
  CALL_CHECK_D(statusEx, Example_Init(), EXAMPLE_STATUS_OK, "Example process init error %u");

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
