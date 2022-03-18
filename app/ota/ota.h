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

#ifndef __OTA_H
#define __OTA_H

#include <stdint.h>
#include "BlockDevice.h"
#include "FileSystem.h"

#define INTERNAL_FLASH_FLAG         (1 << 1)
#define QSPI_FLASH_FLAG             (1 << 2)
#define SDCARD_FLAG                 (1 << 3)
#define RAW_FLAG                    (1 << 4)
#define FATFS_FLAG                  (1 << 5)
#define LITTLEFS_FLAG               (1 << 6)
#define MBR_FLAG                    (1 << 7)

enum storageType {
    INVALID = 0,
    INTERNAL_FLASH_OFFSET = INTERNAL_FLASH_FLAG | RAW_FLAG,
    INTERNAL_FLASH_FATFS = INTERNAL_FLASH_FLAG | FATFS_FLAG,
    INTERNAL_FLASH_LITTLEFS = INTERNAL_FLASH_FLAG | LITTLEFS_FLAG,
    QSPI_FLASH_OFFSET = QSPI_FLASH_FLAG | RAW_FLAG,
    QSPI_FLASH_FATFS = QSPI_FLASH_FLAG | FATFS_FLAG,
    QSPI_FLASH_LITTLEFS = QSPI_FLASH_FLAG | LITTLEFS_FLAG,
    QSPI_FLASH_FATFS_MBR = QSPI_FLASH_FLAG | FATFS_FLAG | MBR_FLAG,
    QSPI_FLASH_LITTLEFS_MBR = QSPI_FLASH_FLAG | LITTLEFS_FLAG | MBR_FLAG,
    SD_OFFSET = SDCARD_FLAG | RAW_FLAG,
    SD_FATFS = SDCARD_FLAG | FATFS_FLAG,
    SD_LITTLEFS = SDCARD_FLAG | LITTLEFS_FLAG,
    SD_FATFS_MBR = SDCARD_FLAG | FATFS_FLAG | MBR_FLAG,
    SD_LITTLEFS_MBR = SDCARD_FLAG | LITTLEFS_FLAG | MBR_FLAG,
};

enum OTABlockDevice {
    SECONDARY_BLOCK_DEVICE = 0,
    SCRATCH_BLOCK_DEVICE = 1
};

struct BlockTableData {
    uint32_t storage_type;
    uint8_t raw_type;
    bool raw_flag;
    bool mbr_flag;
    uint8_t fs_type;
    uint32_t data_offset;
    uint32_t update_size;
    mbed::BlockDevice* raw_bd;
    mbed::BlockDevice* log_bd;
    mbed::BlockDevice* file_bd;
    mbed::FileSystem*  log_fs;
};

#define WRONG_OTA_BINARY     (-1)
#define MOUNT_FAILED         (-2)
#define NO_OTA_FILE          (-3)
#define INIT_FAILED          (-4)

#define FILEBD_READ_SIZE      0x1
#define FILEBD_WRITE_SIZE     0x1
#define FILEBD_ERASE_SIZE     0x1000

#endif //__OTA_H
