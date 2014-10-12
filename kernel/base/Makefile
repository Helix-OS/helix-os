CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= 
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE) -g
ASFLAGS		= -f elf
LDFLAGS		= -T linker.ld

OBJS 		=  $(patsubst %.c, %.o, $(shell find . -name "*.c" ))
OBJS 		+= $(patsubst %.s, %.o, $(shell find . -name "*.s" ))

all: kernel

%.o : %.c
	@echo "[kernel base] CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o : %.s
	@echo "[kernel base] AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

kernel: $(OBJS)
	@$(CC) $(LDFLAGS) $(CFLAGS) -lgcc -o helix_kernel-$(ARCH) $(OBJS)
	@echo "[kernel base] CC -o helix_kernel-$(ARCH) *.o"
	@#echo CC $(LDFLAGS) $(CFLAGS) -lgcc -o helix_kernel-$(ARCH) $(OBJS)

clean:
	-rm $(OBJS)

.PHONY: all
