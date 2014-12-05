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

all: dummy_mod

%.o : %.c
	@echo "      [fatfs] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "      [fatfs] AS $<"
	@$(AS) $(ASFLAGS) $<

dummy_mod: $(OBJS)
	@echo "      [fatfs] LD $(LDFLAGS) -o ../../modobjs/fatfs_mod.o *.o"
	@$(LD) $(LDFLAGS) -o ../../modobjs/fatfs_mod.o *.o

clean:
	@rm -f *.o

.PHONY: all
