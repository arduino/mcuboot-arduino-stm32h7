# mbed-mcuboot-demo
Demo of mcuboot with Mbed

Initialize the mcuboot submodule:
`git submodule init && git submodule update`

Pull the most recent mcuboot mbed port:
`cd mcuboot && git checkout mbed-port && git pull`

Build the mbed program:
```
mbed target NRF52840_DK
mbed toolchain GCC_ARM
mbed compile
```

Flash bootloader application to the NRF52840_DK.

Then flash the included `signed-usb.hex` to the NRF52840_DK.

This enables drag-and-drop programming of the target over USB MSD.

Plug the NRF52840_DK's target USB connector into your laptop. It should show up as a 2MB flash drive.

Copy the `signed.bin` binary over to the flash drive. You can watch serial output for progress of flashing.

After a while the target should disconnect and start flashing itself... the on-board LED3 should start to blink when done.
