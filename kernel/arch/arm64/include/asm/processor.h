/*
 * Based on arch/arm/include/asm/processor.h
 *
 * Copyright (C) 1995-1999 Russell King
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
#ifndef __ASM_PROCESSOR_H_
#define __ASM_PROCESSOR_H_

#include <base/const.h>

#define KERNEL_DS		UL(-1)
#define USER_DS			((UL(1) << MAX_USER_VA_BITS) - 1)

#ifndef __ASSEMBLY__

#include <base/string.h>
#include <base/init.h>

#include <rtochius/threads.h>

#include <asm/cpufeature.h>
#include <asm/cache.h>
#include <asm/ptrace.h>

#define DEFAULT_MAP_WINDOW_64	(UL(1) << VA_BITS)
#define TASK_SIZE_64		(UL(1) << vabits_user)

#define TASK_SIZE		TASK_SIZE_64

#define STACK_TOP_MAX		DEFAULT_MAP_WINDOW_64

#define STACK_TOP		STACK_TOP_MAX

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct debug_info {
};

struct thread_struct {
	struct cpu_context	cpu_context;	/* cpu context */

	/*
	 * Whitelisted fields for hardened usercopy:
	 * Maintainers must ensure manually that this contains no
	 * implicit padding.
	 */
	struct {
		unsigned long	tp_value;	/* TLS register */
		unsigned long	tp2_value;
		struct user_fpsimd_state fpsimd_state;
	} uw;

	unsigned int		fpsimd_cpu;
	void			*sve_state;	/* SVE registers, if any */
	unsigned int		sve_vl;		/* SVE vector length */
	unsigned int		sve_vl_onexec;	/* SVE vl after next exec */
	unsigned long		fault_address;	/* fault info */
	unsigned long		fault_code;	/* ESR_EL1 value */
	struct debug_info	debug;		/* debugging */
};

static inline void arch_thread_struct_whitelist(unsigned long *offset,
						unsigned long *size)
{
	/* Verify that there is no padding among the whitelisted fields: */
	BUILD_BUG_ON(sizeof_field(struct thread_struct, uw) !=
		     sizeof_field(struct thread_struct, uw.tp_value) +
		     sizeof_field(struct thread_struct, uw.tp2_value) +
		     sizeof_field(struct thread_struct, uw.fpsimd_state));

	*offset = offsetof(struct thread_struct, uw);
	*size = sizeof_field(struct thread_struct, uw);
}

#define task_user_tls(t)	(&(t)->thread.uw.tp_value)

/* Sync TPIDR_EL0 back to thread_struct for current */
void tls_preserve_current_state(void);

#define INIT_THREAD {				\
	.fpsimd_cpu = NR_CPUS,			\
}

static inline void start_thread_common(struct pt_regs *regs, unsigned long pc)
{
	memset(regs, 0, sizeof(*regs));
	forget_syscall(regs);
	regs->pc = pc;
}

static inline void start_thread(struct pt_regs *regs, unsigned long pc,
				unsigned long sp)
{
	start_thread_common(regs, pc);
	regs->pstate = PSR_MODE_EL0t;

	if (arm64_get_ssbd_state() != ARM64_SSBD_FORCE_ENABLE)
		regs->pstate |= PSR_SSBS_BIT;

	regs->sp = sp;
}

/* Forward declaration, a strange C thing */
struct task_struct;

/* Free all resources held by a thread. */
extern void release_thread(struct task_struct *);

unsigned long get_wchan(struct task_struct *p);

static inline void cpu_relax(void)
{
	asm volatile("yield" ::: "memory");
}

/* Thread switching */
extern struct task_struct *cpu_switch_to(struct task_struct *prev,
					 struct task_struct *next);

#define task_pt_regs(p) \
	((struct pt_regs *)(THREAD_SIZE + task_stack_page(p)) - 1)

#define current_pt_regs() task_pt_regs(current)

/*
 * Prefetching support
 */
#define ARCH_HAS_PREFETCH
static inline void prefetch(const void *ptr)
{
	asm volatile("prfm pldl1keep, %a0\n" : : "p" (ptr));
}

#define ARCH_HAS_PREFETCHW
static inline void prefetchw(const void *ptr)
{
	asm volatile("prfm pstl1keep, %a0\n" : : "p" (ptr));
}

#define ARCH_HAS_SPINLOCK_PREFETCH
static inline void spin_lock_prefetch(const void *ptr)
{
	asm volatile("prfm pstl1strm, %a0" : : "p" (ptr));
}

extern unsigned long __ro_after_init signal_minsigstksz; /* sigframe size */
extern void __init minsigstksz_setup(void);

/*
 * work pending _TIF_FLAGS
 */
extern asmlinkage void do_notify_resume(struct pt_regs *regs,
				 unsigned long thread_flags);

#endif /* !__ASSEMBLY__ */
#endif /* !__ASM_PROCESSOR_H_ */
