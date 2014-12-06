CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586

INCLUDE		= 
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DPLATFORM=$(PLATFORM) -DARCH=$(ARCH) -I$(INCLUDE) -g
ASFLAGS		= -f elf
LDFLAGS		= -T arch/$(ARCH)/targets/$(PLATFORM)/linker.ld

OBJS 		+= $(patsubst %.c, %.o, $(shell find arch/$(ARCH) -name "*.c" ))
OBJS 		+= $(patsubst %.s, %.o, $(shell find arch/$(ARCH) -name "*.s" ))

OBJS 		+= $(patsubst %.c, %.o, $(shell find arch/$(ARCH)/targets/$(PLATFORM) -name "*.c" ))
OBJS 		+= $(patsubst %.s, %.o, $(shell find arch/$(ARCH)/targets/$(PLATFORM) -name "*.s" ))

OBJS 		+= $(patsubst %.c, %.o, $(shell find mem tasking vfs datastructs -name "*.c" ))
OBJS 		+= $(patsubst %.s, %.o, $(shell find mem tasking vfs datastructs -name "*.s" ))

OBJS 		+= $(patsubst %.c, %.o, $(wildcard *.c))
OBJS 		+= $(patsubst %.s, %.o, $(wildcard *.s))

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
