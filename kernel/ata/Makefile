CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= $(shell pwd)/../include
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -fPIC -shared

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

all: ata_mod

%.o : %.c
	@$(CC) $(CFLAGS) -c $<
	@echo CC $<

%.o : %.s
	@$(AS) $(ASFLAGS) $<
	@echo AS $<

ata_mod: $(OBJS)
	@$(LD) $(LDFLAGS) -o ../modules/ata_mod.o *.o
	@echo LD $(LDFLAGS) -o ../modules/ata_mod.o *.o

clean:
	rm *.o

.PHONY: all
