#ifndef __TARGET_INIT_H
#define __TARGET_INIT_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_mdma.h"
#include "stm32h7xx_hal_qspi.h"
#if MCUBOOT_ENVIE_DFU
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_flash.h"
#endif

#define BOOTLOADER_CONFIG_MAGIC   0xA0
#define BOOTLOADER_VERSION        2

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

int target_init(void);
void envie_loop(void);

#endif /* __TARGET_INIT_H */
