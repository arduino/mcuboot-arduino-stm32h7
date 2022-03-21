/*
  Copyright (c) 2022 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __TARGET_INIT_H
#define __TARGET_INIT_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_mdma.h"
#include "stm32h7xx_hal_qspi.h"
#if MCUBOOT_APPLICATION_DFU
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_flash.h"
#endif

#if MCUBOOT_APPLICATION_DFU
#define APP_DEFAULT_ADD USBD_DFU_APP_DEFAULT_ADD
#else
#define APP_DEFAULT_ADD 0x08040000
#endif

#define BOOTLOADER_CONFIG_MAGIC   0xA0
#define BOOTLOADER_VERSION        3

#define USB_SPEED_HIGH   1
#define USB_SPEED_FULL   2

#ifndef BOARD_USB_SPEED
#ifdef USE_USB_HS
#define BOARD_USB_SPEED    USB_SPEED_HIGH
#else
#define BOARD_USB_SPEED    USB_SPEED_FULL
#endif
#endif

#ifndef BOARD_HAS_ETHERNET
#define BOARD_HAS_ETHERNET   1
#endif

#ifndef BOARD_HAS_WIFI
#define BOARD_HAS_WIFI       1
#endif

#ifndef BOARD_RAM_SIZE
#define BOARD_RAM_SIZE       8
#endif

#ifndef BOARD_QSPI_SIZE
#define BOARD_QSPI_SIZE      16
#endif

#ifndef BOARD_HAS_VIDEO
#define BOARD_HAS_VIDEO      1
#endif

#ifndef BOARD_HAS_CRYPTO
#define BOARD_HAS_CRYPTO     1
#endif

#ifndef BOARD_EXTCLOCK
#define BOARD_EXTCLOCK       25
#endif

int target_debug_init(void);
int target_loop(void);
int target_debug(void);
int target_led_off(void);

#endif /* __TARGET_INIT_H */
