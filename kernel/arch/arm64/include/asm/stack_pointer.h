/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_STACK_POINTER_H_
#define __ASM_STACK_POINTER_H_

/*
 * how to get the current stack pointer from C
 */
register unsigned long current_stack_pointer asm ("sp");

#endif /* !__ASM_STACK_POINTER_H_ */
