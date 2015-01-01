MAKE=make
PREFIX=$(shell pwd)
TARGET=i586-elf
BINUTILS=binutils-2.24
GCC=gcc-4.8.2

CLEAN_DIRS=binbuild-$(TARGET) gccbuild-$(TARGET)
CLEAN_SOURCE=$(BINUTILS) $(GCC)

all: bin/$(TARGET)-ld bin/$(TARGET)-gcc

$(BINUTILS).tar.gz:
	wget "http://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.gz"

$(GCC).tar.gz:
	wget "http://ftp.gnu.org/gnu/gcc/$(GCC)/$(GCC).tar.gz"

$(BINUTILS): $(BINUTILS).tar.gz
	tar xzf $(BINUTILS).tar.gz

$(GCC): $(GCC).tar.gz
	tar xzf $(GCC).tar.gz

bin/$(TARGET)-ld bin/$(TARGET)-as bin/$(TARGET)-readelf bin/$(TARGET)-objcopy: $(BINUTILS)
	mkdir -p binbuild-$(TARGET)
	cd binbuild-$(TARGET); ../$(BINUTILS)/configure --disable-werror --prefix=$(PREFIX) \
		--target=$(TARGET) --disable-nls; \
	$(MAKE); $(MAKE) install;

bin/$(TARGET)-gcc bin/$(TARGET)-g++: $(GCC) bin/$(TARGET)-ld bin/$(TARGET)-as
	mkdir -p gccbuild-$(TARGET)
	cd gccbuild-$(TARGET); export PATH="$$PATH:$(PREFIX)";\
	../$(GCC)/configure --prefix=$(PREFIX) --target=$(TARGET) \
		--disable-nls --enable-languages=c,c++ --without-headers; \
	$(MAKE) all-gcc all-target-libgcc; \
	$(MAKE) install-gcc install-target-libgcc;

clean-build:
	-rm -rf $(CLEAN_DIRS)

clean-source:
	-rm -rf $(CLEAN_SOURCE)

clean-archives:
	-rm -rf *.tar.*

clean-bin:
	-rm -rf ./bin ./$(TARGET) ./include ./lib ./share ./libexec

clean-all: clean-build clean-archives clean-bin

.PHONY: all
