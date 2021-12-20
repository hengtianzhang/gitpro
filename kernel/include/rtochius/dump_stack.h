/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_DUMP_STACK_H_
#define __RTOCHIUS_DUMP_STACK_H_

#include <base/compiler.h>

__printf(1, 2) void dump_stack_set_arch_desc(const char *fmt, ...);

void dump_stack_print_info(const char *log_lvl);
extern void show_regs_print_info(const char *log_lvl);

#endif /* !__RTOCHIUS_DUMP_STACK_H_ */
