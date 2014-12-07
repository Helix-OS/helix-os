ARCH		= i586
TARGET		= $(ARCH)-elf
MAKE		= make
MAKEFLAGS       = --no-print-directory
EMULATOR	= qemu-system-i386
EMU_FLAGS	= -hda helix.img -s -serial tcp::9090,server -m 32
CROSS		= $(shell pwd)/cross
PLATFORM        = pc

KNAME		= obsidian-$(ARCH)

NATIVECC	= gcc
CC		= $(CROSS)/bin/$(TARGET)-gcc
LD		= $(CROSS)/bin/$(TARGET)-ld
AS		= nasm
OBJCOPY		= $(CROSS)/bin/$(TARGET)-objcopy
STRIP		= $(CROSS)/bin/$(TARGET)-strip
CONFIG_C_FLAGS	= -g
MKCONFIG        = config.mk
MKROOT          = $(shell pwd)

CURRENT_VER     = 0.1.0

.PHONY: all
all:     check $(MKCONFIG) helix-kernel userspace image
dev-all: check $(MKCONFIG) ktools helix-kernel userspace image docs test

helix-kernel: $(MKCONFIG)
	@cd kernel; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT)

ktools:
	@cd tools; $(MAKE) ARCH=$(ARCH) PLATFORM=$(PLATFORM)

userspace: $(MKCONFIG)
	@#cd userland; $(MAKE) all KNAME=$(KNAME) \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)\
		  PLATFORM=$(PLATFORM)
	@cd userland; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT)

image:
	@echo -e "[ ] Generating image..."
	@cp -r boot-image build
	@cp kernel/helix_kernel-i586 build
	@tar c kernel/modobjs kernel/config > build/initrd.tar
	@./tools/mk_image.sh
	@echo "To boot: $(EMULATOR) $(EMU_FLAGS)"
	@echo -e "[ ] done"

cross-cc: check
	@echo -e "[ ] Making cross-compiler..."
	@cd cross; $(MAKE) MAKE=$(MAKE) TARGET=$(TARGET)
	@echo -e "[ ] done"

debug:
	@gdb -x tools/gdbscript

test:
	$(EMULATOR) $(EMU_FLAGS)

docs:
	@echo -e "[ ] Making documentation..."
	@cd doc; doxygen doxy.conf > /dev/null
	@echo -e "[ ] done";

check:
	@if [ ! -e cross/.cross_check ]; then \
		echo "[W] Warning:";\
		echo "[W] Building the cross compiler using \`$(MAKE) cross-cc\` is recommended for best results.";\
		echo "[W] Things may break otherwise, proceed at your own risk.";\
	fi
	@./tools/check_depends.sh

user-clean:
	@-cd userland; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT) clean
	@#-cd userland; $(MAKE) clean KNAME=$(KNAME) \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)

kernel-clean:
	@-cd kernel; $(MAKE) MKCONFIG="$(MKCONFIG)" MKROOT="$(MKROOT)" clean

tools-clean:
	@-cd tools; $(MAKE) clean

clean: kernel-clean user-clean tools-clean
	@-cd tools; $(MAKE) clean
	@-rm *.img
	@-rm -rf build
	@-rm config.mk

.PHONY: $(MKCONFIG)
$(MKCONFIG):
	@echo "# Global make variable configuration" > $(MKCONFIG)
	@echo ARCH           = $(ARCH) >> $(MKCONFIG)
	@echo TARGET         = $(TARGET) >> $(MKCONFIG)
	@echo MAKE           = $(MAKE) >> $(MKCONFIG)
	@echo MAKEFLAGS      = $(MAKEFLAGS) >> $(MKCONFIG)
	@echo PLATFORM       = $(PLATFORM) >> $(MKCONFIG)
	@echo LD             = $(LD) >> $(MKCONFIG)
	@echo AS             = $(AS) >> $(MKCONFIG)
	@echo CC             = $(CC) >> $(MKCONFIG)
	@echo NATIVECC       = $(NATIVECC) >> $(MKCONFIG)
	@echo OBJCOPY        = $(OBJCOPY) >> $(MKCONFIG)
	@echo STRIP          = $(STRIP) >> $(MKCONFIG)
	@echo CONFIG_C_FLAGS = $(CONFIG_C_FLAGS) >> $(MKCONFIG)
	@echo MKCONFIG       = $(MKCONFIG) >> $(MKCONFIG)
	@echo MKROOT         = $(MKROOT) >> $(MKCONFIG)
	@echo CURRENT_VER    = $(CURRENT_VER) >> $(MKCONFIG)
