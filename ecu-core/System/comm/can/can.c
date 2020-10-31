/*
 * can.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#include "can.h"

#include <string.h>
#include <stdio.h>

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
static struct CAN_BusData* CAN_GetBusData(const CAN_TypeDef* bus)
{
  struct CAN_BusData* storage;
  if (bus == CAN1) {
    storage = &canBusData[CAN_BUS1_INDEX];
  } else if (bus == CAN2) {
    storage = &canBusData[CAN_BUS2_INDEX];
  } else if (bus == CAN3) {
    storage = &canBusData[CAN_BUS3_INDEX];
  } else {
    // TODO: throw error: invalid CAN bus
  }
  // TODO: probably return a status, pass storage out through a param

  return storage;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];

  /* Get RX message */
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
    // TODO handle error
    printf("error in recv\n");
    return;
  }

  struct CAN_BusData* busData = CAN_GetBusData(hcan->Instance);
  busData->callback(rxHeader.StdId, rxData, rxHeader.DLC);
}

// ------------------- Public methods -------------------
void CAN_Init(void)
{
  // Initialize mem to 0
  memset(canBusData, 0, sizeof(canBusData));

  // create thread for processing the callbacks outside of an interrupt
  // TODO
}

void CAN_Config(CAN_HandleTypeDef* handle)
{
  struct CAN_BusData* storage = CAN_GetBusData(handle->Instance);
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
    // TODO error handling
    printf("error: HAL_CAN_ConfigFilter failed\n");
    return;
  }

  // Start bus
  if (HAL_CAN_Start(storage->handle) != HAL_OK) {
    // TODO error handling
    printf("error: HAL_CAN_Start failed\n");
    return;
  }

  //Activate CAN RX interrupt
  if (HAL_CAN_ActivateNotification(storage->handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    // TODO error handling
    printf("error: HAL_CAN_ActivateNotification failed\n");
    return;
  }

  printf("CAN bus config complete\n");
}

void CAN_SetCallback(const CAN_TypeDef* bus, const CAN_Callback method)
{
  // Find the current bus
  struct CAN_BusData* storage = CAN_GetBusData(bus);

  // Store callback
  storage->callback = method;
}

void CAN_SendMessage(const CAN_TypeDef* bus, uint32_t msgId, uint8_t* data, size_t n)
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
  struct CAN_BusData* busData = CAN_GetBusData(bus);

  // Send
  HAL_StatusTypeDef err = HAL_CAN_AddTxMessage(
      busData->handle, &txHeader, data, &busData->txMailbox);
  if (err != HAL_OK) {
    // TODO: error
    printf("error: failed to send CAN message %d\n", err);
    return;
  }
}

// TODO: return types
