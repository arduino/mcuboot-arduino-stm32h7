#!/bin/bash
rm -rf BUILD/PORTENTA_H7_M7/GCC_ARM/
mbed compile -m PORTENTA_H7_M7 -t GCC_ARM 
cd BUILD/PORTENTA_H7_M7/GCC_ARM/
mv mbed-os/targets/TARGET_STM/TARGET_STM32H7/TARGET_STM32H747xI/TARGET_PORTENTA_H7/FileBlockDevice.o .
rm -rf mbed-os/
find . -name "*.o" | xargs arm-none-eabi-ar -csr libbootutil.a