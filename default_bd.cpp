/*
 * default_bd.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

#include "SlicingBlockDevice.h"
#if MCUBOOT_AS_ENVIE || MCUBOOT_USE_FILE_BD
#include "MBRBlockDevice.h"
#include "FileBlockDevice.h"
#include "FATFileSystem.h"
#endif

#if MCUBOOT_AS_ENVIE
#include "FlashIAPBlockDevice.h"
#include "SDMMCBlockDevice.h"
//#include "LittleFileSystem.h"
#include "ota.h"
#endif

#if COMPONENT_SPIF
#include "SPIFBlockDevice.h"
#endif

#if COMPONENT_QSPIF
#include "QSPIFBlockDevice.h"
#endif

#if COMPONENT_DATAFLASH
#include "DataFlashBlockDevice.h"
#endif

#if COMPONENT_SD
#include "SDBlockDevice.h"

#if (STATIC_PINMAP_READY)
const spi_pinmap_t static_spi_pinmap = get_spi_pinmap(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, NC);
#endif
#endif

#if MCUBOOT_AS_ENVIE
static BlockDevice *default_bd = NULL;
static MBRBlockDevice *logical_bd = NULL;
#endif

BlockDevice *BlockDevice::get_default_instance()
{
#if MCUBOOT_AS_ENVIE
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;
    BlockDevice *raw_bd = NULL;

    int rc = getOTAData(&storage_type, &data_offset, &update_size);
    if (rc!=0) {
        printf("OTA not configured using default configuration\n");
        storage_type = QSPI_FLASH_FATFS_MBR;
    }

    if (storage_type & INTERNAL_FLASH_FLAG) {
        if (storage_type & (FATFS_FLAG | LITTLEFS_FLAG)) {
            // have a filesystem, use offset as partition start
            printf("INTERNAL_FLASH + FS\n");
            static FlashIAPBlockDevice flashIAP_bd(0x8000000 + data_offset, 2 * 1024 * 1024 - data_offset);
            raw_bd = &flashIAP_bd;
        } else {
            // raw device, no offset
            printf("INTERNAL_FLASH\n");
            static FlashIAPBlockDevice flashIAP_bd(0x8000000, 2 * 1024 * 1024);
            raw_bd = &flashIAP_bd;
        }
    }

    if (storage_type & SDCARD_FLAG) {
        printf("SD_FLASH \n");
        static SDMMCBlockDevice SDMMC_bd;
        raw_bd = &SDMMC_bd;
    }

    if (storage_type & QSPI_FLASH_FLAG) {
        printf("QSPI_FLASH\n");
        static QSPIFBlockDevice QSPIF_bd(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        raw_bd = &QSPIF_bd;
    }

    return raw_bd;
#else //MCUBOOT_AS_ENVIE
#if COMPONENT_SPIF

    static SPIFBlockDevice default_bd;

    return &default_bd;

#elif COMPONENT_QSPIF

    static QSPIFBlockDevice default_bd(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);

    return &default_bd;

#elif COMPONENT_DATAFLASH

    static DataFlashBlockDevice default_bd;

    return &default_bd;

#elif COMPONENT_SD

#if (STATIC_PINMAP_READY)
    static SDBlockDevice default_bd(
        static_spi_pinmap,
        MBED_CONF_SD_SPI_CS
    );
#else
    static SDBlockDevice default_bd;
#endif

    return &default_bd;

#else

    return NULL;

#endif

#endif //MCUBOOT_AS_ENVIE
}

/**
 * You can override this function to suit your hardware/memory configuration
 * By default it simply returns what is returned by BlockDevice::get_default_instance();
 */
