include $(MKROOT)/$(MKCONFIG)

MODULES = $(wildcard modules/*)
MODOBJS = $(patsubst modules/%, modobjs/%_mod.o, $(MODULES))
INCLUDE = $(shell pwd)/include

all: kernel kmodules

kernel:
	@cd base; make MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT) INCLUDE=$(INCLUDE)
	@mv base/helix_kernel-$(ARCH) .

modobjs:
	@mkdir modobjs

modobjs/%_mod.o : modules/%
	@cd $<; make MKCONFIG=$(MKCONFIG) MKROOT=$(MKROOT) INCLUDE=$(INCLUDE)

kmodules: modobjs $(MODOBJS)

clean:
	-@for module in $(MODULES); do\
		cd $$module;\
		make clean;\
		cd ../..;\
	done;
	-cd base; make clean
	-rm modobjs/*.o

.PHONY: all
