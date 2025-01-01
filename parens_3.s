	.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $8, %rsp
	movl $4, -4(%rbp)
	negl -4(%rbp)
	movl -4(%rbp), -8(%rbp)
	negl -8(%rbp)
	movl -8(%rbp), %eax
	movq %rbp, %rsp
	popq %rbp
	ret
	.section    .note.GNU-stack,"",@progbits
