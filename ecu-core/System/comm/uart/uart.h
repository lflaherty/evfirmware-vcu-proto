/*
 * uart.h
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_UART_H_
#define COMM_UART_UART_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef enum
{
  UART_STATUS_OK        = 0x00U,
  UART_STATUS_ERROR_TX  = 0x01U
} UART_Status_T;

/**
 * @brief Initialize UART driver interface
 */
UART_Status_T UART_Init(void);

/**
 * @brief Configure UART bus
 * This should be called from main. Main will retain ownership of handle ptr.
 *
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_Config(UART_HandleTypeDef* handle);

/**
 * @brief Send a serial message
 *
 * @param handle UARTdevice handle
 * @param data Array of data to send
 * @param n Length of data array
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_SendMessage(UART_HandleTypeDef* handle, uint8_t* data, size_t len);


#endif /* COMM_UART_UART_H_ */
