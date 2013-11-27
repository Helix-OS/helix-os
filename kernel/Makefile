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

all: kernel

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.o : %.s
	$(AS) $(ASFLAGS) $<

kernel: $(OBJS)
	@cd base; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE)
	@mv base/helix_kernel-$(ARCH) .

clean:
	-cd base; make clean

.PHONY: all
