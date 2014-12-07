include $(MKROOT)/$(MKCONFIG)

INCLUDE		= $(shell pwd)/../include
CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
ASFLAGS		= -f elf
LDFLAGS		= -fPIC -shared

OBJS		= $(patsubst %.c, %.o, $(wildcard *c))
OBJS		+=$(patsubst %.s, %.o, $(wildcard *s))

all: devfs_mod

%.o : %.c
	@echo "      [devfs] CC $<"
	@$(CC) $(CFLAGS) -c $<

%.o : %.s
	@echo "      [devfs] AS $<"
	@$(AS) $(ASFLAGS) $<

devfs_mod: $(OBJS)
	@echo "      [devfs] LD $(LDFLAGS) -o ../../modobjs/devfs_mod.o *.o"
	@$(LD) $(LDFLAGS) -o ../../modobjs/devfs_mod.o *.o

clean:
	@-rm -f *.o

.PHONY: all
