#if  MCUBOOT_APPLICATION_HOOKS

#include "ota.h"
#include "rtc.h"
#include "bootutil/bootutil_log.h"


#include "SlicingBlockDevice.h"
#include "FlashIAPBlockDevice.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FileBlockDevice.h"
#include "FATFileSystem.h"

#if MCUBOOT_APPLICATION_SDCARD
#include "SDMMCBlockDevice.h"
#endif

#if MCUBOOT_APPLICATION_LITTLEFS
#include "LittleFileSystem.h"
#endif

static bool BlockTableLoaded = false;
static BlockTableData block_info[2];

static void loadOTAData(void) {
    RTCInit();
    /*
     * Magic 0x07AA is set by Arduino_Portenta_OTA
     * Magic 0xDF59 is set by the loader if RESET_REASON_PIN_RESET
     */

    int magic = RTCGetBKPRegister(RTC_BKP_DR0);
    if (magic == 0x07AA) {
        // DR1 contains the backing storage type
        // DR2 the offset in case of raw device / MBR
        // DR3 the update size
        block_info[SECONDARY_BLOCK_DEVICE].storage_type = (storageType)RTCGetBKPRegister(RTC_BKP_DR1);
        block_info[SECONDARY_BLOCK_DEVICE].raw_type = RTCGetBKPRegister(RTC_BKP_DR1) & 0x00000007;
        block_info[SECONDARY_BLOCK_DEVICE].raw_flag = RTCGetBKPRegister(RTC_BKP_DR1) & RAW_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].mbr_flag = RTCGetBKPRegister(RTC_BKP_DR1) & MBR_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].data_offset = RTCGetBKPRegister(RTC_BKP_DR2);
        block_info[SECONDARY_BLOCK_DEVICE].update_size = RTCGetBKPRegister(RTC_BKP_DR3);

        // DR4 contains the backing storage type
        // DR5 the offset in case of raw device / MBR
        // DR6 the scratch size
        block_info[SCRATCH_BLOCK_DEVICE].storage_type = (storageType)RTCGetBKPRegister(RTC_BKP_DR4);
        block_info[SCRATCH_BLOCK_DEVICE].raw_type = RTCGetBKPRegister(RTC_BKP_DR4) & 0x00000007;
        block_info[SCRATCH_BLOCK_DEVICE].raw_flag = RTCGetBKPRegister(RTC_BKP_DR4) & RAW_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].mbr_flag = RTCGetBKPRegister(RTC_BKP_DR4) & MBR_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].data_offset = RTCGetBKPRegister(RTC_BKP_DR5);
        block_info[SCRATCH_BLOCK_DEVICE].update_size = RTCGetBKPRegister(RTC_BKP_DR6);
        BOOT_LOG_INF("Custom OTA data");

        /* Print loaded Data */
        BOOT_LOG_INF("Secondary [%d] [%d]", block_info[SECONDARY_BLOCK_DEVICE].storage_type, block_info[SECONDARY_BLOCK_DEVICE].raw_type);
        BOOT_LOG_INF("Scratch [%d] [%d]", block_info[SCRATCH_BLOCK_DEVICE].storage_type, block_info[SCRATCH_BLOCK_DEVICE].raw_type);
    } else {
#if ALL_IN_SD
        block_info[SECONDARY_BLOCK_DEVICE].storage_type = SD_FATFS;
        block_info[SECONDARY_BLOCK_DEVICE].data_offset = 2;
        block_info[SECONDARY_BLOCK_DEVICE].update_size = MCUBOOT_SLOT_SIZE;
        block_info[SECONDARY_BLOCK_DEVICE].raw_type = SDCARD_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].raw_flag = 0;
        block_info[SECONDARY_BLOCK_DEVICE].mbr_flag = 0;

        block_info[SCRATCH_BLOCK_DEVICE].storage_type = SD_FATFS;
        block_info[SCRATCH_BLOCK_DEVICE].data_offset = 2;
        block_info[SCRATCH_BLOCK_DEVICE].update_size = MCUBOOT_SCRATCH_SIZE;
        block_info[SCRATCH_BLOCK_DEVICE].raw_type = SDCARD_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].raw_flag = 0;
        block_info[SCRATCH_BLOCK_DEVICE].mbr_flag = 0;
