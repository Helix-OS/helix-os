CFLAGS		= -nostartfiles -nodefaultlibs -nostdlib -fno-builtin -Wall -DARCH=$(ARCH) -I$(INCLUDE)
LDFLAGS		= -fPIC -shared

OBJS=$(patsubst %.c, %.o, $(wildcard *c))

%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $<

all: $(OBJS)
	@#echo LD *.o
	@#$(LD) $(LDFLAGS) -o ../ramfs.o *.o
	@mv *.o ../

clean:
	@-rm *.o

.PHONY: all
