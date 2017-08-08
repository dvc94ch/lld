# Check R_RISCV_* relocations calculation.

# RUN: llvm-mc -filetype=obj -triple=riscv32-unknown-none %s -o %t1.o
# RUN: llvm-mc -filetype=obj -triple=riscv32-unknown-none \
# RUN:     %S/Inputs/riscv-foo.s -o %t2.o
# RUN: ld.lld %t1.o %t2.o -o %t.exe -script %S/Inputs/riscv-foo.script
# RUN: llvm-objdump -d -t -s %t.exe | FileCheck %s

# REQUIRES: riscv

# CHECK: foo_far:
# CHECK: 11111000:	13 00 00 00 	addi	zero, zero, 0
# CHECK: foo_near:
# CHECK: 11113000:	13 00 00 00 	addi	zero, zero, 0

  .section .text.test
	.globl _start
	.extern foo

_start:
	addi t1, t1, 0
# CHECK: _start:
# CHECK: 11113004:	13 03 03 00 	addi	t1, t1, 0

R_RISCV_32:
	.long foo_far
# CHECK: R_RISCV_32:
# CHECK: 11113008:	00 10 11 11  <unknown>

R_RISCV_64:
	.quad foo_far
# CHECK: R_RISCV_64:
# CHECK: 1111300c:	00 10 11 11  <unknown>
# CHECK: 11113010:	00 00 00 00  <unknown>

R_RISCV_HI20:
	lui t1, %hi(foo_far)
# CHECK: R_RISCV_HI20:
# CHECK: 11113014:	37 13 11 11 	lui	t1, 69905

R_RISCV_HI20_OFFSET:
	lui t1, %hi(foo_far+4)
# CHECK: R_RISCV_HI20_OFFSET:
# CHECK: 11113018:	37 13 11 11 	lui	t1, 69905

R_RISCV_LO12_I:
	addi t1, t1, %lo(foo_far)
# CHECK: R_RISCV_LO12_I:
# CHECK: 1111301c:	13 03 03 00 	addi	t1, t1, 0

R_RISCV_LO12_I_OFFSET:
	addi t1, t1, %lo(foo_far+4)
# CHECK: R_RISCV_LO12_I_OFFSET:
# CHECK: 11113020:	13 03 43 00 	addi	t1, t1, 4

R_RISCV_LO12_S:
	sb t1, %lo(foo_far)(a2)
# CHECK: R_RISCV_LO12_S:
# CHECK: 11113024:	23 00 66 00 	sb	t1, 0(a2)

R_RISCV_LO12_S_OFFSET:
	sb t1, %lo(foo_far+4)(a2)
# CHECK: R_RISCV_LO12_S_OFFSET:
# CHECK: 11113028:	23 02 66 00 	sb	t1, 4(a2)


R_RISCV_PCREL_HI20:
	auipc t1, %pcrel_hi(foo_far)
# CHECK: R_RISCV_PCREL_HI20:
# CHECK: 1111302c:	17 e3 ff ff 	auipc	t1, 1048574

R_RISCV_PCREL_HI20_OFFSET:
	auipc t1, %pcrel_hi(foo_far+4)
# CHECK: R_RISCV_PCREL_HI20_OFFSET:
# CHECK: 11113030:	17 e3 ff ff 	auipc	t1, 1048574

R_RISCV_JAL:
	jal zero, foo_near
# CHECK: R_RISCV_JAL:
# CHECK: 11113034:	6f f0 df fc 	jal	zero, -52

R_RISCV_BRANCH:
	bgeu a0, a1, foo_near
# CHECK: R_RISCV_BRANCH:
# CHECK: 11113038:	e3 74 b5 fc 	bgeu	a0, a1, -56
