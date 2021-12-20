/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SCHED_DEBUG_H_
#define __RTOCHIUS_SCHED_DEBUG_H_

/*
 * Various scheduler/task debugging interfaces:
 */
/* Attach to any functions which should be ignored in wchan output. */
#define __sched		__attribute__((__section__(".sched.text")))

/* Linker adds these: start and end of __sched functions */
extern char __sched_text_start[], __sched_text_end[];

/* Is this address in the __sched functions? */
extern int in_sched_functions(unsigned long addr);

extern void show_regs(struct pt_regs *);

#endif /* !__RTOCHIUS_SCHED_DEBUG_H_ */
