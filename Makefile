CONFIG        = config.mk
CURRENT_VER   = 0.1.0
ALL_TARGETS   =
ALL_CLEAN     =

.PHONY: do_all
do_all: all

include buildconf/$(CONFIG)
include kernel/Makefile.objs

# TODO: have these be included too
ALL_TARGETS += userspace $(ARCH_TARGETS)
ALL_CLEAN   += user-clean tools-clean

.PHONY: all
all: $(ALL_TARGETS)
dev-all: $(ALL_TARGETS) ktools docs

ktools:
	@cd tools; $(MAKE) ARCH=$(ARCH) PLATFORM=$(PLATFORM)

userspace: $(MKCONFIG)
	@cd userland; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT)

cross-cc: check
	@echo -e "[ ] Making cross-compiler..."
	@cd cross; $(MAKE) MAKE=$(MAKE) TARGET=$(TARGET)
	@echo -e "[ ] done"

debug:
	@gdb -x tools/gdbscript

test:
	$(EMULATOR) $(EMU_FLAGS)

docs:
	@echo -e "[ ] Making documentation..."
	@cd doc; doxygen doxy.conf > /dev/null
	@echo -e "[ ] done";

user-clean:
	@-cd userland; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT) clean

tools-clean:
	@-cd tools; $(MAKE) clean

clean: $(ALL_CLEAN)
	@-rm -rf build
	@-rm *.img
	@-rm $(MKCONFIG)
