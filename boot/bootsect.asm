global _start, start
extern kmain
    
%define MULTIBOOT_HEADER_MAGIC  0x1BADB002
%define MULTIBOOT_HEADER_FLAGS	0x00000003
%define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


_start:
	jmp start

; The Multiboot header
align 4
multiboot_header:
dd MULTIBOOT_HEADER_MAGIC
dd MULTIBOOT_HEADER_FLAGS
dd CHECKSUM     
; ----- Multiboot Header Ends Here -----

start:
	push ebx
	call kmain

	cli ; stop interrupts
	hlt ; halt the CPU

