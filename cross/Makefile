MAKE=make
PREFIX=$(shell pwd)
TARGET=i586-elf
BINUTILS=binutils-2.23.1
GCC=gcc-4.7.2
GMP=gmp-5.0.5
MPC=mpc-1.0.1
MPFR=mpfr-3.1.1
CHECKFILE=.cross_check

CLEAN_DIRS=$(GCC) $(BINUTILS) $(GMP) $(MPC) $(MPFR)

all: check download binutils gmp mpfr mpc gcc test

check:
	@if [ ! -e $(CHECKFILE) ]; then \
		touch $(CHECKFILE); \
	else \
		echo -e "[\033[0;31mWarning\033[0;0m]";\
		echo -e "Cross compiler already built, build again?\n[y/n] \c";\
		read input;\
		if [ $$input != "y" ]; then \
			echo "Abandon ship!";\
			exit 1;\
		fi;\
	fi

download: 
	@if [ ! -e $(BINUTILS).tar.bz2 ]; then \
		echo -e "[\033[0;32mDownloading binutils\033[0;0m]";\
		wget http://ftp.gnu.org/gnu/binutils/$(BINUTILS).tar.bz2; \
		echo -e "[\033[0;32mdone\033[0;0m]";\
	fi
	@if [ ! -e $(GMP).tar.bz2 ]; then \
		echo -e "[\033[0;32mDownloading gmp\033[0;0m]";\
		wget http://ftp.gnu.org/gnu/gmp/$(GMP).tar.bz2; \
		echo -e "[\033[0;32mdone\033[0;0m]";\
	fi
	@if [ ! -e $(MPC).tar.gz ]; then \
		echo -e "[\033[0;32mDownloading mpc\033[0;0m]";\
		wget http://ftp.gnu.org/gnu/mpc/$(MPC).tar.gz; \
		echo -e "[\033[0;32mdone\033[0;0m]";\
	fi
	@if [ ! -e $(MPFR).tar.bz2 ]; then \
		echo -e "[\033[0;32mDownloading mpfr\033[0;0m]";\
		wget http://ftp.gnu.org/gnu/mpfr/$(MPFR).tar.bz2; \
		echo -e "[\033[0;32mdone\033[0;0m]";\
	fi
	@if [ ! -e $(GCC).tar.bz2 ]; then \
		echo -e "[\033[0;32mDownloading gcc\033[0;0m]";\
		wget http://ftp.gnu.org/gnu/gcc/gcc-4.7.2/$(GCC).tar.bz2; \
		echo -e "[\033[0;32mdone\033[0;0m]";\
	fi

binutils:
	@echo -e "[\033[0;32mMaking $(BINUTILS)\033[0;0m]";
	@echo Extracting...
	@tar xf $(BINUTILS).tar.bz2
	@cd $(BINUTILS); ./configure --prefix=$(PREFIX) --target=$(TARGET) --disable-nls; \
	$(MAKE); $(MAKE) install;

gmp:
	@echo -e "[\033[0;32mMaking $(GMP)\033[0;0m]"
	@echo Extracting...
	@tar xf $(GMP).tar.bz2
	@cd $(GMP); ./configure --prefix=$(PREFIX); \
	$(MAKE); $(MAKE) install;

mpc:
	@echo -e "[\033[0;32mMaking $(MPC)\033[0;0m]";
	@echo Extracting...
	@tar xf $(MPC).tar.gz
	@cd $(MPC); ./configure --prefix=$(PREFIX) --with-gmp=$(PREFIX); --with-mpfr=$(PREFIX)\
	$(MAKE); $(MAKE) install;

mpfr:
	@echo -e "[\033[0;32mMaking $(MPFR)\033[0;0m]";
	@echo Extracting...
	@tar xf $(MPFR).tar.bz2
	@cd $(MPFR); ./configure --prefix=$(PREFIX) --with-gmp=$(PREFIX);\
	$(MAKE); $(MAKE) install;

gcc:
	@echo -e "[\033[0;32mMaking $(GCC)\033[0;0m]";
	@echo Extracting...
	@tar xf $(GCC).tar.bz2
	@cd $(GCC); export PATH="$$PATH:$(PREFIX)";\
	./configure --prefix=$(PREFIX) --target=$(TARGET) --disable-nls --enable-languages=c --without-headers --with-gmp=$(PREFIX) --with-mpc=$(PREFIX) --with-mpfr=$(PREFIX);\
	$(MAKE); $(MAKE) install;

test:
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
	rm -rf $(CLEAN_DIRS)
	rm $(CHECKFILE)

clean-archives:
	rm -rf *.tar.*

clean-all: clean-build clean-archives
	rm -rf *.*
	
.PHONY: all
