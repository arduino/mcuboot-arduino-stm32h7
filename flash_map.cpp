/*
 * flash_map.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

mbed::BlockDevice* mcuboot_secondary_bd = mbed::BlockDevice::get_default_instance();

void mbed_mcuboot_user_init(void) {
	mcuboot_secondary_bd->init();
}
