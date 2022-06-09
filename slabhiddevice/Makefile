###########################################
# Makefile for libslabhiddevice
# using libusb-1.0
#
# Silicon Labs
# Modified July 12th, 2016
###########################################

# This file gets put into the source package and is used to build the actual library.

# it will produce .so and .a

CPU_ARCH?=$(shell arch)
BUILD?=./build
BUILDPATH?=$(BUILD)/lib/$(CPU_ARCH)
OBJPATH?=$(BUILD)/obj/$(CPU_ARCH)

INSTALLPATH?=/usr/local/

# required packages
PKG_CONFIG_DEPENDENCIES = libusb-1.0

# Library Variables

LIB_BASE_NAME = slabhiddevice
LIB_NAME = lib$(LIB_BASE_NAME)
LIB_VER = 1.0
LIB_SOVER = 1

SONAME=$(LIB_NAME).so.$(LIB_SOVER)
LIBFULLNAME=$(LIB_NAME).so.$(LIB_VER)

LIBFULLPATH=$(BUILDPATH)/$(LIBFULLNAME)
ARCHIVE_FULL_PATH=$(BUILDPATH)/$(LIB_NAME).a


PUBLICHEADERSPATH=./$(LIB_BASE_NAME)/include

# sources

CSRCS = hid-libusb.c
CPPSRCS =HIDDevice.cpp
CPPSRCS+=SLABHIDDevice.cpp
CPPSRCS+=ReportQueue.cpp
CPPSRCS+=OsDep.cpp

COBJS=$(CSRCS:%.c=%.o)
CPPOBJS=$(CPPSRCS:%.cpp=%.o)

OBJFILES = $(CPPOBJS) $(COBJS)

OBJFULLPATHS=$(addprefix $(OBJPATH)/, $(OBJFILES))


INCLUDES ?= -I$(PUBLICHEADERSPATH) -I./common/include/ -I./common/linux/

INCLUDES += $(foreach depcflgs,$(PKG_CONFIG_DEPENDENCIES), `pkg-config $(depcflgs) --cflags`)

LIBS      += $(foreach deplib,$(PKG_CONFIG_DEPENDENCIES), `pkg-config --libs $(deplib)`)

VPATH+=$(LIB_BASE_NAME)/src common/linux common/src

# targets: build clean install

CC       ?= gcc
CFLAGS   ?= -Wall -fPIC -g $(INCLUDES)

CXX      ?= g++
CXXFLAGS ?= -Wall -fPIC -g $(INCLUDES)

AR       ?= ar
ARFLAGS  ?= rcs

all: $(LIBFULLPATH) $(ARCHIVE_FULL_PATH)

$(OBJPATH)/%.o: %.c 
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(OBJPATH)/%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

$(OBJPATH):
	mkdir -p $@

$(BUILDPATH):
	mkdir -p $@
	
$(LIBFULLPATH): $(OBJPATH) $(BUILDPATH) $(OBJFULLPATHS)
	$(CXX) $(ARCHFLAG) -shared -Wl,-soname,$(SONAME) -o $@ $(OBJFULLPATHS) $(LIBS)
	cd $(BUILDPATH) && \
	ln -sf $(LIBFULLNAME) $(SONAME) && \
	ln -sf $(SONAME) $(LIB_NAME).so
	

$(ARCHIVE_FULL_PATH): $(OBJFULLPATHS)
	$(AR) $(ARFLAGS) $(BUILDPATH)/$(LIB_NAME).a $(OBJFULLPATHS)

debug:
	echo $(OBJFULLPATHS)
	
clean:
	rm -rf $(BUILDPATH)
	rm -rf $(OBJPATH)
	rm -rf $(BUILD)

install:
	mkdir -p $(INSTALLPATH)/include
	mkdir -p $(INSTALLPATH)/lib
	cp $(PUBLICHEADERSPATH)/*.h $(INSTALLPATH)/include
	cp $(LIBFULLPATH) $(INSTALLPATH)/lib/
	cd $(INSTALLPATH)/lib/ && \
	ln -sf $(LIBFULLNAME) $(SONAME) && \
	ln -sf $(SONAME) $(LIB_NAME).so
	
	@echo "Installed at: "$(INSTALLPATH)
	
.PHONY: all clean install debug

