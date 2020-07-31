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

void mbed_mcuboot_user_init(void) {

#if MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

    // Set up the serial bootloader

#endif // MBED_CONF_APP_SERIAL_BOOTLOADER_ENABLE

}



