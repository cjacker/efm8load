DESTDIR=
PREFIX=/usr/local

CPU_ARCH?=$(shell arch)

all:
	make -C slabhiddevice
	make -C slabhidtosmbus
	rm -f ./libslabhiddevice.so.1.0
	rm -f ./libslabhidtosmbus.so.1.0
	cp slabhiddevice/build/lib/$(CPU_ARCH)/libslabhiddevice.so.1.0 .
	cp slabhidtosmbus/build/lib/$(CPU_ARCH)/libslabhidtosmbus.so.1.0 . 

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/efm8load
	install -m0755 libslabhiddevice.so.1.0 $(DESTDIR)$(PREFIX)/efm8load/
	install -m0755 libslabhidtosmbus.so.1.0 $(DESTDIR)$(PREFIX)/efm8load/
	install -m0644 *.py $(DESTDIR)$(PREFIX)/efm8load/
	sed -i 's@./libslabhiddevice.so.1.0@$(PREFIX)/efm8load/libslabhiddevice.so.1.0@g' $(DESTDIR)$(PREFIX)/efm8load/hidport.py
	sed -i 's@./libslabhiddevice.so.1.0@$(PREFIX)/efm8load/libslabhiddevice.so.1.0@g' $(DESTDIR)$(PREFIX)/efm8load/smbport.py
	sed -i 's@./libslabhidtosmbus.so.1.0@$(PREFIX)/efm8load/libslabhidtosmbus.so.1.0@g' $(DESTDIR)$(PREFIX)/efm8load/smbport.py
	install -m0644 intelhex-LICENSE.txt pyserial-LICENSE.txt $(DESTDIR)$(PREFIX)/efm8load/ 	
	install -m0755 scripts/hex2boot $(DESTDIR)$(PREFIX)/bin
	install -m0755 scripts/efm8load $(DESTDIR)$(PREFIX)/bin
	sed -i 's@PREFIX@$(PREFIX)@g' $(DESTDIR)$(PREFIX)/bin/efm8load
	sed -i 's@PREFIX@$(PREFIX)@g' $(DESTDIR)$(PREFIX)/bin/hex2boot

clean:
	rm -f ./libslabhiddevice.so.1.0
	rm -f ./libslabhidtosmbus.so.1.0
	rm -rf ./slabhiddevice/build
	rm -rf ./slabhidtosmbus/build
