# efmload - official EFM8 bootloader download utility with all source codes collected

EFM8 Factory Bootloader package [AN945SW](https://www.silabs.com/documents/public/example-code/AN945SW.zip) contains all EFM8 bootloader images with UART, USB or SMBus interface. The source of the efm8load tool, a python script `efm8load.py` is also included under /Tools/Source folder. Since Python is a cross-platform language, and therefore the script can run on a Linux system.

However, if run the `efm8load.py` on Linux directly, it will throw some errors because hidport.py or smbport.py need to load 'libslabhiddevice.so.1.0' and  'libslabhiddevice.so.1.0' not provided in same package. And this libs and source codes provided in [USBXpressHostSDK for Linux](https://www.silabs.com/documents/public/software/USBXpressHostSDK-Linux.tar).

This repo collected all related source codes together and provide a Makefile to build them.


# Usage:

```
$ git clone https://github.com/cjacker/efmload.git
$ git submodule update --init
$ cd efmload
$ make
$ export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
$ ./hex2boot/hex2boot filename.hex -o filename.efm8
$ sudo python efm8load.py -p /dev/ttyUSB0 filename.efm8
```

Change the `/dev/ttyUSB0` to match you device.


