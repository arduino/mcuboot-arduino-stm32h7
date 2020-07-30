/*
 * default_bd.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

#if COMPONENT_SPIF
#include "components/storage/blockdevice/COMPONENT_SPIF/SPIFBlockDevice.h"
#endif

#if COMPONENT_QSPIF
#include "components/storage/blockdevice/COMPONENT_QSPIF/QSPIFBlockDevice.h"
#endif

#if COMPONENT_DATAFLASH
#include "components/storage/blockdevice/COMPONENT_DATAFLASH/DataFlashBlockDevice.h"
#endif

#if COMPONENT_SD
#include "components/storage/blockdevice/COMPONENT_SD/SDBlockDevice.h"

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

    static QSPIFBlockDevice default_bd;

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

