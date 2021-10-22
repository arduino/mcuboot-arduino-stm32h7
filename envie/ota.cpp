#if MCUBOOT_AS_ENVIE

#include "ota.h"
#include "rtc.h"
#include "bootutil/bootutil_log.h"


#include "SlicingBlockDevice.h"
#include "FlashIAPBlockDevice.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FileBlockDevice.h"
#include "FATFileSystem.h"

#if MCUBOOT_ENVIE_SDCARD
#include "SDMMCBlockDevice.h"
#endif

#if MCUBOOT_ENVIE_LITTLEFS
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
#elif SCRATCH_INTERNAL_FLASH
        /* TODO Check error readinf internal flash */
        block_info[SECONDARY_BLOCK_DEVICE].storage_type = QSPI_FLASH_FATFS_MBR;
        block_info[SECONDARY_BLOCK_DEVICE].data_offset = 2;
        block_info[SECONDARY_BLOCK_DEVICE].update_size = MCUBOOT_SLOT_SIZE;
        block_info[SECONDARY_BLOCK_DEVICE].raw_type = QSPI_FLASH_FLAG;
        block_info[SECONDARY_BLOCK_DEVICE].raw_flag = 0;
        block_info[SECONDARY_BLOCK_DEVICE].mbr_flag = 1;

        block_info[SCRATCH_BLOCK_DEVICE].storage_type = INTERNAL_FLASH_OFFSET;
        block_info[SCRATCH_BLOCK_DEVICE].data_offset = MCUBOOT_SCRATCH_START_ADDR;
        block_info[SCRATCH_BLOCK_DEVICE].update_size = MCUBOOT_SCRATCH_SIZE;
        block_info[SCRATCH_BLOCK_DEVICE].raw_type = INTERNAL_FLASH_FLAG;
        block_info[SCRATCH_BLOCK_DEVICE].raw_flag = 1;
        block_info[SCRATCH_BLOCK_DEVICE].mbr_flag = 0;
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

    /* Print loaded Data */
    BOOT_LOG_INF("Secondary BlockDevice");
    BOOT_LOG_INF("Storage type: %d", block_info[SECONDARY_BLOCK_DEVICE].storage_type);
    BOOT_LOG_INF("Raw type: %d", block_info[SECONDARY_BLOCK_DEVICE].raw_type);
    BOOT_LOG_INF("Scratch BlockDevice");
    BOOT_LOG_INF("Storage type: %d", block_info[SCRATCH_BLOCK_DEVICE].storage_type);
    BOOT_LOG_INF("Raw type: %d", block_info[SCRATCH_BLOCK_DEVICE].raw_type);
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
            BOOT_LOG_ERR("Secondary block device on internal flash not supported");
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
#else
            BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
        } else {
            BOOT_LOG_ERR("Cannot configure secondary/scratch raw block device");
        }

        /* Setup sliced block devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                //block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice flashIAP_bd(data_offset, update_size);
                //block_info[SCRATCH_BLOCK_DEVICE].raw_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
                BOOT_LOG_ERR("Secondary block device on internal flash not supported");
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
#else
                BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
            } else {
                BOOT_LOG_ERR("Cannot configure secondary/scratch raw block device");
            }
        } else

        /* Setup MBR device  */
        if(block_info[SECONDARY_BLOCK_DEVICE].mbr_flag) {
            /* If using the same underlying block device configuration must be the same */
            if(block_info[SECONDARY_BLOCK_DEVICE].storage_type != block_info[SCRATCH_BLOCK_DEVICE].storage_type) {
                BOOT_LOG_ERR("Secondary and Scratch storage type must be the same");
            }

            /* If using the same underlying block device mbr partition must be the same */
            if(block_info[SECONDARY_BLOCK_DEVICE].data_offset != block_info[SCRATCH_BLOCK_DEVICE].data_offset) {
                BOOT_LOG_ERR("Secondary and Scratch mbr partition must be the same");
            }

            block_info[SECONDARY_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset);
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;

            /* Initialize block device */
            err = block_info[SECONDARY_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Error initializing common mbr device");
            }
        } else {
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].log_bd;
        }

        /* Setup FS */
        if(!block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if((block_info[SECONDARY_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_ENVIE_LITTLEFS
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new LittleFileSystem("fs");
#else
                BOOT_LOG_ERR("LittleFileSystem not supported");
#endif
            } else {
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new FATFileSystem("fs");
            }

            block_info[SCRATCH_BLOCK_DEVICE].log_fs = block_info[SECONDARY_BLOCK_DEVICE].log_fs;

            err = block_info[SECONDARY_BLOCK_DEVICE].log_fs->mount(block_info[SECONDARY_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting fs on common mbr device");
            }

            /* Setup FileBlockDevice */
            block_info[SECONDARY_BLOCK_DEVICE].file_bd = new FileBlockDevice("/fs/update.bin", "rb+", block_info[SECONDARY_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
            block_info[SCRATCH_BLOCK_DEVICE].file_bd = new FileBlockDevice("/fs/scratch.bin", "rb+", block_info[SCRATCH_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }

    } else {
        /* Declare raw block devices */
        if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
            //block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice flashIAP_bd(data_offset, update_size);
            BOOT_LOG_ERR("Secondary block device on internal flash not supported");
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
#else
            BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
        } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SECONDARY_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        } else {
            BOOT_LOG_ERR("Cannot configure secodary raw block device");
        }

        if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = new FlashIAPBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].data_offset, block_info[SCRATCH_BLOCK_DEVICE].update_size);
        } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = new SDMMCBlockDevice();
#else
            BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
        } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
            block_info[SCRATCH_BLOCK_DEVICE].raw_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13, PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        } else {
            BOOT_LOG_ERR("Cannot configure scratchraw block device");
        }

        /* Setup Raw sliced devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                BOOT_LOG_ERR("Secondary block device on internal flash not supported");
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
#else
                BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
            } else if (block_info[SECONDARY_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SECONDARY_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset, block_info[SECONDARY_BLOCK_DEVICE].update_size);
            } else {
                BOOT_LOG_ERR("Cannot configure secodary raw block device");
            }
        } else

        /* Setup MBR devices */
        if(block_info[SECONDARY_BLOCK_DEVICE].mbr_flag) {
            /* Setup MBR devices and FS if scratch and secondary are using different underlying block devices */
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SECONDARY_BLOCK_DEVICE].raw_bd, block_info[SECONDARY_BLOCK_DEVICE].data_offset);

            /* Initialize block device */
            int err = block_info[SECONDARY_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Error initializing secondary mbr device");
            }
        } else {
            block_info[SECONDARY_BLOCK_DEVICE].log_bd = block_info[SECONDARY_BLOCK_DEVICE].raw_bd;
        }

        /* Setup Raw sliced devices */
        if(block_info[SCRATCH_BLOCK_DEVICE].raw_flag) {
            if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & INTERNAL_FLASH_FLAG) {
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SCRATCH_BLOCK_DEVICE].raw_bd;
            } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & SDCARD_FLAG) {
#if MCUBOOT_ENVIE_SDCARD
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset, block_info[SCRATCH_BLOCK_DEVICE].update_size);
#else
                BOOT_LOG_ERR("SDMMCBlockDevice not supported");
