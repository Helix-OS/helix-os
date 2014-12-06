CC		= gcc
AS		= nasm
LD		= ld
ARCH		= i586
PLATFORM        = pc

INCLUDE		= $(shell pwd)/include
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DPLATFORM=$(PLATFORM) -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -T linker.ld

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

MODULES 	= $(wildcard modules/*)

all: kernel kmodules

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.o : %.s
	$(AS) $(ASFLAGS) $<

kernel: $(OBJS)
	@cd base; make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) PLATFORM=$(PLATFORM) INCLUDE=$(INCLUDE) 
	@mv base/helix_kernel-$(ARCH) .

kmodules:
	@mkdir -p modobjs
	@for module in $(MODULES); do\
		cd $$module;\
		make AS=$(AS) CC=$(CC) LD=$(LD) ARCH=$(ARCH) INCLUDE=$(INCLUDE);\
		cd ../..;\
	done;

clean:
	-@for module in $(MODULES); do\
		cd $$module;\
		make clean;\
		cd ../..;\
	done;
	-cd base; make clean
	-rm modobjs/*.o

.PHONY: all
