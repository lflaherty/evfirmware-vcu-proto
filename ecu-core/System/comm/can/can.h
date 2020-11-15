/*
 * can.h
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#ifndef COMM_CAN_CAN_H_
#define COMM_CAN_CAN_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define CAN_MAX_BUSSES 3    /* Max number of CAN busses */

typedef enum
{
  CAN_STATUS_OK		              = 0x00U,
  CAN_STATUS_ERROR_TX           = 0x01U,
  CAN_STATUS_ERROR_CFG_FILTER   = 0x02U,
  CAN_STATUS_ERROR_START        = 0x03U,
  CAN_STATUS_ERROR_START_NOTIFY = 0x04U,
  CAN_STATUS_ERROR_INVALID_BUS  = 0x05U,
} CAN_Status_T;

/**
 * @brief Callback method typedef
 * Params:
 *    Msg ID
 *    Array (of up to 8) of data points
 *    Length of data
 */
typedef void (*CAN_Callback)(uint32_t, uint8_t*, size_t);

/**
 * @brief Initialize CAN driver interface
 */
CAN_Status_T CAN_Init(void);

/**
 * @brief Configure CAN bus
 * This should be called from main. Main will retain ownership of handle ptr.
 *
 * @return Return status. 0 for success. See CAN_Status_T for more.
 */
CAN_Status_T CAN_Config(CAN_HandleTypeDef* handle);

/**
 * @brief Adds a method to the callback list. Method will be invoked when a
 * CAN frame is received.
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param callback Method to call during callback
 */
CAN_Status_T CAN_SetCallback(const CAN_TypeDef* bus, const CAN_Callback callback);

/**
 * @brief Send a message on the CAN bus
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param msgId CAN Frame ID
 * @param data Array of data to send
 * @param n Length of data array. Max 8.
 * @return Return status. 0 for success. See CAN_Status_T for more.
 */
CAN_Status_T CAN_SendMessage(const CAN_TypeDef* bus, uint32_t msgId, uint8_t* data, size_t n);

#endif /* COMM_CAN_CAN_H_ */
