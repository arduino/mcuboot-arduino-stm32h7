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

#ifndef __BOARD_H
#define __BOARD_H

#define BOARD_APP_DEFAULT_ADD 0x08040000

#define BOOTLOADER_CONFIG_MAGIC   0xA0
#define BOOTLOADER_VERSION        6

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

#if BOARD_HAS_VIDEO
  #define BOARD_VIDEO_ENABLE                  PJ_2
  #define BOARD_VIDEO_RESET                   PJ_3
#endif

#if defined TARGET_PORTENTA_H7_M7
  #define BOARD_GREEN_LED                     PK_6
  #define BOARD_RED_LED                       PK_5
  #define BOARD_BLUE_LED                      PK_7

  #define BOARD_USB_RESET                     PJ_4
  #define BOARD_BOOT_SEL                      PI_8

  #define BOARD_I2C_SCL                       PB_6
  #define BOARD_I2C_SDA                       PB_7

  #define BOARD_USBD_VID                      0x2341
  #define BOARD_USBD_PID                      0x035B

  #define BOARD_USBD_STRING                   "Portenta H7 MCUboot"

  #define BOARD_QSPI_SO0                      PD_11
  #define BOARD_QSPI_SO1                      PD_12
  #define BOARD_QSPI_SO2                      PF_7
  #define BOARD_QSPI_SO3                      PD_13
  #define BOARD_QSPI_SCK                      PF_10
  #define BOARD_QSPI_CS                       PG_6

  #define BOARD_USB_OTG_FS_DM_DP_PIN          (GPIO_PIN_11 | GPIO_PIN_12)
  #define BOARD_USB_OTG_FS_DM_DP_MODE         (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_FS_DM_DP_PULL         (GPIO_NOPULL)
  #define BOARD_USB_OTG_FS_DM_DP_SPEED        (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_FS_DM_DP_ALTERNATE    (GPIO_AF10_OTG1_FS)
  #define BOARD_USB_OTG_FS_DM_DP_GPIO         (GPIOA)

  #define BOARD_USB_OTG_HS_CLK_PIN            (GPIO_PIN_5)
  #define BOARD_USB_OTG_HS_CLK_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_CLK_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_CLK_SPEED          (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_HS_CLK_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_CLK_GPIO           (GPIOA)

  #define BOARD_USB_OTG_HS_D0_PIN             (GPIO_PIN_3)
  #define BOARD_USB_OTG_HS_D0_MODE            (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_D0_PULL            (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_D0_SPEED           (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_HS_D0_ALTERNATE       (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_D0_GPIO            (GPIOA)

  #define BOARD_USB_OTG_HS_D1_D7_PIN          (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13)
  #define BOARD_USB_OTG_HS_D1_D7_MODE         (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_D1_D7_PULL         (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_D1_D7_ALTERNATE    (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_D1_D7_GPIO         (GPIOB)

  #define BOARD_USB_OTG_HS_STP_PIN            (GPIO_PIN_0)
  #define BOARD_USB_OTG_HS_STP_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_STP_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_STP_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_STP_GPIO           (GPIOC)

  #define BOARD_USB_OTG_HS_NXT_PIN            (GPIO_PIN_4)
  #define BOARD_USB_OTG_HS_NXT_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_NXT_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_NXT_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_NXT_GPIO           (GPIOH)

  #define BOARD_USB_OTG_HS_DIR_PIN            (GPIO_PIN_11)
  #define BOARD_USB_OTG_HS_DIR_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_DIR_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_DIR_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_DIR_GPIO           (GPIOI)

#elif defined TARGET_NICLA_VISION
  #define BOARD_GREEN_LED                     PC_13
  #define BOARD_RED_LED                       PE_3
  #define BOARD_BLUE_LED                      PF_4

  #define BOARD_USB_RESET                     PA_2

  #define BOARD_I2C_SCL                       PF_1
  #define BOARD_I2C_SDA                       PF_0

  #define BOARD_USBD_VID                      0x2341
  #define BOARD_USBD_PID                      0x035F

  #define BOARD_USBD_STRING                   "Nicla Vision MCUboot"

  #define BOARD_QSPI_SO0                      PD_11
  #define BOARD_QSPI_SO1                      PF_9
  #define BOARD_QSPI_SO2                      PE_2
  #define BOARD_QSPI_SO3                      PD_13
  #define BOARD_QSPI_SCK                      PF_10
  #define BOARD_QSPI_CS                       PG_6

  #define BOARD_USB_OTG_FS_DM_DP_PIN          (GPIO_PIN_11 | GPIO_PIN_12)
  #define BOARD_USB_OTG_FS_DM_DP_MODE         (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_FS_DM_DP_PULL         (GPIO_PULLUP)
  #define BOARD_USB_OTG_FS_DM_DP_SPEED        (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_FS_DM_DP_ALTERNATE    (GPIO_AF10_OTG1_FS)
  #define BOARD_USB_OTG_FS_DM_DP_GPIO         (GPIOA)

  #define BOARD_USB_OTG_HS_CLK_PIN            (GPIO_PIN_5)
  #define BOARD_USB_OTG_HS_CLK_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_CLK_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_CLK_SPEED          (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_HS_CLK_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_CLK_GPIO           (GPIOA)

  #define BOARD_USB_OTG_HS_D0_PIN             (GPIO_PIN_3)
  #define BOARD_USB_OTG_HS_D0_MODE            (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_D0_PULL            (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_D0_SPEED           (GPIO_SPEED_FREQ_VERY_HIGH)
  #define BOARD_USB_OTG_HS_D0_ALTERNATE       (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_D0_GPIO            (GPIOA)

  #define BOARD_USB_OTG_HS_D1_D7_PIN          (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13)
  #define BOARD_USB_OTG_HS_D1_D7_MODE         (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_D1_D7_PULL         (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_D1_D7_ALTERNATE    (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_D1_D7_GPIO         (GPIOB)

  #define BOARD_USB_OTG_HS_STP_PIN            (GPIO_PIN_0)
  #define BOARD_USB_OTG_HS_STP_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_STP_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_STP_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_STP_GPIO           (GPIOC)

  #define BOARD_USB_OTG_HS_NXT_PIN            (GPIO_PIN_3)
  #define BOARD_USB_OTG_HS_NXT_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_NXT_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_NXT_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_NXT_GPIO           (GPIOC)

  #define BOARD_USB_OTG_HS_DIR_PIN            (GPIO_PIN_2)
  #define BOARD_USB_OTG_HS_DIR_MODE           (GPIO_MODE_AF_PP)
  #define BOARD_USB_OTG_HS_DIR_PULL           (GPIO_NOPULL)
  #define BOARD_USB_OTG_HS_DIR_ALTERNATE      (GPIO_AF10_OTG2_HS)
  #define BOARD_USB_OTG_HS_DIR_GPIO           (GPIOC)

#else

#endif

#endif /* __BOARD_H */
