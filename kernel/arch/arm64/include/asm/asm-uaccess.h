/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ASM_UACCESS_H_
#define __ASM_ASM_UACCESS_H_

#include <asm/kernel-pgtable.h>
#include <asm/mmu.h>
#include <asm/sysreg.h>
#include <asm/assembler.h>

/*
 * User access enabling/disabling macros.
 */
	.macro	uaccess_ttbr0_disable, tmp1, tmp2
	.endm

	.macro	uaccess_ttbr0_enable, tmp1, tmp2, tmp3
	.endm

/*
 * These macros are no-ops when UAO is present.
 */
	.macro	uaccess_disable_not_uao, tmp1, tmp2
	uaccess_ttbr0_disable \tmp1, \tmp2
	.endm

	.macro	uaccess_enable_not_uao, tmp1, tmp2, tmp3
	uaccess_ttbr0_enable \tmp1, \tmp2, \tmp3
	.endm

	.macro uao_ldp l, reg1, reg2, addr, post_inc
		USER(\l, ldp \reg1, \reg2, [\addr], \post_inc)
	.endm
	.macro uao_stp l, reg1, reg2, addr, post_inc
		USER(\l, stp \reg1, \reg2, [\addr], \post_inc)
	.endm
	.macro uao_user_alternative l, inst, alt_inst, reg, addr, post_inc
		USER(\l, \inst \reg, [\addr], \post_inc)
	.endm

/*
 * Remove the address tag from a virtual address, if present.
 */
	.macro	clear_address_tag, dst, addr
	tst	\addr, #(1 << 55)
	bic	\dst, \addr, #(0xff << 56)
	csel	\dst, \dst, \addr, eq
	.endm

#endif /* !__ASM_ASM_UACCESS_H_ */