#elif MIX_SD_QSPI
        block_info[SECONDARY_BLOCK_DEVICE].storage_type = SD_FATFS;
        block_info[SECONDARY_BLOCK_DEVICE].data_offset = 2;
        block_info[SECONDARY_BLOCK_DEVICE].update_size = MCUBOOT_SLOT_SIZE;
        block_info[SECONDARY_BLOCK_DEVICE].raw_type = SDCARD_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].raw_flag = 0;
        block_info[SECONDARY_BLOCK_DEVICE].mbr_flag = 0;

        block_info[SCRATCH_BLOCK_DEVICE].storage_type = QSPI_FLASH_FATFS_MBR;
        block_info[SCRATCH_BLOCK_DEVICE].data_offset = 2;
        block_info[SCRATCH_BLOCK_DEVICE].update_size = MCUBOOT_SCRATCH_SIZE;
        block_info[SCRATCH_BLOCK_DEVICE].raw_type = QSPI_FLASH_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].raw_flag = 0;
        block_info[SCRATCH_BLOCK_DEVICE].mbr_flag = 1;
#else
        block_info[SECONDARY_BLOCK_DEVICE].storage_type = QSPI_FLASH_FATFS_MBR;
        block_info[SECONDARY_BLOCK_DEVICE].data_offset = 2;
        block_info[SECONDARY_BLOCK_DEVICE].update_size = MCUBOOT_SLOT_SIZE;
        block_info[SECONDARY_BLOCK_DEVICE].raw_type = QSPI_FLASH_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].raw_flag = 0;
        block_info[SECONDARY_BLOCK_DEVICE].mbr_flag = 1;

        block_info[SCRATCH_BLOCK_DEVICE].storage_type = QSPI_FLASH_FATFS_MBR;
        block_info[SCRATCH_BLOCK_DEVICE].data_offset = 2;
        block_info[SCRATCH_BLOCK_DEVICE].update_size = MCUBOOT_SCRATCH_SIZE;
        block_info[SCRATCH_BLOCK_DEVICE].raw_type = QSPI_FLASH_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].raw_flag = 0;
        block_info[SCRATCH_BLOCK_DEVICE].mbr_flag = 1;
#endif
        BOOT_LOG_INF("Default OTA data");
    }
    return;
}


