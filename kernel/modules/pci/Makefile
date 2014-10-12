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

all: pci_mod

%.o : %.c
	@echo "[pci] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "[pci] AS $<"
	@$(AS) $(ASFLAGS) $<

pci_mod: $(OBJS)
	@echo "[pci] LD $(LDFLAGS) -o ../../modobjs/pci_mod.o *.o"
	@$(LD) $(LDFLAGS) -o ../../modobjs/pci_mod.o *.o

clean:
	@rm -f *.o

.PHONY: all
