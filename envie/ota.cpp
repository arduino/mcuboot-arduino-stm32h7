#if MCUBOOT_AS_ENVIE

#include "ota.h"

//static storageType _storage_type = INVALID;
//static uint32_t _data_offset = 0;
//static uint32_t _update_size = 0;

static storageType _storage_type = SD_FATFS;
static uint32_t _data_offset = 2;
static uint32_t _update_size = MCUBOOT_SLOT_SIZE;


void setOTAData(enum storageType storage_type, uint32_t data_offset, uint32_t update_size) {
	_storage_type = storage_type;
  _data_offset = data_offset;
  _update_size = update_size;

}
void getOTAData(enum storageType* storage_type, uint32_t* data_offset, uint32_t* update_size) {
  if(_storage_type == INVALID) {
    *storage_type = QSPI_FLASH_FATFS_MBR;
    *data_offset = 2;
    *update_size = MCUBOOT_SLOT_SIZE;
  } else {
    *storage_type = _storage_type;
    *data_offset = _data_offset;
    *update_size = _update_size;
  }
}

#endif
