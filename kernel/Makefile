CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= $(shell pwd)/include
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -T linker.ld

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

all: kernel kmodules

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.o : %.s
	$(AS) $(ASFLAGS) $<

kernel: $(OBJS)
	@cd base; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@mv base/helix_kernel-$(ARCH) .

kmodules:
	@mkdir -p modules
	@cd dummy; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@cd pci; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@cd ata; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@cd vga; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@cd vfs; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
clean:
	-cd base; make clean
	-cd pci; make clean
	-cd ata; make clean
	-cd vga; make clean
	-cd vfs; make clean
	-cd dummy; make clean
	-rm modules/*.o

.PHONY: all
