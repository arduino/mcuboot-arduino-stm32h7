/*
 * default_bd.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

#include "SlicingBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FileBlockDevice.h"
#include "FATFileSystem.h"

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

BlockDevice *BlockDevice::get_default_instance()
{
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

}

#define MCUBOOT_USE_FILE_BD

/**
 * You can override this function to suit your hardware/memory configuration
 * By default it simply returns what is returned by BlockDevice::get_default_instance();
 */
mbed::BlockDevice* get_secondary_bd(void) {
    // In this case, our flash is much larger than a single image so
    // slice it into the size of an image slot
    mbed::BlockDevice* default_bd = mbed::BlockDevice::get_default_instance();
#if !defined MCUBOOT_USE_FILE_BD
    static mbed::SlicingBlockDevice sliced_bd(default_bd, 0x0, MCUBOOT_SLOT_SIZE);
    return &sliced_bd;
#else
    static mbed::MBRBlockDevice mbr_bd(default_bd, 2);
    static mbed::FATFileSystem secondary_bd_fs("fs");

    mbr_bd.init();
    int err =  secondary_bd_fs.mount(&mbr_bd);
    static mbed::FileBlockDevice file_bd(&mbr_bd, "/fs/update.bin", O_RDWR, MCUBOOT_SLOT_SIZE);
    return &file_bd;
#endif
}

