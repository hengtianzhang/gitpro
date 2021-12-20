/*
 *  linux/kernel/fork.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
#define pr_fmt(fmt) "fork: " fmt

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also entry.S and others).
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/memory.c': 'copy_page_range()'
 */
#include <rtochius/sched.h>
#include <rtochius/sched/task.h>
#include <rtochius/sched/task_stack.h>

void set_task_stack_end_magic(struct task_struct *tsk)
{
	unsigned long *stackend;

	stackend = end_of_stack(tsk);
	*stackend = STACK_END_MAGIC;	/* for overflow detection */
}

static void release_task_stack(struct task_struct *tsk)
{
}

void put_task_stack(struct task_struct *tsk)
{
	if (atomic_dec_and_test(&tsk->stack_refcount))
		release_task_stack(tsk);
}

void thread_stack_cache_init(void)
{

}

void __put_task_struct(struct task_struct *tsk)
{
}

int __weak arch_dup_task_struct(struct task_struct *dst,
					       struct task_struct *src)
{
	*dst = *src;
	return 0;
}
