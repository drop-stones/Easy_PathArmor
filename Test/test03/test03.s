	.file	"test03.c"
	.intel_syntax noprefix
	.section	.rodata
.LC0:
	.string	"label0"
.LC1:
	.string	"label1"
.LC2:
	.string	"label2"
.LC3:
	.string	"label3 (can't reach!)"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 32
	mov	DWORD PTR [rbp-20], edi
	mov	QWORD PTR [rbp-32], rsi
	mov	rax, QWORD PTR [rbp-32]
	add	rax, 8
	mov	rax, QWORD PTR [rax]
	mov	rdi, rax
	call	atoi
	mov	ecx, eax
	mov	edx, 1431655766
	mov	eax, ecx
	imul	edx
	mov	eax, ecx
	sar	eax, 31
	sub	edx, eax
	mov	eax, edx
	mov	DWORD PTR [rbp-4], eax
	mov	edx, DWORD PTR [rbp-4]
	mov	eax, edx
	add	eax, eax
	add	eax, edx
	sub	ecx, eax
	mov	eax, ecx
	mov	DWORD PTR [rbp-4], eax
	nop
	nop
	nop
	imul	rax, 0xc
	lea	rax, [rax + .L2]  #[.L2 + rax * 12]
	jmp	rax
.L2:
	mov	edi, OFFSET FLAT:.LC0
	call	puts
	jmp	.L6
.L3:
	mov	edi, OFFSET FLAT:.LC1
	call	puts
	jmp	.L6
.L4:
	mov	edi, OFFSET FLAT:.LC2
	call	puts
	jmp	.L6
.L5:
	mov	edi, OFFSET FLAT:.LC3
	call	puts
	jmp	.L6
.L6:
	mov	eax, 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
