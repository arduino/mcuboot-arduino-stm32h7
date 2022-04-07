<img src="https://content.arduino.cc/website/Arduino_logo_teal.svg" height="100" align="right" />

`MCUboot Arduino`
=================
MCUboot bootloader porting for Arduino [Mbed OS](https://os.mbed.com/docs/mbed-os/latest/introduction/index.html) based boards.

The following boards are supported:
 * [Portenta H7](https://store.arduino.cc/products/portenta-h7)
 * [Portenta H7 Lite](https://store.arduino.cc/products/portenta-h7-lite)
 * [Portenta H7 Lite Connected](https://store.arduino.cc/products/portenta-h7-lite)
 * [Nicla Vision](https://store.arduino.cc/products/nicla-vision)

## :grey_question: What
MCUboot provides secure boot for 32-bit microcontrollers. For a detailed description on what MCUboot does and how it works please read the [official documentaion](https://docs.mcuboot.com/).

## :zap: Main features
### Signed and encrypted updates
MCUboot has support for encrypting/decrypting images on-the-fly while upgrading; furthermore, before booting a Sketch, it will check if the computed signature is matching the one embedded in the image.
 
### Confirm or revert updates
After an update the new Sketch can update the content of the flash at runtime to mark itself as `OK`. MCUboot will then still choose to run it during the next boot. When this happens, the swap is made `permanent`. If this doesn’t happen, MCUboot will perform a `revert` swap during the next boot by swapping the image back into its original location, and attempting to boot the old Sketch.

### Sketch bootstrap
If no valid image is found in the primary slot MCUboot will search a valid image in the secondary slot and if any it will load it inside the primary slot.

### Reset recovery
If a reset occurs in the middle of a swap operation, the two images may be discontiguous in flash. MCUboot recovers from this condition by using the image trailers to determine how the image parts are distributed in flash and restarting the swap.

### Backward compatible with default Arduino booloader
If signing and encryption keys are not stored in flash alongside MCUboot, the Sketch signature verification is skipped and any valid Sketch can be booted.

## :gear: How
### Switch to MCUboot
* Run this [Sketch](https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/STM32H747_System/examples/STM32H747_updateBootloader/STM32H747_updateBootloader.ino) to upload the latest released binary into your board
* Flash the bootloader binary file with your preferred debugger @ flash address `0x08000000`

### Enable signature and encryption
By default signature verification and encryption support are disabled. To enable them you have to write your signature and encryption keys inside your board.
In this project MCUboot is configured to support `ecdsa-p256` keys for both signature and encryption.

To write the default keys in flash you can use this [Sketch](https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/STM32H747_System/examples/STM32H747_updateBootloader/STM32H747_updateBootloader.ino)

:warning: WARNING :warning: The default keys are public therefore is not safe to use them for production, they are included only for evaluation purpose.

### Customize signing and encryption keys
You can use your preferred tool the generate your `ecdsa-p256` keys. With imgtool:
```
imgtool keygen -k ecsdsa-p256-signing-key.pem -t ecdsa-p256
imgtool keygen -k ecsdsa-p256-encrypt-key.pem -t ecdsa-p256
```
The public signing key and the private encryption key have to be written in flash at this addresses:
```
signing key @ 0x8000300
encrypt key @ 0x8000400
```
To get this data from the generated pem files with imgtool:
```
imgtool getpub -k ecsdsa-p256-signing-key.pem 
imgtool getpriv -k ecsdsa-p256-encrypt-key.pem
```
Copy and paste the key data in this [Sketch](https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/STM32H747_System/examples/STM32H747_updateBootloader/STM32H747_updateBootloader.ino) and run it to flash the keys alongside the bootloader.

### Create a signed and encrypted update Sketch
To create a signed and encrypted Sketch an additional step is needed after the Sketch binary is generated. This additional step is done passing the binary through `imgtool`. The flags used by the board to create a secure Sketch are defined [here](https://github.com/arduino/ArduinoCore-mbed/blob/fa628e35011a92fb7e54fa6bfd9a69be33173bf8/boards.txt#L79-L86). The resulting command resembles as follows:
```
imgtool sign --key ecdsa-p256-signing-key.pem --encrypt ecdsa-p256-encrypt-key.pem input.bin output.bin --align 32 --max-align 32 --version 1.2.3+4 --header-size 0x20000 --pad-header --slot-size 0x1E0000
```

### Load an update sketch
The bootloader exposes a DFU interface that can be used to upload a sketch in the QSPI flash of the board as a file. The upload process ends setting the pending flag to the update binary file and resetting the board. After reset MCUboot takes care of applying the update.
```
dfu-util --device 0x2341:0x035b -D update.ino.bin -a2 --dfuse-address=0xA0000000:leave
```

### Confirm an update sketch
MCUboot expects that every update have to be confirmed otherwise it will revert to the previous running sketch as soon as the board is resetted. To confirm a sketch you have to call `MCUboot::confirmSketch()` in your `setup()`.
```
void setup() {
  ...
  
  if(applicationSelfCheck() == OK) {
    MCUboot::confirmSketch()
  }
  
  ...
}
```

### Flash is allocated
The diagram below shows the default memory map configuration used for this project:

```
INTERNAL FLASH                                             QSPI FLASH

Slot 0                                                     Scratch
0x08020000 - 0x081FFFFF                                    MBRBlockDevice partition 2 scratch.bin
  0x08020000 header
  0x08040000 Sketch                                        Slot 1
  0x081E0000 trailer                                       MBRBlockDevice partition 2 update.bin

Bootloader                
0x08000000 - 0x0801FFF    
  0x080002F0 bootloader id
  0x08000300 signing key  
  0x08000400 encrypt key  
  0x0801F000 board data
```

### Build MCUboot from source
The following command will setup the mbed environment and clone the needed repositories before compile for Portenta H7.

```
mbed config root . && mbed deploy
mbed compile -m PORTENTA_H7_M7 -t GCC_ARM --profile=release --profile mbed-os/tools/profiles/extensions/lto.json
```
Additional flags are needed for [Lite](generate_rel.sh#L24), [Lite Connected](generate_rel.sh#L35) and [Nicla Vision](generate_rel.sh#L46) boards.

### Debug

1.  LED
 - MCUboot operations: slot verify, copy, erase or swap the board LED will blink in violet (red+blue).
 - MCUboot idle: The board green LED will fade-in fade-out

2. Serial
 - MCUboot debug prints are disabled by default. They can be enabled putting `BT_SEL` (`PI8`) pin `HIGH` or calling `MCUboot::bootDebug(1);` in your Sketch.

## :mag_right: Other resources

* [MCUboot repository](https://github.com/mcu-tools/mcuboot)
* [MCUboot demo for nRF52840](https://github.com/AGlass0fMilk/mbed-mcuboot-demo)
* [MCUboot documentation](https://docs.mcuboot.com/)
