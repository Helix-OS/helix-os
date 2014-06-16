SRC = $(wildcard *.c)
OBJ = $(SRC:.c=)
CFLAGS=-g -I"$(PREFIX)/include" -nostdlib -nodefaultlibs -nostartfiles

.c:
	@echo CC $<
	$(CC) $(CFLAGS) -o $@ $< $(PREFIX)/lib/{crt0,libc,syscalls}.o

all: $(OBJ)

clean:
	@-rm $(OBJ)

install:
	cp $(OBJ) $(PREFIX)/bin

.PHONY: all
