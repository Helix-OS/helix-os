CC	= gcc
CFLAGS	= -Wall -O2
OBJS	= $(patsubst %.c, %, $(wildcard *.c))

all: $(OBJS)

clean:
	rm $(OBJS)

.PHONY: all
