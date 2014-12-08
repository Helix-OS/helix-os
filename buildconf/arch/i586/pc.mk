EMULATOR      = qemu-system-i386
EMU_FLAGS     = -hda helix.img -s -serial tcp::9090,server -m 32
TARGET        = $(ARCH)-elf

ARCH_TARGETS  = image

image:
	@echo -e "[ ] Generating image..."
	@cp -r boot-image build
	@cp kernel/helix_kernel-i586 build
	@tar c kernel/modobjs kernel/config > build/initrd.tar
	@./tools/mk_image.sh
	@echo "To boot: $(EMULATOR) $(EMU_FLAGS)"
	@echo -e "[ ] done"
