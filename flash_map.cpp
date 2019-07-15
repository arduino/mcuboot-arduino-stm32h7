/*
 * flash_map.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"
#include "QSPIFBlockDevice.h"

QSPIFBlockDevice qspi_bd(QSPI_FLASH1_IO0, QSPI_FLASH1_IO1, QSPI_FLASH1_IO2, QSPI_FLASH1_IO3,
		QSPI_FLASH1_SCK, QSPI_FLASH1_CSN, QSPIF_POLARITY_MODE_0, MBED_CONF_QSPIF_QSPI_FREQ);

mbed::BlockDevice* mcuboot_secondary_bd = (mbed::BlockDevice*) &qspi_bd;

void mbed_mcuboot_user_init(void) {
	qspi_bd.init();
}
