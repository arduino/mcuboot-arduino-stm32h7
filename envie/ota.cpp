#if MCUBOOT_AS_ENVIE

#include "ota.h"
#include "rtc.h"
#include "bootutil/bootutil_log.h"

#define SD_OTA_TEST 1

void getOTAData(enum storageType* storage_type, uint32_t* data_offset, uint32_t* update_size) {
  RTCInit();
  /*
   * Magic 0x07AA is set by Arduino_Portenta_OTA
   * Magic 0xDF59 is set by the loader if RESET_REASON_PIN_RESET
   */

#if SD_OTA_TEST
  *storage_type = SD_FATFS;
  *data_offset = 2;
  *update_size = MCUBOOT_SLOT_SIZE;
  return;
#endif

  int magic = RTCGetBKPRegister(RTC_BKP_DR0);
  if (magic == 0x07AA) {
    // DR1 contains the backing storage type, DR2 the offset in case of raw device / MBR
    *storage_type = (storageType)RTCGetBKPRegister(RTC_BKP_DR1);
    *data_offset = RTCGetBKPRegister(RTC_BKP_DR2);
    *update_size = RTCGetBKPRegister(RTC_BKP_DR3);
    //BOOT_LOG_INF("Custom OTA data");
  } else {
    *storage_type = QSPI_FLASH_FATFS_MBR;
    *data_offset = 2;
    *update_size = MCUBOOT_SLOT_SIZE;
    //BOOT_LOG_INF("Default OTA data");
  }
  return;

  //BOOT_LOG_INF("Storage type %d",(int)*storage_type);
  //BOOT_LOG_INF("Offset %d",*data_offset);
  //BOOT_LOG_INF("Update size %d",*update_size);
}

#endif
