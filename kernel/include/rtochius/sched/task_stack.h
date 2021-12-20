/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SCHED_TASK_STACK_H_
#define __RTOCHIUS_SCHED_TASK_STACK_H_

#include <rtochius/sched.h>
#include <rtochius/memory.h>

#include <uapi/rtochius/magic.h>

extern struct task_struct init_task;

static inline void *task_stack_page(const struct task_struct *tsk)
{
	return tsk->stack;
}

static inline unsigned long *end_of_stack(const struct task_struct *tsk)
{
	return tsk->stack;
}

static inline void *try_get_task_stack(struct task_struct *tsk)
{
	return atomic_inc_not_zero(&tsk->stack_refcount) ?
		task_stack_page(tsk) : NULL;
}

extern void put_task_stack(struct task_struct *tsk);

#define task_stack_end_corrupted(task) \
		(*(end_of_stack(task)) != STACK_END_MAGIC)

static inline int object_is_on_stack(const void *obj)
{
	void *stack = task_stack_page(current);

	return (obj >= stack) && (obj < (stack + THREAD_SIZE));
}

extern void thread_stack_cache_init(void);

extern void set_task_stack_end_magic(struct task_struct *tsk);

extern unsigned long arch_align_stack(unsigned long sp);

#endif /* !__RTOCHIUS_SCHED_TASK_STACK_H_ */
