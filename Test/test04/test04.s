	.file	"test04.c"
	.intel_syntax noprefix
	.comm	fp,8,8
	.section	.rodata
.LC0:
	.string	"A->"
	.text
	.globl	A
	.type	A, @function
A:
.LFB2:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	QWORD PTR fp[rip], OFFSET FLAT:B
	mov	edi, OFFSET FLAT:.LC0
	mov	eax, 0
	call	printf
	mov	rdx, QWORD PTR fp[rip]
	mov	eax, 0
	call	rdx
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	A, .-A
	.section	.rodata
.LC1:
	.string	"C->"
	.text
	.globl	C
	.type	C, @function
C:
.LFB3:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	QWORD PTR fp[rip], OFFSET FLAT:D
	mov	edi, OFFSET FLAT:.LC1
	mov	eax, 0
	call	printf
	mov	rdx, QWORD PTR fp[rip]
	mov	eax, 0
	call	rdx
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	C, .-C
	.section	.rodata
.LC2:
	.string	"B->"
	.text
	.globl	B
	.type	B, @function
B:
.LFB4:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	QWORD PTR fp[rip], OFFSET FLAT:E
	mov	edi, OFFSET FLAT:.LC2
	mov	eax, 0
	call	printf
	mov	rdx, QWORD PTR fp[rip]
	mov	eax, 0
	call	rdx
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	B, .-B
	.section	.rodata
.LC3:
	.string	"D->"
	.text
	.globl	D
	.type	D, @function
D:
.LFB5:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	QWORD PTR fp[rip], OFFSET FLAT:E
	mov	edi, OFFSET FLAT:.LC3
	mov	eax, 0
	call	printf
	mov	rdx, QWORD PTR fp[rip]
	mov	eax, 0
	call	rdx
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	D, .-D
	.section	.rodata
.LC4:
	.string	"E"
	.text
	.globl	E
	.type	E, @function
E:
.LFB6:
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	mov	edi, OFFSET FLAT:.LC4
	call	puts
	nop
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	E, .-E
	.globl	main
	.type	main, @function
main:
.LFB7:
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
	mov	DWORD PTR [rbp-4], eax
	mov	eax, DWORD PTR [rbp-4]
	and	eax, 1
	test	eax, eax
	jne	.L7
	mov	eax, 0
	call	A
	jmp	.L8
.L7:
	mov	eax, 0
	call	C
.L8:
	mov	eax, 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.5) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
