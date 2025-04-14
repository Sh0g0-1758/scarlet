	.globl main
	.text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $0, %rsp
	movl $0, %eax
	movq %rbp, %rsp
	popq %rbp
	ret
	movl $0, %eax
	movq %rbp, %rsp
	popq %rbp
	ret
	.data
	.align 8
ptr.0:
	.quad 0
	.section    .note.GNU-stack,"",@progbits
