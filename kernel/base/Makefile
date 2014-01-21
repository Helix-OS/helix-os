CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= 
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -T linker.ld

#OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
#OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))
OBJS 		=  $(patsubst %.c, %.o, $(shell find . -name "*.c" ))
OBJS 		+= $(patsubst %.s, %.o, $(shell find . -name "*.s" ))

all: kernel

%.o : %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $@

%.o : %.s
	@echo AS $<
	@$(AS) $(ASFLAGS) $< -o $@

kernel: $(OBJS)
	@$(LD) $(LDFLAGS) -o helix_kernel-$(ARCH) $(OBJS)
	@echo LD $(LDFLAGS) -o helix_kernel-$(ARCH) $(OBJS)

clean:
	-rm $(OBJS)

.PHONY: all
