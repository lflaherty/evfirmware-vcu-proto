/*
 * can.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#include "can.h"

#include <string.h>

// ------------------- Private data -------------------

/**
 * @brief CAN Bus storage
 */
struct CAN_BusData {
  CAN_HandleTypeDef* handle;
  uint32_t txMailbox;
  CAN_Callback callback;
};
static struct CAN_BusData canBusData[CAN_MAX_BUSSES];

#define CAN_BUS1_INDEX 0
#define CAN_BUS2_INDEX 1
#define CAN_BUS3_INDEX 2


// ------------------- Private methods -------------------
/**
 * @brief Get a pointer to the CAN bus data given the instance
 *
 * @param bus Bus instance  (CAN1, CAN2, etc.)
 * @param retVal Pointer to return value. Sets to CAN_STATUS_OK if successful.
 * Assigns the integer to. If not CAN_STATUS_OK, CAN_BusData* is not assigned.
 * @return Pointer to CAN bus data structure.
 */
static struct CAN_BusData* CAN_GetBusData(const CAN_TypeDef* bus, CAN_Status_T* retVal)
{
  *retVal = CAN_STATUS_OK;

  struct CAN_BusData* storage;
  if (bus == CAN1) {
    storage = &canBusData[CAN_BUS1_INDEX];
  } else if (bus == CAN2) {
    storage = &canBusData[CAN_BUS2_INDEX];
  } else if (bus == CAN3) {
    storage = &canBusData[CAN_BUS3_INDEX];
  } else {
    // set retVal to be an error
    *retVal = CAN_STATUS_ERROR_INVALID_BUS;
  }

  return storage;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  /* Get RX message */
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
    return;
  }

  CAN_Status_T retVal;
  struct CAN_BusData* busData = CAN_GetBusData(hcan->Instance, &retVal);
  if (retVal != CAN_STATUS_OK) {
    // did not find a valid bus
    return;
  }

  // TODO: don't do this work in the ISR
  busData->callback(rxHeader.StdId, rxData, rxHeader.DLC);
}

// ------------------- Public methods -------------------
CAN_Status_T CAN_Init(void)
{
  // Initialize mem to 0
  memset(canBusData, 0, sizeof(canBusData));

  // create thread for processing the callbacks outside of an interrupt
  // TODO

  return CAN_STATUS_OK;
}

CAN_Status_T CAN_Config(CAN_HandleTypeDef* handle)
{
  CAN_Status_T retVal;
  struct CAN_BusData* storage = CAN_GetBusData(handle->Instance, &retVal);
  if (retVal != CAN_STATUS_OK) {
    return retVal;
  }
  storage->handle = handle;

  // Filter config
  CAN_FilterTypeDef  sFilterConfig;

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(storage->handle, &sFilterConfig) != HAL_OK) {
    return CAN_STATUS_ERROR_CFG_FILTER;
  }

  // Start bus
  if (HAL_CAN_Start(storage->handle) != HAL_OK) {
    return CAN_STATUS_ERROR_START;
  }

  //Activate CAN RX interrupt
  if (HAL_CAN_ActivateNotification(storage->handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    return CAN_STATUS_ERROR_START_NOTIFY;
  }

  return CAN_STATUS_OK;
}

CAN_Status_T CAN_SetCallback(const CAN_TypeDef* bus, const CAN_Callback method)
{
  // Find the current bus
  CAN_Status_T retVal;
  struct CAN_BusData* storage = CAN_GetBusData(bus, &retVal);
  if (retVal != CAN_STATUS_OK) {
    return retVal;
  }

  // Store callback
  storage->callback = method;

  return CAN_STATUS_OK;
}

CAN_Status_T CAN_SendMessage(const CAN_TypeDef* bus, uint32_t msgId, uint8_t* data, size_t n)
{
  // Construct header
  CAN_TxHeaderTypeDef txHeader;
  txHeader.StdId = msgId;
  txHeader.ExtId = msgId;
  txHeader.DLC = n;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.IDE = CAN_ID_STD; // Standard ID
  txHeader.TransmitGlobalTime = DISABLE;

  // Fetch CAN bus details
  CAN_Status_T retVal;
  struct CAN_BusData* busData = CAN_GetBusData(bus, &retVal);
  if (retVal != CAN_STATUS_OK) {
    return retVal;
  }

  // Send
  HAL_StatusTypeDef err = HAL_CAN_AddTxMessage(
      busData->handle, &txHeader, data, &busData->txMailbox);
  if (err != HAL_OK) {
    return CAN_STATUS_ERROR_TX;
  }

  return CAN_STATUS_OK;
}

