#if MCUBOOT_AS_ENVIE

#include "ota.h"
#include "rtc.h"
#include "bootutil/bootutil_log.h"

#include "BlockDevice.h"
#include "SlicingBlockDevice.h"
#include "FlashIAPBlockDevice.h"
#include "QSPIFBlockDevice.h"
#include "SDMMCBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FileBlockDevice.h"
#include "FATFileSystem.h"
#include "bootutil/bootutil_log.h"

#if MCUBOOT_ENVIE_LITTLEFS
#include "LittleFileSystem.h"
#endif

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

/*
 * MCUBOOT_AS_ENVIE     -> Secondary Block device defined by getOTAData [SDCARD, QSPI] Internal flash not supported
 *                      -> Scratch Block device defined by getOTAData [SDCARD, QSPI, FLASH]
 *
 *                      -> Secondary Block device on RAW QSPI @0x0000000
 *                      -> Scratch Block device on Internal flash @MCUBOOT_SCRATCH_START_ADDR
 */

BlockDevice *BlockDevice::get_default_instance()
{
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;

    getOTAData(&storage_type, &data_offset, &update_size);

    /* Used only by Scratch block device */
    if (storage_type & INTERNAL_FLASH_FLAG) {
        if (storage_type & (FATFS_FLAG | LITTLEFS_FLAG)) {
            // have a filesystem, use offset as partition start
            static FlashIAPBlockDevice flashIAP_bd(0x8000000 + data_offset, 2 * 1024 * 1024 - data_offset);
            return &flashIAP_bd;
        } else {
            // raw device, no offset
            static FlashIAPBlockDevice flashIAP_bd(0x8000000, 2 * 1024 * 1024);
            return &flashIAP_bd;
        }
    }

#if MCUBOOT_ENVIE_SDCARD
    if (storage_type & SDCARD_FLAG) {
        static SDMMCBlockDevice SDMMC_bd;
        BOOT_LOG_INF("SDMMCBlockDevice");
        return &SDMMC_bd;
    }
#endif

    if (storage_type & QSPI_FLASH_FLAG) {
        static QSPIFBlockDevice QSPIF_bd(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        BOOT_LOG_INF("QSPIFBlockDevice");
        return &QSPIF_bd;
    }

    return NULL;
}

/**
 * You can override this function to suit your hardware/memory configuration
 * By default it simply returns what is returned by BlockDevice::get_default_instance();
 */
mbed::BlockDevice* get_secondary_bd(void) {
    // In this case, our flash is much larger than a single image so
    // slice it into the size of an image slot
    mbed::BlockDevice* default_bd = mbed::BlockDevice::get_default_instance();
    mbed::BlockDevice* logical_bd;
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;

    getOTAData(&storage_type, &data_offset, &update_size);

    if (storage_type & MBR_FLAG)  {
        logical_bd = new MBRBlockDevice(default_bd, data_offset);
        int err = logical_bd->init();
        if (err) {
            BOOT_LOG_ERR("Error initializing secondary mbr device");
        }
#if MCUBOOT_ENVIE_LITTLEFS
        if((storage_type & LITTLEFS_FLAG)) {
            static LittleFileSystem secondary_bd_fs("secondary");
            err = secondary_bd_fs.mount(logical_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting littlefs on secondary mbr device");
            }
        } else {
#endif
            static FATFileSystem secondary_bd_fs("secondary");
            err = secondary_bd_fs.mount(logical_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting fatfs on secondary mbr device");
            }
#if MCUBOOT_ENVIE_LITTLEFS
        }
#endif

        static mbed::FileBlockDevice file_bd("/secondary/update.bin", "rb+", update_size);
        return &file_bd;
    } else {
        int err = default_bd->init();
        if (err) {
            BOOT_LOG_ERR("Error initializing secondary raw device");
        }
#if MCUBOOT_ENVIE_LITTLEFS
         if((storage_type & LITTLEFS_FLAG)) {
            static LittleFileSystem secondary_bd_fs("secondary");
            err = secondary_bd_fs.mount(default_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting littlefs on secondary raw device");
            }
        } else {
#endif
            static FATFileSystem secondary_bd_fs("secondary");
            err = secondary_bd_fs.mount(default_bd);
            if (err) {
                BOOT_LOG_ERR("Error mounting fatfs on secondary raw device");
            }
#if MCUBOOT_ENVIE_LITTLEFS
        }
#endif

        static mbed::FileBlockDevice file_bd("/secondary/update.bin", "rb+", update_size);
        return &file_bd;
    }

    return NULL;
}

mbed::BlockDevice* get_scratch_bd(void) {
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;

    getOTAData(&storage_type, &data_offset, &update_size);

    /*
     * If the secondary slot is not into the internal QSPI flash, i.e. SDMCC; initialize
     * QSPI storage and mount the filesystem.
     *
     * WARNING: by default we are assuming the QSPI flash is formatted as MBR device with FAT
     *
     */


        mbed::BlockDevice* default_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        mbed::BlockDevice* logical_bd = new MBRBlockDevice(default_bd, 2);

        int err = logical_bd->init();
        if (err) {
            BOOT_LOG_ERR("Error initializing scratch mbr device");
        }

        static mbed::FATFileSystem secondary_bd_fs("scratch");
        err = secondary_bd_fs.mount(logical_bd);
        if (err) {
            BOOT_LOG_ERR("Error mounting fatfs on scratch mbr device");
        }


    static mbed::FileBlockDevice file_bd("/scratch/scratch.bin", "rb+", MCUBOOT_SCRATCH_SIZE);
    return &file_bd;
}

#endif
