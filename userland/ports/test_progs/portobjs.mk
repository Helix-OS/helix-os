test_progs-port: userland/build dalibc-port
	@cd userland/ports/test_progs; \
		$(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) PREFIX=$(PWD)"/userland/build" all install

test_progs-port-clean:
	@-cd userland/ports/test_progs; \
		$(MAKE) AS=$(AS) CC=$(CC) LD=$(LD) PREFIX=$(PWD)"/userland/build" clean

USER_CLEAN += test_progs-port-clean
