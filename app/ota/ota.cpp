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

#include "mbed.h"
#include "QSPIFBlockDevice.h"
#include "FlashIAPBlockDevice.h"
#include "MBRBlockDevice.h"
#include "SDMMCBlockDevice.h"

#include "FATFileSystem.h"
#include "LittleFileSystem.h"

#include "ota.h"
#include "bootutil/bootutil_log.h"

BlockDevice* bd = NULL;
mbed::FileSystem* fs = NULL;

extern FlashIAP flash;

const uint32_t M7_FLASH_BASE = 0x8040000;
const uint32_t M4_FLASH_BASE = 0x8100000;

uint32_t getOTABinaryBase(uint32_t firstWord) {

  BOOT_LOG_DBG("First OTA binary word: %lx", firstWord);
  if ((firstWord & 0xFF000000) == 0x20000000
     || (firstWord & 0xFF000000) == 0x24000000
     || (firstWord & 0xFF000000) == 0x30000000
     || (firstWord & 0xFF000000) == 0x38000000) {
    BOOT_LOG_DBG("Flashing on M7");
    return M7_FLASH_BASE;
  }
  if ((firstWord & 0xFF000000) == 0x10000000) {
    BOOT_LOG_DBG("Flashing on M4");
    return M4_FLASH_BASE;
  }
  return 0xFFFFFFFF;
}

static Timeout restart_timer;

size_t getFilesize(const char* filename) {
  struct stat st;
  if(stat(filename, &st) != 0) {
    return 0;
  }
  return st.st_size;
}

int tryOTA(enum storageType storage_type, uint32_t data_offset, uint32_t update_size) {
  int err;
  flash.init();
  BOOT_LOG_DBG("Configuration: ");
  if (storage_type & INTERNAL_FLASH_FLAG) {
    BOOT_LOG_DBG("  INTERNAL_FLASH ");
    if (storage_type & (FATFS_FLAG | LITTLEFS_FLAG)) {
      // have a filesystem, use offset as partition start
      bd = new FlashIAPBlockDevice(0x8000000 + data_offset, 2 * 1024 * 1024 - data_offset);
    } else {
      // raw device, no offset
      bd = new FlashIAPBlockDevice(0x8000000, 2 * 1024 * 1024);
    }
  }
  if (storage_type & QSPI_FLASH_FLAG) {
    BOOT_LOG_DBG("  QSPI_FLASH ");
    extern QSPIFBlockDevice qspi_flash;
    bd = &qspi_flash;
  }
  if (storage_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
    BOOT_LOG_DBG("  SD_FLASH ");
    bd = new SDMMCBlockDevice();
#else
    BOOT_LOG_ERR("  SD NOT SUPPORTED");
#endif
  }
  if (storage_type & MBR_FLAG) {
    BOOT_LOG_DBG("  MBR ");
    BlockDevice* physical_block_device = bd;
    bd = new mbed::MBRBlockDevice(physical_block_device, data_offset);
  }
  if (storage_type & LITTLEFS_FLAG) {
    BOOT_LOG_DBG("  LITTLEFS ");
    fs = new LittleFileSystem("fs");
  }
  if (storage_type & FATFS_FLAG) {
    BOOT_LOG_DBG("  FATFS ");
    fs = new FATFileSystem("fs");
  }
  if (fs != NULL) {
    err = fs->mount(bd);
    if (err) {
      BOOT_LOG_DBG("Mount failed");
      return MOUNT_FAILED;
    }
    FILE* update = fopen("/fs/UPDATE.BIN", "rb");
    if (update == NULL) {
      BOOT_LOG_DBG("No OTA file");
      return NO_OTA_FILE;
    }
    uint32_t temp[4];
    fread((uint8_t*)temp, 1, 4, update);
    fseek(update, 0, SEEK_SET);
    uint32_t base = getOTABinaryBase(temp[0]);
    if (base == 0xFFFFFFFF) {
      BOOT_LOG_DBG("Couldn't decide if M7 or M4");
      return WRONG_OTA_BINARY;
    }
    // Ignore update_size and use file size instead
    update_size = getFilesize("/fs/UPDATE.BIN");
    uint32_t sector_size = flash.get_sector_size(base);
    if (sector_size > 4096 * 4) {
        sector_size = 4096 * 4;
    }
    uint8_t* buf = (uint8_t*)malloc(sector_size);
    BOOT_LOG_DBG("Sector size: %d", sector_size);
    for (uint32_t i = 0; i < update_size; i+= sector_size) {
      fread(buf, 1, sector_size, update);
      // erase?
      if (((uint32_t)base + i) % flash.get_sector_size(base) == 0) {
        BOOT_LOG_DBG("Erasing: %x %x", (uint32_t)base + i, flash.get_sector_size(base));
        flash.erase((uint32_t)base + i, flash.get_sector_size(base));
        wait_us(10000);
      }
      flash.program(buf, (uint32_t)base + i, sector_size);
      wait_us(1000);
    }
  } else if (bd != NULL) {
    // read first chuck of the file to understand if we need to flash the M4 or the M7
    err = bd->init();
    if (err != 0) {
      BOOT_LOG_DBG("Init failed");
      return INIT_FAILED;
    }
    int sz = bd->get_program_size();
    if (sz < 16) {
      sz = 16;
    }
    uint32_t temp[sz];
    bd->read(temp, (uint32_t)data_offset, sz);
    uint32_t base = getOTABinaryBase(temp[0]);
    if (base == 0xFFFFFFFF) {
      BOOT_LOG_DBG("Couldn't decide if M7 or M4");
      return WRONG_OTA_BINARY;
    }
    uint32_t sector_size = flash.get_sector_size(base);
    if (sector_size > 4096 * 4) {
      sector_size = 4096 * 4;
    }
    uint8_t* buf = (uint8_t*)malloc(sector_size);
    for (uint32_t i = 0; i < update_size; i+= sector_size) {
      bd->read(buf, (uint32_t)data_offset + i, sector_size);
      // erase?
      if (((uint32_t)base + i) % flash.get_sector_size(base) == 0) {
        flash.erase((uint32_t)base + i, flash.get_sector_size(base));
        wait_us(10000);
      }
      flash.program(buf, (uint32_t)base + i, sector_size);
      wait_us(1000);
    }
  }
  flash.deinit();
  restart_timer.attach(NVIC_SystemReset, 0.2f);
  return 0;
}
