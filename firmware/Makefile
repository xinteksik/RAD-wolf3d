# RAD-Wolf firmware link, derived from the tested RAD/Circle build.
REPO ?= $(abspath ../..)
STDLIB_ROOT ?= $(REPO)/upstream/circle-stdlib
RAD := $(REPO)/upstream/RAD-Doom/Source
RW := $(REPO)/src
CORE := $(REPO)/build/wolf-core-arm64
CIRCLEHOME := $(STDLIB_ROOT)/libs/circle
NEWLIBDIR := $(STDLIB_ROOT)/install/aarch64-none-circle
include $(STDLIB_ROOT)/Config.mk
AARCH := 64
RASPPI := 3
PREFIX64 := aarch64-none-elf-
TARGET := kernel8
RAD_NAMES := config rad_reu rad_doom_hijack lowlevel_arm64 gpio_defs helpers lowlevel_dma mempool
RAD_OBJS := $(addsuffix .o,$(RAD_NAMES))
WOLF_OBJS := $(addprefix wolf/,$(notdir $(wildcard $(CORE)/*.o)))
OBJS := radwolf_kernel.o radwolf_rad_video.o $(RAD_OBJS) $(WOLF_OBJS)
INCLUDE += -I$(NEWLIBDIR)/include -I$(STDLIB_ROOT)/include -I$(RW) -I$(RW)/baremetal -I$(RAD) -I$(RAD)/../include -I$(REPO)/upstream/Wolf4SDL
LIBS := $(NEWLIBDIR)/lib/libm.a $(NEWLIBDIR)/lib/libc.a $(NEWLIBDIR)/lib/libcirclenewlib.a \
 $(CIRCLEHOME)/addon/SDCard/libsdcard.a $(CIRCLEHOME)/lib/usb/libusb.a \
 $(CIRCLEHOME)/lib/input/libinput.a $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
 $(CIRCLEHOME)/lib/fs/libfs.a $(CIRCLEHOME)/lib/net/libnet.a \
 $(CIRCLEHOME)/lib/sched/libsched.a $(CIRCLEHOME)/lib/libcircle.a
include $(CIRCLEHOME)/Rules.mk
CFLAGS += -Ofast -g0 -Wno-unused-variable -fexceptions -DRADWOLF_BAREMETAL
CPPFLAGS += $(CFLAGS) -include $(CIRCLEHOME)/include/assert.h
$(shell mkdir -p wolf)
radwolf_kernel.o: $(RW)/radwolf_kernel.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
radwolf_rad_video.o: $(RW)/radwolf_rad_video.c $(RW)/radwolf_gray.h $(RW)/radwolf_flat.h
	$(CC) $(CFLAGS) -std=gnu99 -c -o $@ $<
$(RAD_OBJS): %.o: $(RAD)/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
wolf/%.o: $(CORE)/%.o
	cp $< $@
all: $(TARGET).img
-include $(DEPS)
