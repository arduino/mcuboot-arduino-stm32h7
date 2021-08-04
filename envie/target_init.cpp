#include "mbed.h"
#include "target_init.h"

// clock source is selected with CLOCK_SOURCE in json config
#define USE_PLL_HSE_EXTC     0x8  // Use external clock (ST Link MCO)
#define USE_PLL_HSE_XTAL     0x4  // Use external xtal (X3 on board - not provided by default)
#define USE_PLL_HSI          0x2  // Use HSI internal clock

volatile const uint8_t bootloader_data[] __attribute__ ((section (".bootloader_version"), used)) = {
  BOOTLOADER_CONFIG_MAGIC,
  BOOTLOADER_VERSION,
  CLOCK_SOURCE,
  PORTENTA_USB_SPEED,
  PORTENTA_HAS_ETHERNET,
  PORTENTA_HAS_WIFI,
  PORTENTA_RAM_SIZE,
  PORTENTA_QSPI_SIZE,
  PORTENTA_HAS_VIDEO,
  PORTENTA_HAS_CRYPTO,
  PORTENTA_EXTCLOCK,
};

