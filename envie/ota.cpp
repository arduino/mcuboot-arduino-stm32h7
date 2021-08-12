#include "mbed.h"
#include "ota.h"

int setOTAData(enum storageType storage_type, uint32_t data_offset, uint32_t update_size) {
	return 0;
}
int getOTAData(enum storageType* storage_type, uint32_t* data_offset, uint32_t* update_size) {
  *storage_type = SD_FATFS;	
  *data_offset = 2;
  *update_size = MCUBOOT_SLOT_SIZE;
  return 0;	
}