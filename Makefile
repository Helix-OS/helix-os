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


all: check helix-kernel userspace image
dev-all: check ktools helix-kernel userspace image docs test

debug:
	@gdb -x tools/gdbscript

check:
	@if [ ! -e cross/.cross_check ]; then \
		echo "[W] Warning:";\
		echo "[W] Building the cross compiler using \`$(MAKE) cross-cc\` is recommended for best results.";\
		echo "[W] Things may break otherwise, proceed at your own risk.";\
	fi
	@./tools/check_depends.sh

helix-kernel:
	@cd kernel; $(MAKE) KNAME=$(KNAME) CONFIG_C_FLAGS="$(CONFIG_C_FLAGS)" \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)\
		  PLATFORM=$(PLATFORM)

ktools:
	cd tools; $(MAKE) ARCH=$(ARCH) PLATFORM=$(PLATFORM)

image:
	@echo -e "[\033[0;34mGenerating image...\033[0;0m]"
	@echo -e "[\033[0;32mMaking image\033[0;0m]"
	@cp -r boot-image build
	@cp kernel/helix_kernel-i586 build
	@tar c kernel/modobjs kernel/config > build/initrd.tar
	@./tools/mk_image.sh
	@echo "To boot: $(EMULATOR) $(EMU_FLAGS)"
	@echo -e "[\033[0;32mdone\033[0;0m]"
	@echo -e "[\033[0;34mdone\033[0;0m]";

userspace:
	@cd userland; $(MAKE) all KNAME=$(KNAME) \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)\
		  PLATFORM=$(PLATFORM)

test:
	$(EMULATOR) $(EMU_FLAGS)

cross-cc:
	@echo -e "[\033[0;34mMaking cross-compiler...\033[0;0m]"
	@#cd cross; $(MAKE) MAKE=$(MAKE) TARGET=$(TARGET) -j $$(( `cat /proc/cpuinfo | grep "^proc" | wc -l` * 2 ))
	@cd cross; $(MAKE) MAKE=$(MAKE) TARGET=$(TARGET) 
	@echo -e "[\033[0;34mdone\033[0;0m]"

docs:
	@echo -e "[\033[0;34mMaking documentation...\033[0;0m]"
	@cd doc; doxygen doxy.conf > /dev/null
	@echo -e "[\033[0;34mdone\033[0;0m]";

user-clean:
	@-cd userland; $(MAKE) clean KNAME=$(KNAME) \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)

kernel-clean:
	@-cd kernel; $(MAKE) clean


tools-clean:
	@-cd tools; $(MAKE) clean

clean: kernel-clean user-clean tools-clean
	@-cd tools; $(MAKE) clean
	@-rm *.img
	@-rm -rf build

.PHONY: all
