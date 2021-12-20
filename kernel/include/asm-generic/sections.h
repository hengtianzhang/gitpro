/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_GENERIC_SECTIONS_H_
#define __ASM_GENERIC_SECTIONS_H_

/* References to section boundaries */

#include <base/compiler.h>
#include <base/types.h>

extern char _text[], _stext[], _etext[];
extern char _data[], _sdata[], _edata[];
extern char __bss_start[], __bss_stop[];
extern char __init_begin[], __init_end[];
extern char _sinittext[], _einittext[];
extern char __start_ro_after_init[], __end_ro_after_init[];
extern char _end[];
extern char __per_cpu_load[], __per_cpu_start[], __per_cpu_end[];
extern char __entry_text_start[], __entry_text_end[];
extern char __start_rodata[], __end_rodata[];
extern char __irqentry_text_start[], __irqentry_text_end[];
extern char __softirqentry_text_start[], __softirqentry_text_end[];
extern char __start_once[], __end_once[];
extern char __start_notes[], __stop_notes[];

extern char __start_archive[], __end_archive[];
extern char __start_archive_drivers[], __end_archive_drivers[];

#endif /* !__ASM_GENERIC_SECTIONS_H_ */
