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

## Binary Signing

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
