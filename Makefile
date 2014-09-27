ARCH		= i586
TARGET		= $(ARCH)-elf
MAKE		= make
EMULATOR	= qemu-system-i386
#EMU_FLAGS	= -hda helix.img -hdb fattest.hdd -s -serial stdio -m 32
#EMU_FLAGS	= -hda helix.img -hdb userland/user.hdd -s -serial file:debug.log -m 32
EMU_FLAGS	= -hda helix.img -s -serial file:debug.log -m 32
#EMU_FLAGS	= -kernel kernel/helix_kernel-i586 -serial stdio -nographic -m 16 -s
CROSS		= $(shell pwd)/cross

KNAME		= obsidian-$(ARCH)

NATIVECC	= gcc
CC		= $(CROSS)/bin/$(TARGET)-gcc
LD		= $(CROSS)/bin/$(TARGET)-ld
AS		= nasm
OBJCOPY		= $(CROSS)/bin/$(TARGET)-objcopy
STRIP		= $(CROSS)/bin/$(TARGET)-strip
CONFIG_C_FLAGS	= -g

all: check helix-kernel ktools userspace image
dev-all: check helix-kernel ktools userspace image docs test

debug:
	@gdb -x tools/gdbscript

check:
	@if [ ! -e cross/.cross_check ]; then \
		echo "-----=[ Warning ]=-----";\
		echo "It is recommended that you build a cross compiler using"; \
		echo "    \"$(MAKE) cross-cc\""; \
		echo "if you haven't done so already, for best results."; \
		echo;\
		echo "Your native compiler may work, but if it refuses to boot or compile,";\
		echo "try the cross compiler before assuming it's a bug.";\
		echo;\
	fi
	@./tools/check_depends.sh

helix-kernel:
	@cd kernel; $(MAKE) KNAME=$(KNAME) CONFIG_C_FLAGS="$(CONFIG_C_FLAGS)" \
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)

ktools:
	cd tools; $(MAKE)

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
		  AS=$(AS) CC=$(CC) LD=$(LD) SPLIT=$(SPLIT) OBJCOPY=$(OBJCOPY) ARCH=$(ARCH)

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

clean: user-clean
	@-cd kernel; $(MAKE) clean
	@-cd tools; $(MAKE) clean
	@-rm *.img
	@-rm -rf build

.PHONY: all
