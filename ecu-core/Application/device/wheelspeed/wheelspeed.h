/*
 * wheelspeed.h
 *
 *  Created on: 7 May 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_WHEELSPEED_WHEELSPEED_H_
#define DEVICE_WHEELSPEED_WHEELSPEED_H_

typedef enum
{
  WHEELSPEED_STATUS_OK     = 0x00U,
  WHEELSPEED_STATUS_ERROR  = 0x01U
} WheelSpeed_Status_T;

/**
 * @brief Initialize the process
 */
WheelSpeed_Status_T WheelSpeed_Init(void);


#endif /* DEVICE_WHEELSPEED_WHEELSPEED_H_ */
