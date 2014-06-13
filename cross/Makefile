MAKE=make
PREFIX=$(shell pwd)
TARGET=i586-elf
BINUTILS=binutils-2.24
GCC=gcc-4.8.2
GMP=gmp-5.1.3
MPC=mpc-1.0.1
MPFR=mpfr-3.1.2
CHECKFILE=.cross_check

CLEAN_DIRS=$(BINUTILS) $(MPFR) $(MPC) $(GMP) $(GCC)

all: check download binutils mpfr gmp mpc gcc test

check:
	@if [ ! -e $(CHECKFILE) ]; then \
		touch $(CHECKFILE); \
	else \
		echo -e "[\033[0;31mWarning\033[0;0m]";\
		echo -e "Cross compiler already built, build again?\n[y/N] \c";\
		read input;\
		if [ $$input != "y" ]; then \
			echo "Abandon ship!";\
			exit 1;\
		fi;\
	fi

download: check
	@for archive in $(CLEAN_DIRS); do \
		if [ ! -e $$archive ]; then \
			if [ ! -e $$archive.tar.gz ]; then\
				echo -e "[\033[0;32mDownloading $$archive\033[0;0m]";\
				_NAME=`echo $$archive | sed 's/-.*//g'`;\
				if [ $$_NAME = "gcc" ]; then \
					wget http://ftp.gnu.org/gnu/$$_NAME/$$archive/$$archive.tar.gz; \
				else \
					wget http://ftp.gnu.org/gnu/$$_NAME/$$archive.tar.gz; \
				fi;\
			fi;\
			echo -e "[\033[0;32mExtracting $$archive\033[0;0m]";\
			nfiles="`tar vft $$archive.tar.gz | wc -l`";\
			tar xvf $$archive.tar.gz | perl pretty.pl $$nfiles; \
			echo -e "[\033[0;32mdone\033[0;0m]";\
		fi;\
	done

binutils: download
	@echo -e "[\033[0;32mMaking $(BINUTILS)\033[0;0m]";
	@cd $(BINUTILS); ./configure --disable-werror --prefix=$(PREFIX) --target=$(TARGET) --disable-nls; \
	$(MAKE); $(MAKE) install;

gmp: download
	@echo -e "[\033[0;32mMaking $(GMP)\033[0;0m]"
	@cd $(GMP); ./configure --prefix=$(PREFIX); \
	$(MAKE); $(MAKE) install;

mpfr: download gmp
	@echo -e "[\033[0;32mMaking $(MPFR)\033[0;0m]";
	@cd $(MPFR); ./configure --prefix=$(PREFIX) --with-gmp=$(PREFIX);\
	$(MAKE); $(MAKE) install;

mpc: download gmp mpfr
	@echo -e "[\033[0;32mMaking $(MPC)\033[0;0m]";
	@cd $(MPC); ./configure --prefix=$(PREFIX) --with-gmp=$(PREFIX) --with-mpfr=$(PREFIX);\
	$(MAKE); $(MAKE) install;

gcc: download binutils gmp mpfr mpc
	@echo -e "[\033[0;32mMaking $(GCC)\033[0;0m]";
	@cd $(GCC); export PATH="$$PATH:$(PREFIX)";\
	./configure --prefix=$(PREFIX) --target=$(TARGET) --disable-nls --enable-languages=c,c++ \
	--disable-install-libiberty \
	--without-headers --with-gmp=$(PREFIX) --with-mpc=$(PREFIX) --with-mpfr=$(PREFIX);\
	$(MAKE) all-gcc; $(MAKE) all-target-gcc;\
	$(MAKE) install-gcc; ##$(MAKE) install-target-gcc;

test: gcc
	@echo -e "[\033[0;32mTesting build...\033[0;0m]";
	@echo "int main(){return 0;}" > cross_test.c
	@$(PREFIX)/bin/$(TARGET)-gcc -nostdlib -nodefaultlibs -o cross_test cross_test.c;\
	if [ $$? -gt 0 ]; then \
		echo -e "[\033[0;31merror\033[0;0m] Test failed";\
	else \
		echo -e "[\033[0;32mTest passed\033[0;0m]";\
	fi
	@rm cross_test*

clean-build:
	-rm -rf $(CLEAN_DIRS)
	-rm $(CHECKFILE)

clean-archives:
	-rm -rf *.tar.*

clean-bin:
	-rm -rf ./bin ./i586-elf ./include ./lib ./share ./libexec

clean-all: clean-build clean-archives clean-bin

.PHONY: all
