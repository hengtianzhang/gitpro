/*
 * Based on arch/arm/include/asm/thread_info.h
 *
 * Copyright (C) 2002 Russell King.
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
#ifndef __ASM_THREAD_INFO_H_
#define __ASM_THREAD_INFO_H_

/*
 *  TIF_SIGPENDING	- signal pending
 *  TIF_NEED_RESCHED	- rescheduling necessary
 */
#define TIF_SIGPENDING		0
#define TIF_NEED_RESCHED	1
#define TIF_FSCHECK			2	/* Check FS is USER_DS on return */
#define TIF_POLLING_NRFLAG	4
#define TIF_SINGLESTEP		21
#define TIF_SVE			23	/* Scalable Vector Extension in use */

#define _TIF_SIGPENDING		(1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED	(1 << TIF_NEED_RESCHED)
#define _TIF_FSCHECK			(1 << TIF_FSCHECK)
#define _TIF_SINGLESTEP			(1 << TIF_SINGLESTEP)
#define _TIF_SVE		(1 << TIF_SVE)

#define _TIF_WORK_MASK		(_TIF_NEED_RESCHED | _TIF_SIGPENDING | _TIF_FSCHECK)

#define _TIF_SYSCALL_WORK	(0)

#ifndef __ASSEMBLY__

#include <base/types.h>

typedef unsigned long mm_segment_t;

/*
 * low level task data that entry.S needs immediate access to.
 */
struct thread_info {
	unsigned long		flags;		/* low level flags */
	mm_segment_t		addr_limit;	/* address limit */
	union {
		u64		preempt_count;	/* 0 => preemptible, <0 => bug */
		struct {
#ifdef CONFIG_CPU_BIG_ENDIAN
			u32	need_resched;
			u32	count;
#else
			u32	count;
			u32	need_resched;
#endif
		} preempt;
	};
};

#define thread_saved_pc(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.pc))
#define thread_saved_sp(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.sp))
#define thread_saved_fp(tsk)	\
	((unsigned long)(tsk->thread.cpu_context.fp))

void arch_setup_new_exec(void);
#define arch_setup_new_exec     arch_setup_new_exec

void arch_release_task_struct(struct task_struct *tsk);

extern int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src);

#define INIT_THREAD_INFO(tsk)						\
{									\
	.flags		= 0,				\
	.preempt_count	= INIT_PREEMPT_COUNT,				\
	.addr_limit	= KERNEL_DS,					\
}

#endif /* !__ASSEMBLY__ */
#endif /* !__ASM_THREAD_INFO_H_ */
