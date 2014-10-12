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
	@echo "[ata] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "[ata] AS $<"
	@$(AS) $(ASFLAGS) $<

ata_mod: $(OBJS)
	@$(LD) $(LDFLAGS) -o ../../modobjs/ata_mod.o *.o
	@echo "[ata] LD $(LDFLAGS) -o ../../modobjs/ata_mod.o *.o"

clean:
	@rm -f *.o

.PHONY: all
