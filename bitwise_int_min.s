	.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $8, %rsp
	movl $2147483647, -4(%rbp)
	negl -4(%rbp)
	movl -4(%rbp), -8(%rbp)
	notl -8(%rbp)
	movl -8(%rbp), %eax
	movq %rbp, %rsp
	popq %rbp
	ret
	.section    .note.GNU-stack,"",@progbits
