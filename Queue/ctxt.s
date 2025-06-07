	.file	"ctxt.c"
	.text
	.globl	_Node
	.bss
	.align 4
_Node:
	.space 12
	.text
	.globl	_EnQuqu
	.def	_EnQuqu;	.scl	2;	.type	32;	.endef
_EnQuqu:
LFB12:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	cmpl	$0, 8(%ebp)
	jne	L2
	movl	$1, %eax
	jmp	L3
L2:
	movl	$12, (%esp)
	call	_malloc
	movl	%eax, -12(%ebp)
	movl	8(%ebp), %eax
	movl	4(%eax), %eax
	movl	-12(%ebp), %edx
	movl	%edx, 8(%eax)
	movl	12(%ebp), %edx
	movl	-12(%ebp), %eax
	movl	%edx, (%eax)
	movl	16(%ebp), %edx
	movl	-12(%ebp), %eax
	movl	%edx, 4(%eax)
	movl	-12(%ebp), %eax
	movl	$0, 8(%eax)
	movl	8(%ebp), %eax
	movl	-12(%ebp), %edx
	movl	%edx, 4(%eax)
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	leal	1(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, 8(%eax)
	movl	$0, %eax
L3:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE12:
	.globl	_DeQuqu
	.def	_DeQuqu;	.scl	2;	.type	32;	.endef
_DeQuqu:
LFB13:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	8(%ebp), %eax
	movl	4(%eax), %eax
	cmpl	%eax, %edx
	jne	L5
	movl	$1, %eax
	jmp	L6
L5:
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	8(%eax), %eax
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	movl	%edx, (%eax)
	movl	-12(%ebp), %eax
	movl	4(%eax), %eax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	movl	%edx, 4(%eax)
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	-12(%ebp), %edx
	movl	8(%edx), %edx
	movl	%edx, 8(%eax)
	movl	-12(%ebp), %eax
	movl	8(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	leal	-1(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, 8(%eax)
	movl	8(%ebp), %eax
	movl	4(%eax), %eax
	cmpl	%eax, -12(%ebp)
	jne	L7
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, 4(%eax)
L7:
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	_free
	movl	$0, %eax
L6:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE13:
	.globl	_Q_Full
	.def	_Q_Full;	.scl	2;	.type	32;	.endef
_Q_Full:
LFB14:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	8(%eax), %eax
	cmpl	$9, %eax
	setbe	%al
	movzbl	%al, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE14:
	.globl	_Read_Q
	.def	_Read_Q;	.scl	2;	.type	32;	.endef
_Read_Q:
LFB15:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	8(%eax), %eax
	movl	%eax, -12(%ebp)
	jmp	L11
L17:
	movl	-12(%ebp), %eax
	movl	(%eax), %eax
	cmpl	%eax, 16(%ebp)
	jne	L12
	cmpl	$1, 16(%ebp)
	je	L13
	cmpl	$2, 16(%ebp)
	je	L14
	jmp	L11
L13:
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_DeQuqu
	jmp	L11
L14:
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_DeQuqu
	nop
	jmp	L11
L12:
	movl	-12(%ebp), %eax
	movl	8(%eax), %eax
	movl	%eax, -12(%ebp)
L11:
	cmpl	$0, -12(%ebp)
	jne	L17
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE15:
	.globl	_Quequ_Init
	.def	_Quequ_Init;	.scl	2;	.type	32;	.endef
_Quequ_Init:
LFB16:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	movl	8(%ebp), %edx
	movl	%edx, 4(%eax)
	movl	8(%ebp), %eax
	movl	4(%eax), %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	movl	8(%ebp), %eax
	movl	(%eax), %eax
	movl	$0, 8(%eax)
	movl	$0, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE16:
	.def	___main;	.scl	2;	.type	32;	.endef
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
LFB17:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	andl	$-16, %esp
	subl	$32, %esp
	call	___main
	movl	$12, (%esp)
	call	_malloc
	movl	%eax, 28(%esp)
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE17:
	.ident	"GCC: (GNU) 11.3.0"
	.def	_malloc;	.scl	2;	.type	32;	.endef
	.def	_free;	.scl	2;	.type	32;	.endef
