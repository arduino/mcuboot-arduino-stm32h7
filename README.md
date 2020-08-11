# mbed-mcuboot-demo
Demo of mcuboot with Mbed-OS

This application builds as a bootloader and should be built with the [corresponding bootable blinky demo application](https://github.com/AGlass0fMilk/mbed-mcuboot-blinky).

## Overview

Go to the mcuboot repository for more information on mcuboot and how to configure it. This section will only cover topics specific to the mbed-os implementation.

There is additonal information, such as an overview of memory regions and basic bootloader operation, in the mbed-mcuboot-blink repository linked to above.

## Configuration

Upon bootup, mcuboot looks at two memory regions, called "primary" and "secondary", to determine if a firmware update is available and should be installed.

### Primary Memory Region

The **"primary"** memory region is always the region of flash beginning where the bootable main application starts (the "application start address") and ending at the end of the physical flash address space **minus the mcuboot scratch memory area size**.

### Scratch Region

If configured as such, mcuboot can perform a "swap" update where it will copy the existing main application into the secondary memory region and the update candidate into the main application region. This allows mcuboot to revert the update in case of a serious firmware bug (ie: brick-proofs the update process). If the updated application fails to mark itself as "okay", mcuboot will revert the update upon the next boot cycle. 

To perform this kind of swap update, mcuboot requires a non-volatile "scratch" space in memory to store pieces of application code and update status information. This enables mcuboot to safely continue an update/revert procedure in the event of a power loss.

The scratch region exists **at the end of the physical flash address space, just after the main application/primary memory region.** The size of the scratch space can be configured using `mcuboot.scratch-size` -- this value **must** be erase-sector aligned (ie: a multiple of the flash's eraseable size).

Care should be exercised when configuring the bootable application to ensure the scratch space and application regions do not overlap. See the bootable blinky demo application readme for more information.

### Secondary Memory Region

The **"secondary"** memory region is provided by you. Typically this is an external flash chip used for bulk data and firmware update storage.

By default, the secondary memory region is provided by the `mbed::BlockDevice::get_default_instance()` API. This functionality can be overridden by redefining the weak function declared in `secondary_bd.h`:

```
mbed::BlockDevice* get_secondary_bd(void);
```

See the example file in this repository, `default_bd.cpp`.

Since the Mbed-OS mcuboot port uses Mbed's `BlockDevice` API, there is a lot of flexibility when providing the secondary memory region. For example, you can use a `FlashIAPBlockDevice` if your application is small enough to fit two copies in the internal flash. If you also use your external flash chip for data storage you can simply wrap the whole `BlockDevice` object in a `SlicingBlockDevice` to give mcuboot access to a certain region of your external flash.

The Mbed-OS mcuboot port also has an additional function that the application can define if needed: `void mbed_mcuboot_user_init(void)`. This is provided as a `weak` symbol so the application can perform any necessary initialization before the rest of the boot process begins.

## Binary Signing and Running the Demo

It is beneficial to first install mcuboot's imgtool python command line tool:

`pip3 install --user -r mcuboot/scripts/requirements.txt && python3 mcuboot/scripts/setup.py install`

This section will only cover steps specific to setting up this project with a signing key pair and signing a main application binary. For more advanced use cases and information, see the [mcuboot documentation](https://github.com/AGlass0fMilk/mcuboot/blob/mbed-port/docs/imgtool.md).

For this project the required steps to sign an application are:

1.) Generate an rsa2048 key pair: `imgtool keygen -k signing-keys.pem -t rsa-2048`

2.) Extract the public key into a C data structure so it can be built into the bootloader: 

`imgtool getpub -k signing-keys.pem >> signing_keys.c`

**Note:** The output of this command formats the variables to a specific name that is declared as an extern in mcuboot's sources. It must **not** be modified manually.

3.) Once all other configuration has occured and the secondary flash BlockDevice has been declared, you may build the bootloader: `mbed compile`

4.) Build the main application. The corresponding main application example can be cloned from this repository: [mbed-mcuboot-blinky](https://github.com/AGlass0fMilk/mbed-mcuboot-blinky). Follow the instructions in the README there and continue to the next step when the application hex/binary has been built.

Briefly, the commands are (from the root directory of this repository):

```
git clone https://github.com/AGlass0fMilk/mbed-mcuboot-blinky.git ../mbed-mcuboot-blinky
cd ../mbed-mcuboot-blinky
mbed new . && mbed deploy
mbed compile -t GCC_ARM -m NRF52840_DK
cp BUILD/NRF52840_DK/GCC_ARM/mbed-mcuboot-blinky.hex ../mbed-mcuboot-demo
cd &_
```

5.) The next step is to sign the main application binary. 

**Note:** even if the internal main application is not verified (ie: the digital signature is not checked) this step **must** be performed so the appropriate application header info is prepended to the binary. mcuboot will not execute the internal main application if this header info is missing or corrupt.

```
imgtool sign -k signing-keys.pem --align 4 -v 1.2.3 --header-size 1024 --pad-header -S 901120 mbed-mcuboot-blinky.hex signed.hex
```

Explanation of each option:

