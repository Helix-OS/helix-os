#EMULATOR      = qemu-system-arm
#EMU_FLAGS     = -hda helix.img -s -serial tcp::9090,server -m 32
EMULATOR      = "echo"
EMU_FLAGS     = "Emulation for the raspi isn't supported yet, sorry."
TARGET        = $(ARCH)-none-eabi

ARCH_TARGETS  = raspi-kimage

image:
	@echo -e "[ ] Generating image..."
	@cp -r boot-image build
	@cp kernel/helix_kernel-$(ARCH) build
	@tar c kernel/modobjs kernel/config > build/initrd.tar
	@./tools/mk_image.sh
	@echo -e "[ ] done"