#endif
            } else if (block_info[SCRATCH_BLOCK_DEVICE].raw_type & QSPI_FLASH_FLAG) {
                block_info[SCRATCH_BLOCK_DEVICE].log_bd = new SlicingBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset, block_info[SCRATCH_BLOCK_DEVICE].update_size);
            } else {
                BOOT_LOG_ERR("Cannot configure scratch raw block device");
            }
        } else

        /* Setup MBR devices */
        if(block_info[SCRATCH_BLOCK_DEVICE].mbr_flag) {
            /* Setup MBR devices and FS if scratch and secondary are using different underlying block devices */
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = new MBRBlockDevice(block_info[SCRATCH_BLOCK_DEVICE].raw_bd, block_info[SCRATCH_BLOCK_DEVICE].data_offset);

            /* Initialize block device */
            err = block_info[SCRATCH_BLOCK_DEVICE].log_bd->init();
            if (err) {
                BOOT_LOG_ERR("Error initializing scratch mbr device");
            }
        } else {
            block_info[SCRATCH_BLOCK_DEVICE].log_bd = block_info[SCRATCH_BLOCK_DEVICE].raw_bd;
        }

        /* Setup FS */
        if(!block_info[SECONDARY_BLOCK_DEVICE].raw_flag) {
            if((block_info[SECONDARY_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_ENVIE_LITTLEFS
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new LittleFileSystem("sec");
#else
                BOOT_LOG_ERR("LittleFileSystem not supported");
#endif
            } else {
                block_info[SECONDARY_BLOCK_DEVICE].log_fs = new FATFileSystem("sec");
            }

            err = block_info[SECONDARY_BLOCK_DEVICE].log_fs->mount(block_info[SECONDARY_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting secondary fs on common log device");
            }

            /* Setup FileBlockDevice */
            block_info[SECONDARY_BLOCK_DEVICE].file_bd = new FileBlockDevice("/sec/update.bin", "rb+", block_info[SECONDARY_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }

        /* Setup FS */
        if(!block_info[SCRATCH_BLOCK_DEVICE].raw_flag) {
            if((block_info[SCRATCH_BLOCK_DEVICE].storage_type & LITTLEFS_FLAG)) {
#if MCUBOOT_ENVIE_LITTLEFS
                block_info[SCRATCH_BLOCK_DEVICE].log_fs = new LittleFileSystem("scr");
#else
                BOOT_LOG_ERR("LittleFileSystem not supported");
#endif
            } else {
                block_info[SCRATCH_BLOCK_DEVICE].log_fs = new FATFileSystem("scr");
            }

            err = block_info[SCRATCH_BLOCK_DEVICE].log_fs->mount(block_info[SCRATCH_BLOCK_DEVICE].log_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting scratch fs on common log device");
            }

            /* Setup FileBlockDevice */
            block_info[SCRATCH_BLOCK_DEVICE].file_bd = new FileBlockDevice("/scr/scratch.bin", "rb+", block_info[SCRATCH_BLOCK_DEVICE].update_size, FILEBD_READ_SIZE, FILEBD_WRITE_SIZE, FILEBD_ERASE_SIZE);
        }
    }
}

/*
 * MCUBOOT_AS_ENVIE     -> Secondary Block device defined by getOTAData [SDCARD, QSPI] Internal flash not supported
 *                      -> Scratch Block device defined by getOTAData [SDCARD, QSPI, FLASH]
 *
 *                      -> Secondary Block device on RAW QSPI @0x0000000
 *                      -> Scratch Block device on Internal flash @MCUBOOT_SCRATCH_START_ADDR
 */
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
