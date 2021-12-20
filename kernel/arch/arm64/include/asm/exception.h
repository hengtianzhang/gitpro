/*
 * Based on arch/arm/include/asm/exception.h
 *
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_EXCEPTION_H_
#define __ASM_EXCEPTION_H_

#include <asm/esr.h>
#include <asm/ptrace.h>

#define __exception	__attribute__((section(".exception.text")))

#define __exception_irq_entry	__exception

/*
 * el1/el0 Invalid mode handlers
 */
extern asmlinkage void bad_mode(struct pt_regs *regs,
						int reason,
						unsigned int esr);

/*
 * el0 instr abort handlers
 */
asmlinkage void __exception do_el0_ia_bp_hardening(unsigned long addr,
						   unsigned int esr,
						   struct pt_regs *regs);

/*
 * el0 Data abort handlers
 * el1 Data/instr abort handlers
 */
extern asmlinkage void __exception do_mem_abort(unsigned long addr,
						unsigned int esr,
						struct pt_regs *regs);

/*
 * el1/el0 sp or pc alignment exception handlers
 */
extern asmlinkage void __exception do_sp_pc_abort(unsigned long addr,
					   unsigned int esr,
					   struct pt_regs *regs);

/*
 * el1/el0 Undefined instruction handlers
 */
asmlinkage void __exception do_undefinstr(struct pt_regs *regs);

/*
 * el0 System instructions, for trapped cache maintenance instructions
 */
asmlinkage void __exception do_sysinstr(unsigned int esr,
						struct pt_regs *regs);

/*
 * el1/el0 Debug exception handlers
 */
asmlinkage int __exception do_debug_exception(unsigned long addr,
					      unsigned int esr,
					      struct pt_regs *regs);

/*
 * el0 bad sync
 */
asmlinkage void bad_el0_sync(struct pt_regs *regs,
						int reason,
						unsigned int esr);

/*
 * el1/el0 serror (NMI)
 */
asmlinkage void do_serror(struct pt_regs *regs, unsigned int esr);

/*
 * el0 SVC handlers
 */
asmlinkage void el0_svc_handler(struct pt_regs *regs);

/*
 * el0 fpsimd acc Floating Point or Advanced SIMD access
 */
asmlinkage void do_fpsimd_acc(unsigned int esr,
					struct pt_regs *regs);

/*
 * el0 sve Scalable Vector Extension access
 */
asmlinkage void do_sve_acc(unsigned int esr, struct pt_regs *regs);

/*
 * el0 fpsimd Floating Point, Advanced SIMD or SVE exception
 */
asmlinkage void do_fpsimd_exc(unsigned int esr, struct pt_regs *regs);

#endif /* !__ASM_EXCEPTION_H_ */
