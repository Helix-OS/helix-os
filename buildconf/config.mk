# The default build config
KNAME		= helix_kernel-$(ARCH)

ARCH            = i586
PLATFORM        = pc
CROSS           = $(shell pwd)/cross

MAKE		= make
MAKEFLAGS       = --no-print-directory
MKROOT          = $(shell pwd)
MKCONFIG        = config.out.mk

include buildconf/kernel/default.mk
include buildconf/arch/$(ARCH)/$(PLATFORM).mk

CC		= $(CROSS)/bin/$(TARGET)-gcc
LD		= $(CROSS)/bin/$(TARGET)-ld
AS		= nasm
OBJCOPY		= $(CROSS)/bin/$(TARGET)-objcopy
STRIP		= $(CROSS)/bin/$(TARGET)-strip
NATIVECC	= gcc
CONFIG_C_FLAGS	= -g

# Global config generation
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
