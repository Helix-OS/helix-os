BITS 32

global loader                                        ; making entry point visible to linker
 
;extern kmain                                        ; kmain is defined in kmain.c
extern arch_init
 
; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN  equ 1<<0                          ; align loaded modules on page boundaries
MEMINFO      equ 1<<1                          ; provide memory map
VIDEO        equ 1<<2                          ; request video mode
FLAGS        equ MODULEALIGN | MEMINFO | VIDEO ; this is the Multiboot 'flag' field
MAGIC        equ 0x1BADB002                    ; 'magic number' lets bootloader find the header
CHECKSUM     equ -(MAGIC + FLAGS)              ; checksum required

KERNEL_VBASE    equ 0xc0000000
KERNEL_PAGE_NUM equ (KERNEL_VBASE >> 22)
 
section .__mbHeader
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd 0 ; unused
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0    ; prefered video type, 0 for linear and 1 for text
    dd 1024 ; prefered video width
    dd 768  ; prefered video height
    dd 32   ; prefered video bpp

section .data
align 0x1000
boot_page_dir:
    dd 0x00000083
    times (KERNEL_PAGE_NUM - 1) dd 0
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUM - 1) dd 0

section .text
 
align 4
; reserve initial kernel stack space
STACKSIZE equ 0x4000                           ; that's 16k.

global mboot
extern end

loader:
    mov ecx, boot_page_dir - KERNEL_VBASE
    mov cr3, ecx

    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    lea ecx, [higher_half_start]
    jmp ecx

global higher_half_start
higher_half_start:
    ;mov dword [boot_page_dir], 0
    ;invlpg [0]

    mov esp, stack + STACKSIZE                 ; set up the stack
    mov ebp, 0

    push eax                                   ; Multiboot magic number
    push esp                                   ; Current stack pointer

    add ebx, KERNEL_VBASE
    push ebx                                   ; Multiboot info structure

    ;call kmain                                ; call kernel proper
    call arch_init                             ; call kernel proper

    cli
.hang:
    hlt                                        ; halt machine should kernel return
    jmp  .hang
 
section .bss
align 4
stack:
    resb STACKSIZE                             ; reserve 16k stack on a doubleword boundary
