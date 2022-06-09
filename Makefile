CPU_ARCH?=$(shell arch)

all:
	make -C slabhiddevice
	make -C slabhidtosmbus
	rm -f ./libslabhiddevice.so.1.0
	rm -f ./libslabhidtosmbus.so.1.0
	cp slabhiddevice/build/lib/$(CPU_ARCH)/libslabhiddevice.so.1.0 .
	cp slabhidtosmbus/build/lib/$(CPU_ARCH)/libslabhidtosmbus.so.1.0 . 

clean:
	rm -f ./libslabhiddevice.so.1.0
	rm -f ./libslabhidtosmbus.so.1.0
	rm -rf ./slabhiddevice/build
	rm -rf ./slabhidtosmbus/build
