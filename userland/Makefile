include $(MKROOT)/$(MKCONFIG)

.PHONY: all
all: subdirs usertree libs

subdirs:
	@mkdir -p build/{bin,lib}

libs:
	@cd dalibc; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) \
		ARCH=i386 HOST=helix PREFIX=$(shell pwd)"/build" all install
	@cd test_progs; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) PREFIX=$(shell pwd)"/build" all install

.PHONY: usertree
usertree:
	cp -rv tree/* build


image:
	@dd if=/dev/zero of=user.hdd bs=1M count=16
	@mkfs.vfat user.hdd
	@mkdir mnt
	@sudo mount -t vfat user.hdd mnt
	@sudo cp -r build/* mnt
	@sudo umount mnt
	@rmdir mnt

clean:
	@-cd dalibc; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) ARCH=i386 HOST=helix clean
	@-cd test_progs; $(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) clean
	@-rm -rf build
	@-rm user.hdd

