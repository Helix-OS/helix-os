CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= $(shell pwd)/../include
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
#LDFLAGS		= -r 
LDFLAGS		= -fPIC -shared

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

all: vga_mod

%.o : %.c
	@echo "        [vga] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "        [vga] AS $<"
	@$(AS) $(ASFLAGS) $<

vga_mod: $(OBJS)
	@echo "        [vga] LD $(LDFLAGS) -o ../../modobjs/vga_mod.o *.o"
	@$(LD) $(LDFLAGS) -o ../../modobjs/vga_mod.o *.o

clean:
	@rm -f *.o

.PHONY: all
