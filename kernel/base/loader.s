BITS 32

global loader					; making entry point visible to linker
 
extern kmain					; kmain is defined in kmain.c
 
; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN	equ 1<<0			; align loaded modules on page boundaries
MEMINFO		equ 1<<1			; provide memory map
FLAGS		equ MODULEALIGN | MEMINFO	; this is the Multiboot 'flag' field
MAGIC		equ 0x1BADB002			; 'magic number' lets bootloader find the header
CHECKSUM	equ -(MAGIC + FLAGS)		; checksum required
 
section .__mbHeader
 
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .text
 
align 4
; reserve initial kernel stack space
STACKSIZE equ 0x4000					; that's 16k.

global mboot
extern end

loader:
	mov esp, stack + STACKSIZE	 ; set up the stack
	mov ebp, 0

	push eax			; Multiboot magic number
	push esp			; Current stack pointer
	push ebx			; Multiboot info structure
 
	call kmain			; call kernel proper
 
	cli
.hang:
	hlt				; halt machine should kernel return
	jmp  .hang
 
section .bss
 
align 4
stack:
	resb STACKSIZE			; reserve 16k stack on a doubleword boundary