mbed::BlockDevice* get_secondary_bd(void) {
    // In this case, our flash is much larger than a single image so
    // slice it into the size of an image slot
#if MCUBOOT_AS_ENVIE
    default_bd = mbed::BlockDevice::get_default_instance();
#else
    mbed::BlockDevice* default_bd = mbed::BlockDevice::get_default_instance();
#endif
#if !defined MCUBOOT_USE_FILE_BD
    static mbed::SlicingBlockDevice sliced_bd(default_bd, 0x0, MCUBOOT_SLOT_SIZE);
    return &sliced_bd;
#else

#if MCUBOOT_AS_ENVIE
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;

    int rc = getOTAData(&storage_type, &data_offset, &update_size);
    if (rc != 0) {
        printf("OTA not configured using default configuration\n");
        storage_type = QSPI_FLASH_FATFS_MBR;
        data_offset = 2;
        update_size = MCUBOOT_SLOT_SIZE;
    }

    if (storage_type & MBR_FLAG)  {
        printf("MBR \n");
        logical_bd = new MBRBlockDevice(default_bd, data_offset);
        int err = logical_bd->init();
        if (err) {
            printf("Error initializing mbr device\n");
        }

        if((storage_type & LITTLEFS_FLAG)) {
            printf("LITTLEFS \n");
            //static LittleFileSystem secondary_bd_fs("secondary");

            //int err = secondary_bd_fs.mount(logical_bd);
            //if (err) {
            //    printf("Error mounting secondary fs\n");
            //}
        } else {
            printf("FATFS \n");
            static FATFileSystem secondary_bd_fs("secondary");

            int err = secondary_bd_fs.mount(logical_bd);
            if (err) {
                printf("Error mounting secondary fs\n");
            }
        }

        static mbed::FileBlockDevice file_bd(logical_bd, "/secondary/update.bin", "rb+", update_size);
        return &file_bd;
    } else {
         if((storage_type & LITTLEFS_FLAG)) {
            printf("LITTLEFS \n");
            //static LittleFileSystem secondary_bd_fs("secondary");

            //int err = secondary_bd_fs.mount(default_bd);
            //if (err) {
            //    printf("Error mounting secondary fs\n");
            //}
        } else {
            printf("FATFS \n");
            static FATFileSystem secondary_bd_fs("secondary");

            int err = secondary_bd_fs.mount(default_bd);
            if (err) {
            printf("Error mounting secondary fs\n");
            }
        }

        static mbed::FileBlockDevice file_bd(default_bd, "/secondary/update.bin", "rb+", update_size);
        return &file_bd;
    }

    return NULL;
#else
    static mbed::MBRBlockDevice mbr_bd(default_bd, 2);

    int err = mbr_bd.init();
    if (err) {
        printf("Error initializing mbr device\n");
    }

    static mbed::FATFileSystem secondary_bd_fs("secondary");
    err = secondary_bd_fs.mount(&mbr_bd);
    if (err) {
        printf("Error mounting secondary fs\n");
    }
    static mbed::FileBlockDevice file_bd(&mbr_bd, "/secondary/update.bin", "rb+", MCUBOOT_SLOT_SIZE);
    return &file_bd;
#endif //MCUBOOT_AS_ENVIE

#endif //MCUBOOT_USE_FILE_BD
}

mbed::BlockDevice* get_scratch_bd(void) {
#if !defined MCUBOOT_USE_FILE_BD
    static FlashIAPBlockDevice scratch_bd(MCUBOOT_SCRATCH_START_ADDR, MCUBOOT_SCRATCH_SIZE);
    return &scratch_bd;
#else
#if MCUBOOT_AS_ENVIE
    storageType storage_type;
    uint32_t data_offset;
    uint32_t update_size;

    int rc = getOTAData(&storage_type, &data_offset, &update_size);
    if (rc != 0) {
        printf("OTA not configured using default configuration\n");
        storage_type = QSPI_FLASH_FATFS_MBR;
        data_offset = 2;
        update_size = MCUBOOT_SLOT_SIZE;
    }

    if(!(storage_type & QSPI_FLASH_FLAG)) {
        default_bd = new QSPIFBlockDevice(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
        logical_bd = new MBRBlockDevice(default_bd, 2);

        int err = logical_bd->init();
        if (err) {
            printf("Error initializing scratch mbr device\n");
        }

        static mbed::FATFileSystem secondary_bd_fs("scratch");
        err = secondary_bd_fs.mount(logical_bd);
        if (err) {
            printf("Error mounting scratch fs\n");
        }
    }

    static mbed::FileBlockDevice file_bd(logical_bd, "/scratch/scratch.bin", "rb+", MCUBOOT_SCRATCH_SIZE);
    return &file_bd;
#else
    mbed::BlockDevice* default_bd = mbed::BlockDevice::get_default_instance();
    static mbed::MBRBlockDevice mbr_bd(default_bd, 2);

    int err = mbr_bd.init();
    if (err) {
        printf("Error initializing mbr device\n");
    }

    static mbed::FATFileSystem secondary_bd_fs("scratch");
    err = secondary_bd_fs.mount(&mbr_bd);
    if (err) {
        printf("Error mounting scratch fs\n");
    }
    static mbed::FileBlockDevice file_bd(&mbr_bd, "/scratch/scratch.bin", "rb+", MCUBOOT_SCRATCH_SIZE);
    return &file_bd;
#endif //MCUBOOT_AS_ENVIE
#endif
}

