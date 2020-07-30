/*
 * user_main.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#include "BlockDevice.h"

/** Enable serial bootloader feature by default */
#ifndef MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE
#define MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE 1
#endif

#if MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

#include "rtos/ThisThread.h"
#include <chrono>

#define BOOT_WAIT_TIMEOUT 5s

using namespace std::chrono;

#endif // MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

mbed::BlockDevice* mcuboot_secondary_bd;

void mbed_mcuboot_user_init(void) {

    mbed::BlockDevice* mcuboot_secondary_bd = mbed::BlockDevice::get_default_instance();

    // Initialize the secondary/update candidate block device
    mcuboot_secondary_bd->init();

#if MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

    // Set up the serial bootloader

#endif // MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

}



