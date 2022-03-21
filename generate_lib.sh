#!/bin/bash
mbed compile -c -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_bootutil.json
echo
echo Generating library
find ./BUILD/PORTENTA_H7_M7/GCC_ARM/ \( -name "FileBlockDevice.o" -o -name "rtc.o" -o -name "ota.o" -o -name "bootutil_extra.o" -o -name "flash_map_backend.o" -o -name "bootutil_public.o" \) | xargs arm-none-eabi-ar -csr libbootutil.a
echo -n "Library: "
find ./ -name "libbootutil.a"
echo
