/*
 * spi.c
 *
 *  Created on: Apr 20, 2021
 *      Author: Liam Flaherty
 */

#include "spi.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// ------------------- Private data -------------------

/* ========= SPI devices definitions ========= */
// Array of callbacks
#define SPI_NUM_BUSSES 4

// Indices for spiDevices
typedef enum
{
  SPI1_IDX       = 0,
  SPI2_IDX       = 1,
  SPI3_IDX       = 2,
  SPI4_IDX       = 3,
  SPI_INVALID_IDX = 127
} SPI_Index_T;

// SPI_Device_T but with some internal fields
typedef struct {
  SPI_Device_T* device;

  bool inUse;                   // True if bus is currently reading/writing
                                // if false, all other data in SPI_Device_T is considered invalid
  SemaphoreHandle_t mutex;      // Used to protect device while access is attempted
  StaticSemaphore_t mutexBuffer; // Buffer for static mutex
} SPI_Device_Internal_T;

// Stores the SPI device/CS pin/callback info for currently in use devices
static SPI_Device_Internal_T spiDevices[SPI_NUM_BUSSES];


/* ========= Rx Task definitions ========= */
static struct {
  // Task handle for Rx task
  TaskHandle_t spiTaskHandle;

  // Holds the TCB for the spi Rx callback thread
  StaticTask_t xTaskBuffer;

  // Callback thread will this this as it's stack
  StackType_t xTask[SPI_STACK_SIZE];
} spiBusTask;

/* ========= ISR -> Thread queue ========= */
#define SPI_QUEUE_ITEM_SIZE   sizeof(SPI_Index_T)

static struct {
  // Handle for queue
  QueueHandle_t spiDataQueue;

  // Used to hold queue's data structure
  StaticQueue_t spiDataStaticQueue;

  // Used as the queue's storage area.
  uint8_t spiDataQueueStorageArea[SPI_QUEUE_LENGTH*SPI_QUEUE_ITEM_SIZE];
} spiBusQueue;




// ------------------- Private methods -------------------
/**
 * Converts the SPIx bus instance to an index for spiDevices
 */
static SPI_Index_T getSPIBusIndex(SPI_TypeDef* spiBus) {
  // find the correct index for this bus instance
  if (spiBus == SPI1) { return SPI1_IDX; }
  else if (spiBus == SPI2) { return SPI2_IDX; }
  else if (spiBus == SPI3) { return SPI3_IDX; }
  else if (spiBus == SPI4) { return SPI4_IDX; }
  else {
    // shouldn't have reached here...
    return SPI_INVALID_IDX;
  }
}

/**
 * Task code for SPI Rx callback thread
 */
