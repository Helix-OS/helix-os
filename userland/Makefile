
subdirs:
	@mkdir -p build/{bin,lib}

all:
	@cd dalibc; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) ARCH=i386 HOST=helix

clean:
	@-cd dalibc; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) ARCH=i386 HOST=helix clean

.PHONY: all
