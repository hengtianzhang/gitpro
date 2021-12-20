/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SCHED_TASK_H_
#define __RTOCHIUS_SCHED_TASK_H_

#include <rtochius/sched.h>
#include <rtochius/spinlock.h>

extern struct task_struct init_task;

extern asmlinkage __visible void schedule_tail(struct task_struct *prev);
extern asmlinkage __visible void __sched preempt_schedule_irq(void);

extern int copy_thread(unsigned long, unsigned long, unsigned long,
			struct task_struct *);

/* Architectures that haven't opted into copy_thread_tls get the tls argument
 * via pt_regs, so ignore the tls argument passed via C. */
static inline int copy_thread_tls(
		unsigned long clone_flags, unsigned long sp, unsigned long arg,
		struct task_struct *p, unsigned long tls)
{
	return copy_thread(clone_flags, sp, arg, p);
}

extern void flush_thread(void);

#define get_task_struct(tsk) do { atomic_inc(&(tsk)->usage); } while(0)

extern void __put_task_struct(struct task_struct *t);

static inline void put_task_struct(struct task_struct *t)
{
	if (atomic_dec_and_test(&t->usage))
		__put_task_struct(t);
}

#define arch_task_struct_size (sizeof(struct task_struct))

#ifndef CONFIG_HAVE_ARCH_THREAD_STRUCT_WHITELIST
/*
 * If an architecture has not declared a thread_struct whitelist we
 * must assume something there may need to be copied to userspace.
 */
static inline void arch_thread_struct_whitelist(unsigned long *offset,
						unsigned long *size)
{
	*offset = 0;
	/* Handle dynamically sized thread_struct. */
	*size = arch_task_struct_size - offsetof(struct task_struct, thread);
}
#endif

/*
 * Protects ->fs, ->files, ->mm, ->group_info, ->comm, keyring
 * subscriptions and synchronises with wait4().  Also used in procfs.  Also
 * pins the final release of task.io_context.  Also protects ->cpuset and
 * ->cgroup.subsys[]. And ->vfork_done.
 *
 * Nests both inside and outside of read_lock(&tasklist_lock).
 * It must not be nested with write_lock_irq(&tasklist_lock),
 * neither inside nor outside.
 */
static inline void task_lock(struct task_struct *p)
{
	spin_lock(&p->alloc_lock);
}

static inline void task_unlock(struct task_struct *p)
{
	spin_unlock(&p->alloc_lock);
}

#endif /* !__RTOCHIUS_SCHED_TASK_H_ */
