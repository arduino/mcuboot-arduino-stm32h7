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

#if  MCUBOOT_APPLICATION_HOOKS

#include "mbed.h"
#include "board.h"
#include "ota.h"
#include "rtc.h"
#include "power.h"
#include "bootutil/bootutil_extra.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "mbedtls/platform.h"

#if MCUBOOT_APPLICATION_DFU
#include "usbd_desc.h"
#include "usbd_dfu_flash.h"
#endif

/*
 * CLOCK_SOURCE is configured with "target.clock_source" in mbed_app.json file
 */
 #define USE_PLL_HSE_EXTC     0x8  // Use external clock (ST Link MCO)
 #define USE_PLL_HSE_XTAL     0x4  // Use external xtal (X3 on board - not provided by default)
 #define USE_PLL_HSI          0x2  // Use HSI internal clock

#if ((CLOCK_SOURCE) & USE_PLL_HSI)
extern "C" uint8_t SetSysClock_PLL_HSI(void);
#elif ((CLOCK_SOURCE) & USE_PLL_HSE_EXTC)
extern "C" uint8_t SetSysClock_PLL_HSE(uint8_t bypass, bool lowspeed);
#else

#endif

volatile const uint8_t bootloader_data[] __attribute__ ((section (".bootloader_version"), used)) = {
  BOOTLOADER_CONFIG_MAGIC,
  BOOTLOADER_VERSION,
  CLOCK_SOURCE,
  BOARD_USB_SPEED,
  BOARD_HAS_ETHERNET,
  BOARD_HAS_WIFI,
  BOARD_RAM_SIZE,
  BOARD_QSPI_SIZE,
  BOARD_HAS_VIDEO,
  BOARD_HAS_CRYPTO,
  BOARD_EXTCLOCK,
};

volatile const uint8_t bootloader_identifier[] __attribute__ ((section (".bootloader_identification"), used)) = "MCUboot Arduino";

#if MCUBOOT_APPLICATION_DFU
USBD_HandleTypeDef USBD_Device;
#endif

DigitalOut red(BOARD_RED_LED, BOARD_LED_OFF);
DigitalOut green(BOARD_GREEN_LED, BOARD_LED_OFF);
DigitalOut blue(BOARD_BLUE_LED, BOARD_LED_OFF);

#if defined (BOARD_BOOT_SEL)
DigitalIn boot_sel(BOARD_BOOT_SEL,PullDown);
#else
bool boot_sel = false;
#endif

Ticker swap_ticker;
bool debug_enabled = false;

static void led_swap_feedback_off(void) {
  swap_ticker.detach();
  red = BOARD_LED_OFF;
  green = BOARD_LED_OFF;
  blue = BOARD_LED_OFF;
}

static void led_swap_feedback() {
  blue = !blue;
  red = !red;
}

static void led_pulse(DigitalOut* led) {
  static uint8_t ledKeepValue = 0;
  static uint8_t ledTargetValue = 20;
  static int8_t ledDirection = 1;
  static int divisor = 0;
  
  if (divisor++ % 40) {
    return;
  }

  if (ledKeepValue == 0) {
    ledTargetValue += ledDirection;
    *led = !*led;
  }
  ledKeepValue ++;

  if (ledTargetValue > 250 || ledTargetValue < 10) {
    ledDirection = -ledDirection;
    ledTargetValue += ledDirection;
  }

  if (ledKeepValue == ledTargetValue) {
    *led = !*led;
  }
}

static bool valid_application() {
  /* Test if user code is programmed starting from USBD_DFU_APP_DEFAULT_ADD
   * address.
   */
  return     (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x20000000) \
          || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x24000000) \
          || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x30000000) \
          || (((*(__IO uint32_t *) 0x08040000) & 0xFF000000) == 0x38000000);

}

static int debug_init(void) {
  RTCInit();
  debug_enabled = ((RTCGetBKPRegister(RTC_BKP_DR7) & 0x00000001) || boot_sel);
  return 0;
}

