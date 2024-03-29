#!/bin/bash
rm -rf release
rm -rf BUILD

mbed compile -c -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_bootutil.json

set -e

echo
echo Generating bootutil library
find ./BUILD/PORTENTA_H7_M7/GCC_ARM/ \( -name "FileBlockDevice.o" -o -name "BSP.o" -o -name "SDMMCBlockDevice.o" -o -name "rtc.o" -o -name "default_bd.o" -o -name "bootutil_extra.o" -o -name "flash_map_backend.o" -o -name "bootutil_public.o" \) | xargs arm-none-eabi-ar -csr libbootutil.a
echo -n "Library: "
find ./ -name "libbootutil.a"

if [[ $1 == "portenta" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for PORTENTA H7
mbed compile -c -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_portenta.json --profile=custom.json -N mcuboot_portenta_h7
mkdir -p release/PORTENTA_H7
cp ./libbootutil.a ./release/PORTENTA_H7
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7.bin ./release/PORTENTA_H7/mcuboot_portenta_h7.bin
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7_application.elf ./release/PORTENTA_H7/mcuboot_portenta_h7.elf
xxd -i ./release/PORTENTA_H7/mcuboot_portenta_h7.bin > ./release/PORTENTA_H7/mcuboot_portenta_h7.h
sed -i "s/unsigned char __release_PORTENTA_H7_mcuboot_portenta_h7_bin/const unsigned char mcuboot_portenta_h7_bin/" ./release/PORTENTA_H7/mcuboot_portenta_h7.h
sed -i "s/__release_PORTENTA_H7_mcuboot_portenta_h7_bin_len/mcuboot_portenta_h7_bin_len/" ./release/PORTENTA_H7/mcuboot_portenta_h7.h
tar -czvf ./release/PORTENTA_H7.tar.gz -C ./release/ PORTENTA_H7
fi

if [[ $1 == "lite" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for PORTENTA H7 Lite
mbed compile -c -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_portenta_lite.json --profile=custom.json -N mcuboot_portenta_h7_lite
mkdir -p release/PORTENTA_H7_Lite
cp ./libbootutil.a ./release/PORTENTA_H7_Lite
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7_lite.bin ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.bin
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7_lite_application.elf ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.elf
xxd -i ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.bin > ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.h
sed -i "s/unsigned char __release_PORTENTA_H7_mcuboot_portenta_h7_lite_bin/const unsigned char mcuboot_portenta_h7_lite_bin/" ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.h
sed -i "s/__release_PORTENTA_H7_mcuboot_portenta_h7_lite_bin_len/mcuboot_portenta_h7_lite_bin_len/" ./release/PORTENTA_H7_Lite/mcuboot_portenta_h7_lite.h
tar -czvf ./release/PORTENTA_H7_Lite.tar.gz -C ./release/ PORTENTA_H7_Lite
fi

if [[ $1 == "connected" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for PORTENTA H7 Lite Connected
mbed compile -c -m PORTENTA_H7_M7 -t GCC_ARM --app=mbed_app_portenta_lite_connected.json --profile=custom.json -N mcuboot_portenta_h7_lite_connected
mkdir -p release/PORTENTA_H7_Lite_Connected
cp ./libbootutil.a ./release/PORTENTA_H7_Lite_Connected
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7_lite_connected.bin ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.bin
cp ./BUILD/PORTENTA_H7_M7/GCC_ARM-CUSTOM/mcuboot_portenta_h7_lite_connected_application.elf ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.elf
xxd -i ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.bin > ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.h
sed -i "s/unsigned char __release_PORTENTA_H7_mcuboot_portenta_h7_lite_connected_bin/const unsigned char mcuboot_portenta_h7_lite_connected_bin/" ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.h
sed -i "s/__release_PORTENTA_H7_mcuboot_portenta_h7_lite_connected_bin_len/mcuboot_portenta_h7_lite_connected_bin_len/" ./release/PORTENTA_H7_Lite_Connected/mcuboot_portenta_h7_lite_connected.h
tar -czvf ./release/PORTENTA_H7_Lite_Connected.tar.gz -C ./release/ PORTENTA_H7_Lite_Connected
fi

if [[ $1 == "nicla" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for NICLA VISION
mbed compile -c -m NICLA_VISION -t GCC_ARM --app=mbed_app_nicla_vision.json --profile=custom.json -N mcuboot_nicla_vision
mkdir -p release/NICLA_VISION
cp ./libbootutil.a ./release/NICLA_VISION
cp ./BUILD/NICLA_VISION/GCC_ARM-CUSTOM/mcuboot_nicla_vision.bin ./release/NICLA_VISION/mcuboot_nicla_vision.bin
cp ./BUILD/NICLA_VISION/GCC_ARM-CUSTOM/mcuboot_nicla_vision_application.elf ./release/NICLA_VISION/mcuboot_nicla_vision.elf
xxd -i ./release/NICLA_VISION/mcuboot_nicla_vision.bin > ./release/NICLA_VISION/mcuboot_nicla_vision.h
sed -i "s/unsigned char __release_NICLA_VISION_mcuboot_nicla_vision_bin/const unsigned char mcuboot_nicla_vision_bin/" ./release/NICLA_VISION/mcuboot_nicla_vision.h
sed -i "s/__release_NICLA_VISION_mcuboot_nicla_vision_bin_len/mcuboot_nicla_vision_bin_len/" ./release/NICLA_VISION/mcuboot_nicla_vision.h
tar -czvf ./release/NICLA_VISION.tar.gz -C ./release/ NICLA_VISION
fi

if [[ $1 == "opta" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for OPTA
mbed compile -c -m OPTA -t GCC_ARM --app=mbed_app_opta.json --profile=custom.json -N mcuboot_opta
mkdir -p release/OPTA
cp ./libbootutil.a ./release/OPTA
cp ./BUILD/OPTA/GCC_ARM-CUSTOM/mcuboot_opta.bin ./release/OPTA/mcuboot_opta.bin
cp ./BUILD/OPTA/GCC_ARM-CUSTOM/mcuboot_opta_application.elf ./release/OPTA/mcuboot_opta.elf
xxd -i ./release/OPTA/mcuboot_opta.bin > ./release/OPTA/mcuboot_opta.h
sed -i "s/unsigned char __release_OPTA_mcuboot_opta_bin/const unsigned char mcuboot_opta_bin/" ./release/OPTA/mcuboot_opta.h
sed -i "s/__release_OPTA_mcuboot_opta_bin_len/mcuboot_opta_bin_len/" ./release/OPTA/mcuboot_opta.h
tar -czvf ./release/OPTA.tar.gz -C ./release/ OPTA
fi

if [[ $1 == "giga" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for GIGA
mbed compile -c -m GIGA -t GCC_ARM --app=mbed_app_giga.json --profile=custom.json -N mcuboot_giga
mkdir -p release/GIGA
cp ./libbootutil.a ./release/GIGA
cp ./BUILD/GIGA/GCC_ARM-CUSTOM/mcuboot_giga.bin ./release/GIGA/mcuboot_giga.bin
cp ./BUILD/GIGA/GCC_ARM-CUSTOM/mcuboot_giga_application.elf ./release/GIGA/mcuboot_giga.elf
xxd -i ./release/GIGA/mcuboot_giga.bin > ./release/GIGA/mcuboot_giga.h
sed -i "s/unsigned char __release_GIGA_mcuboot_giga_bin/const unsigned char mcuboot_giga_bin/" ./release/GIGA/mcuboot_giga.h
sed -i "s/__release_GIGA_mcuboot_opta_bin_len/mcuboot_giga_bin_len/" ./release/GIGA/mcuboot_giga.h
tar -czvf ./release/GIGA.tar.gz -C ./release/ GIGA
fi

if [[ $1 == "gigaw" ]] || [[ $1 == "all" ]]; then
echo
echo Generating binaries for GIGA_WiFi
mbed compile -c -m GIGA -t GCC_ARM --app=mbed_app_giga_wifi.json --profile=custom.json -N mcuboot_giga_wifi
mkdir -p release/GIGA_WiFi
mv ./libbootutil.a ./release/GIGA_WiFi
cp ./BUILD/GIGA/GCC_ARM-CUSTOM/mcuboot_giga_wifi.bin ./release/GIGA_WiFi/mcuboot_giga_wifi.bin
cp ./BUILD/GIGA/GCC_ARM-CUSTOM/mcuboot_giga_wifi_application.elf ./release/GIGA_WiFi/mcuboot_giga_wifi.elf
xxd -i ./release/GIGA_WiFi/mcuboot_giga_wifi.bin > ./release/GIGA_WiFi/mcuboot_giga_wifi.h
sed -i "s/unsigned char __release_GIGA_WiFi_mcuboot_giga_wifi_bin/const unsigned char mcuboot_giga_wifi_bin/" ./release/GIGA_WiFi/mcuboot_giga_wifi.h
sed -i "s/__release_GIGA_WiFi_mcuboot_giga_wifi_bin_len/mcuboot_giga_wifi_bin_len/" ./release/GIGA_WiFi/mcuboot_giga_wifi.h
tar -czvf ./release/GIGA_WiFi.tar.gz -C ./release/ GIGA_WiFi
fi
