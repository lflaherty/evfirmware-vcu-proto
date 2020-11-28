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

#define CAN_MAX_BUSSES 3      /* Max number of CAN busses */
#define CAN_NUM_CALLBACKS 16  /* Max number of CAN callbacks on any bus */

typedef enum
{
  CAN_STATUS_OK		               = 0x00U,
  CAN_STATUS_ERROR_TX            = 0x01U,
  CAN_STATUS_ERROR_CFG_FILTER    = 0x02U,
  CAN_STATUS_ERROR_START         = 0x03U,
  CAN_STATUS_ERROR_START_NOTIFY  = 0x04U,
  CAN_STATUS_ERROR_INVALID_BUS   = 0x05U,
  CAN_STATUS_ERROR_CALLBACK_FULL = 0x06U
} CAN_Status_T;

/**
 * @brief Data structure used to store CAN frames
 */
typedef struct {
  CAN_TypeDef* busInstance;
  uint32_t canId;
  uint8_t data[8];
  uint32_t dlc;
} CAN_DataFrame_T;

/**
 * @brief Callback method typedef
 * Params:
 *    CAN data frame
 */
typedef void (*CAN_Callback)(const CAN_DataFrame_T*);

/**
 * @brief Initialize CAN driver interface
 */
CAN_Status_T CAN_Init(void);

/**
 * @brief Configure CAN bus
 * This should be called from main. Main will retain ownership of handle ptr.
 *
 * @return Return status. CAN_STATUS_OK for success. See CAN_Status_T for more.
 */
CAN_Status_T CAN_Config(CAN_HandleTypeDef* handle);

/**
 * @brief Adds a method to the callback list. Method will be invoked when a
 * CAN frame is received.
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param callback Method to call during callback
 * @return Return status. CAN_STATUS_OK for success. See CAN_Status_T for more.
 */
CAN_Status_T CAN_RegisterCallback(const CAN_TypeDef* bus, const CAN_Callback callback);

/**
 * @brief Send a message on the CAN bus
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param msgId CAN Frame ID
 * @param data Array of data to send
 * @param n Length of data array. Max 8.
 * @return Return status. CAN_STATUS_OK for success. See CAN_Status_T for more.
 * handle->ErrorCode may provide more detailed error information.
 */
CAN_Status_T CAN_SendMessage(const CAN_TypeDef* bus, uint32_t msgId, uint8_t* data, size_t n);

#endif /* COMM_CAN_CAN_H_ */