static int start_dfu(void) {
  RTCSetBKPRegister(RTC_BKP_DR0, 0);

#if ((CLOCK_SOURCE) & USE_PLL_HSI)
  SetSysClock_PLL_HSI();
#elif ((CLOCK_SOURCE) & USE_PLL_HSE_EXTC)
  SetSysClock_PLL_HSE(1, false);
#else

#endif

  SystemCoreClockUpdate();

  led_swap_feedback_off();

  //turnDownEthernet();

#if MCUBOOT_APPLICATION_DFU
  init_Memories();

  /* Otherwise enters DFU mode to allow user programming his application */
  /* Init Device Library */
  USBD_Init(&USBD_Device, &DFU_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_DFU_CLASS);

  /* Add DFU Media interface */
  USBD_DFU_RegisterMedia(&USBD_Device, &USBD_DFU_Flash_fops);

  /* Start Device Process */
  USBD_Start(&USBD_Device);

  /* Set USBHS Interrupt to the lowest priority */
  // HAL_NVIC_SetPriority(OTG_HS_IRQn, 1, 0);

  /* Enable USBHS Interrupt */
  HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
  HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
#endif

  while(1) {
#if MCUBOOT_APPLICATION_DFU
#ifdef USE_USB_HS
    if (USB_OTG_HS->GINTSTS & USB_OTG_HS->GINTMSK) {
#else // USE_USB_FS
    if (USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK) {
#endif
      HAL_PCD_IRQHandler(HAL_PCD_GetHandle());
    }
#endif
    led_pulse(&green);
  }

  return 0;
}

int start_secure_application(void) {
  int rc;

  BOOT_LOG_INF("Starting MCUboot");

  // Initialize mbedtls crypto for use by MCUboot
  mbedtls_platform_context unused_ctx;
  rc = mbedtls_platform_setup(&unused_ctx);
  if(rc != 0) {
    BOOT_LOG_ERR("Failed to setup Mbed TLS, error: %d", rc);
    return -1;
  }

  struct boot_rsp rsp;
  rc = boot_go(&rsp);
  if(rc != 0) {
    BOOT_LOG_ERR("Failed to locate firmware image, error: %d\n", rc);
    return -1;
  }

  led_swap_feedback_off();

  // Run the application in the primary slot
  // Add header size offset to calculate the actual start address of application
  uint32_t address = rsp.br_image_off + rsp.br_hdr->ih_hdr_size;
  BOOT_LOG_INF("Booting firmware image at 0x%x\n", address);
  mbed_start_application(address);
}

static int start_ota(void) {
  // DR1 contains the backing storage type, DR2 the offset in case of raw device / MBR
  storageType storage_type = (storageType)RTCGetBKPRegister(RTC_BKP_DR1);
  uint32_t offset = RTCGetBKPRegister(RTC_BKP_DR2);
  uint32_t update_size = RTCGetBKPRegister(RTC_BKP_DR3);
  BOOT_LOG_INF("Start OTA 0x%X 0x%X 0x%X", storage_type, offset, update_size);
  return tryOTA(storage_type, offset, update_size);
}

int main(void) {
  debug_init();

  BOOT_LOG_INF("Starting Arduino bootloader");

  int magic = RTCGetBKPRegister(RTC_BKP_DR0);
  int reset = ResetReason::get();
  RTCSetBKPRegister(RTC_BKP_DR8, reset);

  // in case we have been reset let's wait 500 ms to see if user is trying to stay in bootloader
  if (reset == RESET_REASON_PIN_RESET) {
    // now that we've been reset let's set magic. resetting with this set will
    // flag we need to stay in bootloader.
    RTCSetBKPRegister(RTC_BKP_DR0, 0xDF59);
    HAL_Delay(500);
  }

#if defined (BOARD_USB_RESET)
  DigitalOut usb_reset(BOARD_USB_RESET, 0);
#endif

#if defined (BOARD_HAS_VIDEO) && (BOARD_HAS_VIDEO)
  DigitalOut video_enable(BOARD_VIDEO_ENABLE, 0);
  DigitalOut video_reset(BOARD_VIDEO_RESET, 0);
#endif

#if defined (BOARD_ETH_RESET)
  DigitalOut eth_reset(BOARD_ETH_RESET, 1);
#endif

  HAL_FLASH_Unlock();

  power_init();
  HAL_Delay(10);

#if defined (BOARD_USB_RESET)
  usb_reset = 1;
  HAL_Delay(10);
  usb_reset = 0;
  HAL_Delay(10);
  usb_reset = 1;
  HAL_Delay(10);
#endif

  if (magic != 0xDF59) {
    if (boot_empty_keys()) {
      BOOT_LOG_INF("Secure keys not configured");
      if ( magic == 0x07AA ) {
        /* Start OTA */
        int ota_result = start_ota();
        if (ota_result == 0) {
          // clean reboot with success flag
          BOOT_LOG_INF("Sketch updated");
          RTCSetBKPRegister(RTC_BKP_DR0, 0);
          HAL_FLASH_Lock();
          // wait for external reboot (watchdog)
          while (1) {}
        } else {
          RTCSetBKPRegister(RTC_BKP_DR0, ota_result);
        }
      }

      if (valid_application()) {
        /* Boot Sketch */
        BOOT_LOG_INF("Booting sketch at 0x%x\n", BOARD_APP_DEFAULT_ADD);
        RTCSetBKPRegister(RTC_BKP_DR0, 0);
        mbed_start_application(BOARD_APP_DEFAULT_ADD);
      } else {
        BOOT_LOG_INF("No sketch found");
      }

    } else {
      /* MCUboot secure boot */
      swap_ticker.attach(&led_swap_feedback, 250ms);
      RTCSetBKPRegister(RTC_BKP_DR0, 0);
      start_secure_application();
    }
  }
  start_dfu();

  return 0;
}

#endif
