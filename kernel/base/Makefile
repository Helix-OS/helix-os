CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= 
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -T linker.ld

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

all: kernel

%.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo AS $<
	@$(AS) $(ASFLAGS) $<

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -o helix_kernel-$(ARCH) *.o

clean:
	-rm *.o

.PHONY: all
