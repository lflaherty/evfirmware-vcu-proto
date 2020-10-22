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

#define CAN_MAX_CALLBACKS 5   /* Max number of callbacks per bus */
#define CAN_MAX_BUSSES 3    /* Max number of CAN busses */

/**
 * @brief Callback method typedef
 */
typedef void (*CAN_Callback)(void);

/**
 * @brief Callback definition
 */
typedef struct {
	CAN_Callback call;
	uint32_t mask;
} CAN_Callback_T;

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
 * CAN frame is received that matches the mask.
 *
 * The callback will be called if ID & !mask == 0
 * (i.e. the mask bits must be 1 on the ID bits for the ID to match)
 *
 * Eg:
 * ID:     0x38  = 0011 1000
 * Mask:   0x3F  = 0011 1111
 * MSG & !Mask   = 0000 0000 => Callback invoked
 *
 * ID:     0x38  = 0011 1000
 * Mask:   0x8F  = 1000 1111
 * MSG & !filter = 0011 0000 => Callback not invoked
 *
 * @param bus CAN bus to use. I.e. CAN1/2/3
 * @param callback Method to call during callback
 * @param mask Mask to filter CAN frame ID
 */
void CAN_RegisterCallback(const CAN_TypeDef* bus, const CAN_Callback callback, uint32_t mask);

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
