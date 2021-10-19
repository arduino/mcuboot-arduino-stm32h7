#ifndef __TARGET_INIT_H
#define __TARGET_INIT_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rtc.h"
#include "stm32h7xx_hal_mdma.h"
#include "stm32h7xx_hal_qspi.h"
#if MCUBOOT_ENVIE_DFU
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_flash.h"
#endif

#define BOOTLOADER_CONFIG_MAGIC   0xA0
#define BOOTLOADER_VERSION        22

#define PORTENTA_USB_SPEED_HIGH   1
#define PORTENTA_USB_SPEED_FULL   2

#ifndef PORTENTA_USB_SPEED
#ifdef USE_USB_HS
#define PORTENTA_USB_SPEED    PORTENTA_USB_SPEED_HIGH
#else
#define PORTENTA_USB_SPEED    PORTENTA_USB_SPEED_FULL
#endif
#endif

#ifndef PORTENTA_HAS_ETHERNET
#define PORTENTA_HAS_ETHERNET   1
#endif

#ifndef PORTENTA_HAS_WIFI
#define PORTENTA_HAS_WIFI       1
#endif

#ifndef PORTENTA_RAM_SIZE
#define PORTENTA_RAM_SIZE       8
#endif

#ifndef PORTENTA_QSPI_SIZE
#define PORTENTA_QSPI_SIZE      16
#endif

#ifndef PORTENTA_HAS_VIDEO
#define PORTENTA_HAS_VIDEO      1
#endif

#ifndef PORTENTA_HAS_CRYPTO
#define PORTENTA_HAS_CRYPTO     1
#endif

#ifndef PORTENTA_EXTCLOCK
#define PORTENTA_EXTCLOCK       25
#endif

int target_init(void);
void envie_loop(void);

#endif /* __TARGET_INIT_H */