static void SPI_RxTask(void* pvParameters)
{
  const TickType_t blockTime = 500 / portTICK_PERIOD_MS; // 500ms
  uint32_t notifiedValue;

  while (1) {
    // wait for notification from ISR
    notifiedValue = ulTaskNotifyTake(pdFALSE, blockTime);

    while (notifiedValue > 0) {
      // process callbacks

      // Receive data from the queue (and don't block)

      // The callback gives us the index of the currently in use device
      SPI_Index_T spiDevIndex;
      BaseType_t recvStatus = xQueueReceive(spiBusQueue.spiDataQueue, &spiDevIndex, 0);

      if (recvStatus == pdTRUE) {
        // Call the CAN callback methods

        // acquire mutex
        // TODO timeout
        if(xSemaphoreTake(spiDevices[spiDevIndex].mutex, (TickType_t)100) == pdTRUE) {
          // check that it's still registered as in use...
          if (spiDevices[spiDevIndex].inUse) {
            // call the callback
            // no data needed as the DMA call put the
            spiDevices[spiDevIndex].device->callback();
          } else {
            // not in use, this case shouldn't happen... discard data
          }

          // mark as no longer in use
          spiDevices[spiDevIndex].inUse = false;

          // release mutex
          xSemaphoreGive(spiDevices[spiDevIndex].mutex);
        }

        notifiedValue--; // one less notification to process
      } else {
        break; // exit loop processing all notifications
      }

    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief SPI Rx interrupt
 *
 * @brief hspi SPI handle provided by interrupt
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  SPI_Index_T spiDevIndex = getSPIBusIndex(hspi->Instance);
  if (spiDevIndex == SPI_INVALID_IDX) {
    return;
  }

  // this should be true but anyway...
  if (spiDevices[spiDevIndex].inUse == true) {
    // pull up CS pin
    HAL_GPIO_WritePin(
        spiDevices[spiDevIndex].device->csPinBank,
        spiDevices[spiDevIndex].device->csPin,
        GPIO_PIN_SET);

    // push spiIndex to the complete queue & notify
    BaseType_t status = xQueueSendToBackFromISR(spiBusQueue.spiDataQueue, &spiDevIndex, NULL);

    // only notify if adding to the queue worked
    if (status == pdPASS) {
      // Notify waiting thread
      vTaskNotifyGiveFromISR(spiBusTask.spiTaskHandle, NULL);
    }
  }
}



// ------------------- Public methods -------------------
SPI_Status_T SPI_Init(void)
{
  printf("SPI_Init begin\n");

  // Initialize spiDevices
  for (uint8_t i = 0; i < SPI_NUM_BUSSES; ++i) {
    // invalidate device and create mutex
    spiDevices[i].device = NULL;
    spiDevices[i].inUse = false;
    spiDevices[i].mutex =
        xSemaphoreCreateMutexStatic(&spiDevices[i].mutexBuffer);

    if(spiDevices[i].mutex == NULL) {
      // error in creating mutex semaphore
      // created static, so not expected, but still check...
      return SPI_STATUS_ERROR_INIT_MUTEX;
    }
  }

  // create thread for processing the callbacks outside of an interrupt
  spiBusTask.spiTaskHandle = xTaskCreateStatic(
      SPI_RxTask,
      "SPI_RxCallback",
      SPI_STACK_SIZE,
      NULL,               // Parameter passed into the task (none in this case)
      tskIDLE_PRIORITY,  // TODO: priority?
      spiBusTask.xTask,
      &spiBusTask.xTaskBuffer);

  // create the ISR -> task data queue
  spiBusQueue.spiDataQueue = xQueueCreateStatic(
      SPI_QUEUE_LENGTH,
      SPI_QUEUE_ITEM_SIZE,
      spiBusQueue.spiDataQueueStorageArea,
      &spiBusQueue.spiDataStaticQueue);

  printf("SPI_Init complete\n");
  return SPI_STATUS_OK;
}

//------------------------------------------------------------------------------
SPI_Status_T SPI_TransmitReceive(
    SPI_Device_T* device,
    uint8_t* txData,
    uint8_t* rxData,
    uint16_t dataLen)
{
  // get index for spiBusTransfers
  SPI_Index_T spiDevIndex = getSPIBusIndex(device->spiHandle->Instance);
  if (spiDevIndex == SPI_INVALID_IDX) {
    return SPI_STATUS_ERROR_INVALID_BUS;
  }

  // Acquire mutex mutex
  // TODO timeout
  if (xSemaphoreTake(spiDevices[spiDevIndex].mutex, (TickType_t)10) != pdTRUE) {
    // could not obtain the semaphore and cannot access resource
    return SPI_STATUS_ERROR_BUSY;
  }

  if (spiDevices[spiDevIndex].inUse == false) {
    // take the device
    spiDevices[spiDevIndex].device = device;
    spiDevices[spiDevIndex].inUse = true; // claim usage

    // release mutex
    xSemaphoreGive(spiDevices[spiDevIndex].mutex);
  } else {
    // it's busy :(

    // release mutex before exit (avoid deadlock)
    xSemaphoreGive(spiDevices[spiDevIndex].mutex);
    return SPI_STATUS_ERROR_BUSY;
  }

  // Pull CS pin low to begin transfer
  HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_RESET);

  if (HAL_SPI_TransmitReceive_DMA(
      device->spiHandle, txData, rxData, dataLen) == HAL_OK) {
    // started transfer ok
    return SPI_STATUS_OK;
  } else {
    // failed transfer

    // Pull CS pin back up
    HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_SET);
    return SPI_STATUS_ERROR_TX;
  }

  return SPI_STATUS_OK;
}


