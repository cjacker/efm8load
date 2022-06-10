# efm8load.py and hex2boot.py - official EFM8 bootloader conversion and download utilities with all source codes collected

EFM8 Factory Bootloader package [AN945SW](https://www.silabs.com/documents/public/example-code/AN945SW.zip) contains all EFM8 bootloader images with UART, USB or SMBus interface. And various utilities such as:

- hex2boot : Hex to Hex to EFM8 Bootload Record conversion utility
- efm8load : EFM8 bootloader download utility

However, if run the `efm8load.py` on Linux directly, it will throw some errors because hidport.py or smbport.py need to load 'libslabhiddevice.so.1.0' and  'libslabhiddevice.so.1.0' not provided in same package. And this libs and source codes provided in [USBXpressHostSDK for Linux](https://www.silabs.com/documents/public/software/USBXpressHostSDK-Linux.tar).

This repo collected all related source codes together and provide a Makefile to build them.

# LICENSE
Official source package from Silicon Labs did not provide a LICENSE file. since the sources contains 'hidapi.h' and 'hid-libusb.c' from [hidapi](https://github.com/signal11/hidapi), that means the source codes opened by Silicon Labs should follow same license of hidapi project.

hex2boot.py follows intelhex license (BSD license):
![screenshot-2022-06-10-09-13-34](https://user-images.githubusercontent.com/1625340/172971337-7fccc873-dfe8-429a-9a7d-99b3002631a8.png)


efm8load.py follows pyserial license (BSD license):
![screenshot-2022-06-10-09-13-51](https://user-images.githubusercontent.com/1625340/172971351-11702137-a6a8-4275-82c2-4e5708d49d3f.png)

# Usage:

```
$ git clone https://github.com/cjacker/efm8load.git
$ cd efm8load
$ make
$ export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
$ python hex2boot.py filename.hex -o filename.efm8
$ sudo python efm8load.py -p /dev/ttyUSB0 filename.efm8
```

Change the `/dev/ttyUSB0` to match you device.


