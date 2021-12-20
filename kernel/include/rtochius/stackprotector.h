/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_STACKPROTECTOR_H_
#define __RTOCHIUS_STACKPROTECTOR_H_

#include <base/compiler.h>
#include <base/random.h>

#include <rtochius/sched.h>

#ifdef CONFIG_STACKPROTECTOR
# include <asm/stackprotector.h>
#else
static inline void boot_init_stack_canary(void) {}
#endif /* CONFIG_STACKPROTECTOR */

#ifdef CONFIG_STACKPROTECTOR
extern __visible void __stack_chk_fail(void);
#endif /* CONFIG_STACKPROTECTOR */

#endif /* !__RTOCHIUS_STACKPROTECTOR_H_ */
