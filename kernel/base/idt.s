[GLOBAL idt_flush]

idt_flush:
	mov eax, [esp + 4]
	lidt [eax]
	ret

%macro ISR_NOERROR 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERROR 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push byte %1
		jmp isr_common_stub
%endmacro

%macro IRQ 2
	[GLOBAL irq%1]
	irq%1:
		cli
		push byte 0
		push byte %2
		jmp irq_common_stub
%endmacro

ISR_NOERROR 0
ISR_NOERROR 1
ISR_NOERROR 2
ISR_NOERROR 3
ISR_NOERROR 4
ISR_NOERROR 5
ISR_NOERROR 6
ISR_NOERROR 7
ISR_ERROR   8
ISR_NOERROR 9
ISR_ERROR   10
ISR_ERROR   11
ISR_ERROR   12
ISR_ERROR   13
ISR_ERROR   14
ISR_NOERROR 15
ISR_NOERROR 16
ISR_NOERROR 17
ISR_NOERROR 18
ISR_NOERROR 19
ISR_NOERROR 20
ISR_NOERROR 21
ISR_NOERROR 22
ISR_NOERROR 23
ISR_NOERROR 24
ISR_NOERROR 25
ISR_NOERROR 26
ISR_NOERROR 27
ISR_NOERROR 28
ISR_NOERROR 29
ISR_NOERROR 30
ISR_NOERROR 31

IRQ 	0,	32
IRQ	1,	33
IRQ	2,	34
IRQ	3,	35
IRQ	4,	36
IRQ	5,	37
IRQ	6,	38
IRQ	7,	39
IRQ	8,	40
IRQ	9,	41
IRQ	10,	42
IRQ	11,	43
IRQ	12,	44
IRQ	13,	45
IRQ	14,	46
IRQ	15,	47

ISR_NOERROR 48
ISR_NOERROR 80 

[EXTERN isr_handler]
[EXTERN irq_handler]

isr_common_stub:
	pusha

	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call isr_handler

	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	popa
	add esp, 8
	sti
	iret

irq_common_stub:
	pusha

	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call irq_handler

	pop ebx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx

	popa
	add esp, 8
	sti
	iret
