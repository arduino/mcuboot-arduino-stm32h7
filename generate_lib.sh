#!/bin/bash
rm -rf BUILD/PORTENTA_H7_M7/GCC_ARM/
mbed compile -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_log_off.json
cd BUILD/PORTENTA_H7_M7/GCC_ARM/
rm -rf mbed-os/
find . -name "*.o" | xargs arm-none-eabi-ar -csr libbootutil.a