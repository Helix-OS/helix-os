CONFIG        = config.mk
CURRENT_VER   = 0.1.0
ARCH          = i586
PLATFORM      = pc
CROSS         = $(shell pwd)/cross
ALL_TARGETS   =

.PHONY: do_all
do_all: all

include buildconf/$(CONFIG)

all: $(ALL_TARGETS)
dev-all: $(ALL_TARGETS) ktools docs

helix-kernel: $(MKCONFIG)
	@cd kernel; $(MAKE) MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT)

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

kernel-clean:
	@-cd kernel; $(MAKE) MKCONFIG="$(MKCONFIG)" MKROOT="$(MKROOT)" clean

tools-clean:
	@-cd tools; $(MAKE) clean

clean: kernel-clean user-clean tools-clean
	@-cd tools; $(MAKE) clean
	@-rm *.img
	@-rm -rf build
	@-rm $(MKCONFIG)

check:
	@if [ ! -e cross/.cross_check ]; then \
		echo "[W] Warning:";\
		echo "[W] Building the cross compiler using \`$(MAKE) cross-cc\` is recommended for best results.";\
		echo "[W] Things may break otherwise, proceed at your own risk.";\
	fi
	@./tools/check_depends.sh

