/*
 * can.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#include "can.h"

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// ------------------- Private data -------------------

/* ========= CAN bus definitions ========= */

/**
 * @brief CAN Bus storage
 */
struct CAN_BusData {
  CAN_HandleTypeDef* handle;
  uint32_t txMailbox;

  uint8_t numCallbacks;  // stores how many callbacks are currently registered
  CAN_Callback callbacks[CAN_NUM_CALLBACKS];
};
static struct CAN_BusData canBusData[CAN_MAX_BUSSES];

#define CAN_BUS1_INDEX 0
#define CAN_BUS2_INDEX 1
#define CAN_BUS3_INDEX 2

/* ========= Rx Task definitions ========= */
/**
 * Stack size for CAN Rx callback thread.
 * Note the units of this: words
 * The STM32 has a 32-bit word size.
 * I.e. 200 word stack size => 200*32bit = 800 Bytes
 */
#define STACK_SIZE 200

// Task handle for Rx task
static TaskHandle_t canTaskHandle;

// Holds the TCB for the CAN Rx callback thread
static StaticTask_t xTaskBuffer;

// Callback thread will this this as it's stack
static StackType_t xTask[STACK_SIZE];

// TODO: create more threads for CAN2 and CAN3

/* ========= ISR -> Thread queue ========= */
#define CAN_QUEUE_LENGTH      50   /* 50 messages */
#define CAN_QUEUE_ITEM_SIZE   sizeof(CAN_DataFrame_T)

// Handle for queue
static QueueHandle_t canDataQueue;

// Used to hold queue's data structure
static StaticQueue_t canDataStaticQueue;

// Used as the queue's storage area.
uint8_t canDataQueueStorageArea[CAN_QUEUE_LENGTH*CAN_QUEUE_ITEM_SIZE];

// TODO: create more queues for CAN2 and CAN3


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

/**
 * Task code for CAN Rx callback thread
 */
static void CAN_RxTask(void* pvParameters)
{
  const TickType_t blockTime = 500 / portTICK_PERIOD_MS; // 500ms
  uint32_t notifiedValue;

  while (1) {
    // wait for notification from ISR
    notifiedValue = ulTaskNotifyTake(pdFALSE, blockTime);

    while (notifiedValue > 0) {
      // process callbacks

      // Receive data from the queue (and don't block)
      CAN_DataFrame_T canData;
      BaseType_t recvStatus = xQueueReceive(canDataQueue, &canData, 0);

      if (recvStatus == pdTRUE) {
        // Call the CAN callback methods
        // TODO: this is hardcoded to CAN1
        // TODO: have other methods? or pvParameters setting the CAN instance?

        uint8_t numCallbacks = canBusData[CAN_BUS1_INDEX].numCallbacks;
        for (uint8_t i = 0; i < numCallbacks; ++i) {
          canBusData[CAN_BUS1_INDEX].callbacks[i](&canData);
        }

        notifiedValue--; // one less notification to process
      } else {
        break; // exit loop processing all notifications
      }

    }
  }
}

/**
 * @brief CAN Rx interrupt
 *
 * @brief hcan CAN Bus handle provided by interrupt
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  CAN_DataFrame_T canData;

  /* Get RX message */
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, canData.data) != HAL_OK) {
    return;
  }

  // Add data to queue and notify waiting thread
  canData.busInstance = hcan->Instance;
  canData.canId = rxHeader.StdId;
  canData.dlc = rxHeader.DLC;
  // (canData.data directly assigned from HAL_CAN_GetRxMessage)

  BaseType_t status = xQueueSendToBackFromISR(canDataQueue, &canData, NULL);

  // only notify if adding to the queue worked
  if (status == pdPASS) {
    // Notify waiting thread
    vTaskNotifyGiveFromISR(canTaskHandle, NULL);
  }
}

// ------------------- Public methods -------------------
CAN_Status_T CAN_Init(void)
{
  // Initialize mem to 0
  memset(canBusData, 0, sizeof(canBusData));

  // create thread for processing the callbacks outside of an interrupt
  canTaskHandle = xTaskCreateStatic(
      CAN_RxTask,
      "CAN_RxCallback",
      STACK_SIZE,
      NULL,               // Parameter passed into the task (none in this case)
      tskIDLE_PRIORITY,  // TODO: priority?
      xTask,
      &xTaskBuffer);

  // create the ISR -> task data queue
  canDataQueue = xQueueCreateStatic(
      CAN_QUEUE_LENGTH,
      CAN_QUEUE_ITEM_SIZE,
      canDataQueueStorageArea,
      &canDataStaticQueue);

  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
CAN_Status_T CAN_RegisterCallback(const CAN_TypeDef* bus, const CAN_Callback method)
{
  // Find the current bus
  CAN_Status_T retVal;
  struct CAN_BusData* storage = CAN_GetBusData(bus, &retVal);
  if (retVal != CAN_STATUS_OK) {
    return retVal;
  }

  // Store callback
  if (storage->numCallbacks == CAN_NUM_CALLBACKS) {
    // the callback array is already full
    return CAN_STATUS_ERROR_CALLBACK_FULL;
  }
  // before incrementing, numCallbacks stores the next index we could add to
  storage->callbacks[storage->numCallbacks] = method;
  storage->numCallbacks++;

  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
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

