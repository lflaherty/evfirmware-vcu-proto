/*
 * uart.c
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#include "uart.h"

#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// ------------------- Private methods -------------------
/**
 * @brief UART DMA Rx interrupt
 * Second byte
 *
 * @brief huart UART handle provided by interrupt
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{

}

/**
 * @brief UART DMA Rx interrupt
 * First byte
 *
 * @brief huart UART handle provided by interrupt
 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart)
{

}


// ------------------- Public methods -------------------
UART_Status_T UART_Init(void)
{
  printf("CAN_Init begin\n");

  // TODO setup memory and create task

  printf("UART_Init complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_Config(UART_HandleTypeDef* handle)
{
  printf("UART_Config begin\n");

  printf("UART_Config complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_SendMessage(UART_HandleTypeDef* handle, uint8_t* data, size_t len)
{
  HAL_StatusTypeDef ret = HAL_UART_Transmit_DMA(handle, data, len);
  if (HAL_OK != ret) {
    return UART_STATUS_ERROR_TX;
  }

  return UART_STATUS_OK;
}
