  .section .text.foo.near
	.globl foo_near

foo_near:
	addi zero, zero, 0


	.section .text.foo.far
	.globl foo_far

foo_far:
	addi zero, zero, 0
