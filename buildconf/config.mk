# The default build config
KNAME		= helix_kernel-$(ARCH)
ARCH            = i586
PLATFORM        = pc

CROSS           = $(shell pwd)/cross
MAKE		= make
MAKEFLAGS       = --no-print-directory
MKROOT          = $(shell pwd)

include buildconf/kernel/default.mk
include buildconf/arch/$(ARCH)/$(PLATFORM).mk
include buildconf/userland/default.mk

CC		= $(CROSS)/bin/$(TARGET)-gcc
C++		= $(CROSS)/bin/$(TARGET)-g++
LD		= $(CROSS)/bin/$(TARGET)-ld
AS		= nasm
OBJCOPY		= $(CROSS)/bin/$(TARGET)-objcopy
STRIP		= $(CROSS)/bin/$(TARGET)-strip
NATIVECC	= gcc
CONFIG_C_FLAGS	= -g
