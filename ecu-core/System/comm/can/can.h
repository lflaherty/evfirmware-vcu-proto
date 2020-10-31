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
void CAN_Init(void);

/**
 * @brief Configure CAN bus
 * This should be called from main. Main will retain ownership of handle ptr.
 */
void CAN_Config(CAN_HandleTypeDef* handle);

/**
 * @brief Adds a method to the callback list. Method will be invoked when a
 * CAN frame is received.
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param callback Method to call during callback
 */
void CAN_SetCallback(const CAN_TypeDef* bus, const CAN_Callback callback);

/**
 * @brief Send a message on the CAN bus
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param msgId CAN Frame ID
 * @param data Array of data to send
 * @param n Length of data array. Max 8.
 */
void CAN_SendMessage(const CAN_TypeDef* bus, uint32_t msgId, uint8_t* data, size_t n);

#endif /* COMM_CAN_CAN_H_ */
