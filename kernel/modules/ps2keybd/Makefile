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

all: ps2keybd_mod

%.o : %.c
	@echo "[ps2keybd] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "[ps2keybd] AS $<"
	@$(AS) $(ASFLAGS) $<

ps2keybd_mod: $(OBJS)
	@echo "[ps2keybd] LD $(LDFLAGS) -o ../../modobjs/ps2keybd_mod.o *.o"
	@$(LD) $(LDFLAGS) -o ../../modobjs/ps2keybd_mod.o *.o

clean:
	@rm -f *.o

.PHONY: all
