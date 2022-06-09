SLABHIDDevice Library
---------------------

The SLABHidDevice Library allows programatic access and control 
to Silicon Labs USBXpress HID Bridge devices: CP2110 and CP2112.

Uncompressing the source package
--------------------------------

$ tar -xf slabhiddevice_1.0.tar.gz


Source Package Contents
-----------------------

- /common/
- /doc/ 
- /Makefile
- /slabhiddevice/

Dependencies
------------

1. libusb-1.0

On Ubuntu:

  $ sudo apt-get install libusb-1.0-0-dev

2. g++

On Ubuntu:

  $ sudo apt-get install g++


Building libslabhiddevice
-------------------------

Build the library:

$ make

Install it to the default location (/usr/local): 

$ sudo make install

This will install the headers at /usr/local/include and the libraries at /usr/local/lib.

Install at an alternative location such as /opt:

$ sudo make install INSTALLPATH=/opt

Configure Your System to Allow libusb to Access Your Device
-----------------------------------------------------------

CP210xManufacturing uses libusb-1.0 to communicate with the CP210x devices over
USB. In order for CP210xManufacturing to recognize your device, you must first
copy a udev file to /etc/udev/rules.d/. This udev file will automatically
enable read and write permissions for a device with the specified USB vendor
ID. Unplug and replug your device after copying a udev file for the permissions
to take effect. On some systems, it may be necessary to reboot the machine
before the new rules are applied.

CP2101 (VID: 0x10c4 PID: 0xea60):
- Copy SiliconLabs.rules to /etc/udev/rules.d/

Other:
- Modify an existing udev rules file for the required vendor ID

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", MODE="0666"
  
  - or - (optionally add a required product ID)

  SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  SUBSYSTEM=="usb_device", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0666"
  
- SiliconLabs.rules does not specify the product ID and will therefore allow
  read/write access to all Silicon Labs USB devices with the Silicon Labs VID (0x10c4).
  The product ID is optional and will further restrict which devices the rules file
  will affect.