- `-k signing-keys.pem`: this specifies the file containing the keys used to sign/verify the application
- `--align 4`: this lets mcuboot know the intrinsic alignment of the flash (32-bits = 4 byte alignemtn)
- `-v 1.2.3`: this sets the version number of the application to 1.2.3
- `--header-size 1024`: this must be the same as the value specified in `mcuboot.header-size` configuration (1024 bytes by default)
- `--pad-header`: this tells imgtool to insert the entire header, including any necessary padding bytes.
- `-S 901120`: this specifies the maximum size of the application ("slot size"). It **must** be the same as the value specified in the **main application's** `target.mbed_app_size` configuration (0xDC000 = 901120)

6.) Flash your target with both the mcuboot bootloader application and the signed main application

## Serial Bootloader Demo Application

This demo optionally includes a serial bootloader demo to reprogram the target using a simple serial protocol.

**Please note:** This serial bootloader is rather slow and has not been optimized for speed at this point. This speed issue is mainly due to Mbed's existing serial implementation that processes UART data byte-by-byte. The python serial loader script has a 1ms (in reality it's probably longer) delay between sending **each byte** of the update binary to ensure no data is dropped due to delays in Mbed's serial stack. A possible optimization in the future is to implement a protocol with a known/negotiated packet size and use the asynchronous serial API in Mbed to collect UART packets of that size in a more efficient and faster way.

**Also note:** This bootloader was developed on and tested using the `nRF52840_DK` and `EP_AGORA` targets

### Enabling the Serial Bootloader

To enable the serial bootloader, modify the `mbed_app.json` configuration as shown below:

```
    "config": {
        "serial-bootloader-build": {
            "help": "Build bootloader with serial update support",
            "value": 1
        }
    }
```

When this configuration option is set to 1, the serial bootloader code will be compiled and run during the `mbed_mcuboot_user_init` hook.

### Operation Overview

Upon boot, the bootloader checks the target's `BTN1` pin to see if it is pulled low (button pressed). If it is, this starts a 5 second timeout where the bootloader waits for a 0-byte over the STDIO serial port. If no 0-byte is received within 5 seconds, the bootloader continues as usual and must be reset with `BTN1` pressed to enter serial bootloader mode again.

If the bootloader receives a 0-byte within the timeout period, it will write a `1` to the serial port to tell the loader script that it's there. The bootloader then prepares the update flash partition by erasing it. Once the erase operation is complete, the bootloader signals to the serial loader script that it is ready to receive data. The serial loader script reads the update binary in chunks and frames each binary chunk with a metadata header of the following format:

```
    typedef struct {
        uint16_t crc16;             /** CRC16 of entire packet */
        uint16_t sequence_num;      /** Sequence number of the OTA update, the special value 0xFFFF means the transfer is over. Wraps at 0xFFFE */
        uint16_t payload_size;      /** Payload size of this packet in bytes */
    } packet_header_t;
```

The header includes a CRC16 of the entire packet (including the other parts of the header), a 16-bit sequence number for synchronization, and a 16-bit payload size indicating how many payload bytes follow the header.

This framed packet is then encoded using COBS-R for transport over the serial link. This allows each packet to be delimited by a 0-byte.

When the bootloader receives a 0-byte, it decodes the collected buffer, checks the CRC16 and then programs the binary payload to the update partition in flash.

At the end of the binary, the serial loader sends a special packet with a sequence number of `0xFFFF` to indicate the transfer is complete. The bootloader calls the mcuboot API function: `boot_set_pending`. This sets flags to tell mcuboot the update partition has an update candidate available.

The bootloader then continues on to mcuboot subroutines. mcuboot will then see the update binary in the update partition, check its validity (through digital signature verification), and load it as appropriate.

The final step of the update process is for the updated application to mark itself as "OK". This is accomplished by calling the mcuboot API function: `boot_set_confirmed`. This prevents the update from being reverted on the next system reset.


### Running the Serial Bootloader Demo

To run the serial bootloader demo, you must have first completed the "Binary Signing and Running the Demo" section above and have mcuboot and the blinky demo application running on your target.

To build the update version of the blinky application, you can compile the blinky demo application with the following command:

```
mbed comile -DMCUBOOT_UPDATE
```

This adds a preprocessor definition, `MCUBOOT_UPDATE`, which changes the blinked LED from `LED1` to `LED2` and increases the blinking rate from 1 second to 250 milliseconds.

Once the build is complete, you must sign the update and convert the hex file to a binary. Use the following command below:

```
imgtool sign -k ../mbed-mcuboot-demo/signing-keys.pem --align 4 -v 1.2.4 --header-size 4096 --pad-header -S 61440 -M 256 --pad BUILD/NRF52840_DK/GCC_ARM/mbed-mcuboot-blinky.hex signed-update.hex
```

In this case, the `-S` (slot size) option should be just enough to fit the application, header (4kB size in this case) and some metadata. The slot size must be a multiple of your target's erase sector size (4kB in this case) so round up to the nearest multiple.

The `-v` option bumps the version in the mcuboot header to `1.2.4`.

To convert the signed hex file to a binary that is compatible with the serial loader script, use the following command (only if you're using the GCC ARM toolchain):

```
arm-none-eabi-objcopy -I ihex -O binary signed-update.hex signed-update.bin
```

Then, to upload the signed update binary, perform the following steps:

1. Press and hold the target's BTN1 while resetting it
2. Within 5 seconds, execute the following: `python serial_loader.py signed-update.bin /dev/ttyACM0`. Make sure to change `/dev/ttyACM0` to the serial port your target is on.
3. Wait for the update operation to be completed.

Once complete, the application should now be the updated version and should be blinking LED2 at a much faster rate than before.
