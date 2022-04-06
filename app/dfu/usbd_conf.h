/**
  ******************************************************************************
  * @file    USB_Device/DFU_Standalone/CM7/Inc/usbd_conf.h
  * @author  MCD Application Team
  * @brief   General low level driver configuration
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF_H
#define __USBD_CONF_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Common Config */
#define USBD_MAX_NUM_INTERFACES               2
#define USBD_MAX_NUM_CONFIGURATION            1
#define USBD_MAX_STR_DESC_SIZ                 0x100
#define USBD_SUPPORT_USER_STRING_DESC         1
#define USBD_SELF_POWERED                     1
#define USBD_DEBUG_LEVEL                      0

/* DFU Class Config */
#define USBD_DFU_MAX_ITF_NUM                   4
#define USBD_DFU_XFER_SIZE                     4096   /* Max DFU Packet Size = 4096 bytes */
#define USBD_DFU_APP_DEFAULT_ADD               BOARD_APP_DEFAULT_ADD /* The first sector (32 KB) is reserved for DFU code */
#define USBD_DFU_MAX_NB_OF_SECTORS             16 /* Max number of sectors */

#define CDC_CLASS_DESC_SIZE                    (8 + 58)
#define ARDUINO_BL_CDC_INTERFACE               CDC_CLASS_DESC_SIZE

/* Exported macro ------------------------------------------------------------*/
/* Memory management macros */
#define USBD_malloc               malloc
#define USBD_free                 free
#define USBD_memset               memset
#define USBD_memcpy               memcpy
#define USBD_Delay                HAL_Delay

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0)
#define  USBD_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define  USBD_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define  USBD_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

/* Exported functions ------------------------------------------------------- */
extern PCD_HandleTypeDef * HAL_PCD_GetHandle(void);

#endif /* __USBD_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