static void initBlockTable(void) {
    int err;

    loadOTAData();

    if(block_info[SECONDARY_BLOCK_DEVICE].raw_type == block_info[SCRATCH_BLOCK_DEVICE].raw_type) {
        /* Declare raw block devices */
        if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
            //block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice flashIAP_bd(data_offset, update_size);
            //block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
            BOOT_LOG_ERR("U on IAP");
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
#else
            BOOT_LOG_ERR("SDMMC");
#endif
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
        } else {
            BOOT_LOG_ERR("Config");
        }

        /* Setup sliced block devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                //block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice flashIAP_bd(data_offset, update_size);
                //block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
                BOOT_LOG_ERR("U on IAP");
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
#else
                BOOT_LOG_ERR("SDMMC");
#endif
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
            } else {
                BOOT_LOG_ERR("Config");
            }
        } else

        /* Setup MBR device  */
        if(block_info[SECONDARY_BLOCK_DEVICE].mbr_flag) {
            /* If using the same underlying block device configuration must be the same */
            if(block_info[SECONDARY_BLOCK_DEVICE].storage_type != block_info[SCRATCH_BLOCK_DEVICE].storage_type) {
                BOOT_LOG_ERR("BD U!S");
            }

            /* If using the same underlying block device mbr partition must be the same */
            if(block_info[SECONDARY_BLOCK_DEVICE].data_offset != block_info[SCRATCH_BLOCK_DEVICE].data_offset) {
                BOOT_LOG_ERR("MBR U!S");
            }

            block_info[SECONDARY_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset);
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;

            /* Initialize block device */
            err = block_info[SECONDARY_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Init");
            }
        } else {
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
        }

        /* Setup FS */
        if(!block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if((block_info[SECONDARY_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_APPLICATION_LITTLEFS
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new LittleFileSystem("fs");
#else
                BOOT_LOG_ERR("LFS");
#endif
            } else {
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new FATFileSystem("fs");
            }

            block_info[SCRATCH_BLOCK_DEVICE].log_fs = block_info[SECONDARY_BLOCK_DEVICE].log_fs;

            err = block_info[SECONDARY_BLOCK_DEVICE].log_fs->mount(block_info[SECONDARY_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Mount");
            }

            /* Setup FileBlockDevice */
            block_info[SECONDARY_BLOCK_DEVICE].file_bd = new FileBlockDevice("/fs/update.bin", "rb+", block_info[SECONDARY_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
            block_info[SCRATCH_BLOCK_DEVICE].file_bd = new FileBlockDevice("/fs/scratch.bin", "rb+", block_info[SCRATCH_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }

    } else {
        /* Declare raw block devices */
        if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
            //block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice flashIAP_bd(data_offset, update_size);
            BOOT_LOG_ERR("U on IAP");
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
#else
            BOOT_LOG_ERR("SDMMC");
#endif
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        } else {
            BOOT_LOG_ERR("U config");
        }

        if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
            BOOT_LOG_ERR("S on IAP");
        } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
#else
            BOOT_LOG_ERR("SDMMC");
#endif
        } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        } else {
            BOOT_LOG_ERR("S config");
        }

        /* Setup Raw sliced devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                BOOT_LOG_ERR("U on IAP");
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
#else
                BOOT_LOG_ERR("SDMMC");
#endif
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
            } else {
                BOOT_LOG_ERR("U config");
            }
        } else

        /* Setup MBR devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].mbr_flag) {
            /* Setup MBR devices and FS if scratch and secondary are using different underlying block devices */
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset);

            /* Initialize block device */
            int err = block_info[SECONDARY_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Init U MBR");
            }
        } else {
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
        }

        /* Setup Raw sliced devices */
        if(block_info[SCRATCH_BLOCK_DEVICE].raw_flag) {
            if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                BOOT_LOG_ERR("S on IAP");
            } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_APPLICATION_SDCARD
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset, block_info[SCRATCH_BLOCK_DEVICE].update_size);
#else
                BOOT_LOG_ERR("SDMMC");
#endif
            } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset, block_info[SCRATCH_BLOCK_DEVICE].update_size);
            } else {
                BOOT_LOG_ERR("S config");
            }
        } else

        /* Setup MBR devices */
        if(block_info[SCRATCH_BLOCK_DEVICE].mbr_flag) {
            /* Setup MBR devices and FS if scratch and secondary are using different underlying block devices */
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset);

            /* Initialize block device */
            err = block_info[SCRATCH_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Init S MBR");
            }
        } else {
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SCRATCH_BLOCK_DEVICE].raw_bd;
        }

        /* Setup FS */
        if(!block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if((block_info[SECONDARY_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_APPLICATION_LITTLEFS
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new LittleFileSystem("sec");
#else
                BOOT_LOG_ERR("LFS");
#endif
            } else {
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new FATFileSystem("sec");
            }

            err = block_info[SECONDARY_BLOCK_DEVICE].log_fs->mount(block_info[SECONDARY_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Mount U on LOG");
            }

            /* Setup FileBlockDevice */
            block_info[SECONDARY_BLOCK_DEVICE].file_bd = new FileBlockDevice("/sec/update.bin", "rb+", block_info[SECONDARY_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }

        /* Setup FS */
        if(!block_info[SCRATCH_BLOCK_DEVICE].raw_flag) {
            if((block_info[SCRATCH_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_APPLICATION_LITTLEFS
                block_info[SCRATCH_BLOCK_DEVICE].log_fs = new LittleFileSystem("scr");
#else
                BOOT_LOG_ERR("LFS");
#endif
            } else {
                block_info[SCRATCH_BLOCK_DEVICE].log_fs = new FATFileSystem("scr");
            }

            err = block_info[SCRATCH_BLOCK_DEVICE].log_fs->mount(block_info[SCRATCH_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Mount S on LOG");
            }

            /* Setup FileBlockDevice */
            block_info[SCRATCH_BLOCK_DEVICE].file_bd = new FileBlockDevice("/scr/scratch.bin", "rb+", block_info[SCRATCH_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }
    }
}

mbed::BlockDevice* get_secondary_bd(void) {

    if(!BlockTableLoaded) {
        initBlockTable();
        BlockTableLoaded = true;
    }

    if(block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
        return block_info[SECONDARY_BLOCK_DEVICE].log_bd;
    } else {
        return block_info[SECONDARY_BLOCK_DEVICE].file_bd;
    }
}

mbed::BlockDevice* get_scratch_bd(void) {

    if(!BlockTableLoaded) {
        initBlockTable();
        BlockTableLoaded = true;
    }

    if(block_info[SCRATCH_BLOCK_DEVICE].raw_flag) {
        return block_info[SCRATCH_BLOCK_DEVICE].log_bd;;
    } else {
        return block_info[SCRATCH_BLOCK_DEVICE].file_bd;
    }
}

#endif
