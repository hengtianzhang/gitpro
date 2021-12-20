/*
 * Based on arch/arm/kernel/process.c
 *
 * Original Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 1996-2000 Russell King - Converted to ARM.
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
#include <rtochius/reboot.h>
#include <rtochius/sched.h>
#include <rtochius/sched/task_stack.h>
#include <rtochius/sched/task.h>
#include <rtochius/thread_info.h>
#include <rtochius/mm_types.h>
#include <rtochius/percpu.h>
#include <rtochius/printf.h>
#include <rtochius/dump_stack.h>
#include <rtochius/cpu.h>

#include <asm-generic/switch_to.h>

#include <asm/system_misc.h>
#include <asm/fpsimd.h>
#include <asm/stacktrace.h>
#include <asm/ptrace.h>
#include <asm/mmu_context.h>

#if defined(CONFIG_STACKPROTECTOR)
#include <rtochius/stackprotector.h>
unsigned long __stack_chk_guard __read_mostly;
#endif

void (*pm_power_off)(void);

void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);

/*
 * This is our default idle handler.
 */
void arch_cpu_idle(void)
{
	/*
	 * This should do all the clock switching and wait for interrupt
	 * tricks
	 */
	cpu_do_idle();
	local_irq_enable();
}

/*
 * Halting simply requires that the secondary CPUs stop performing any
 * activity (executing tasks, handling interrupts). smp_send_stop()
 * achieves this.
 */
void machine_halt(void)
{
	local_irq_disable();
	smp_send_stop();
	while (1);
}

/*
 * Power-off simply requires that the secondary CPUs stop performing any
 * activity (executing tasks, handling interrupts). smp_send_stop()
 * achieves this. When the system power is turned off, it will take all CPUs
 * with it.
 */
void machine_power_off(void)
{
	local_irq_disable();
	smp_send_stop();
	if (pm_power_off)
		pm_power_off();
}

static void print_pstate(struct pt_regs *regs)
{
	u64 pstate = regs->pstate;

	printf("pstate: %08llx (%c%c%c%c %c%c%c%c %cPAN %cUAO)\n",
		pstate,
		pstate & PSR_N_BIT ? 'N' : 'n',
		pstate & PSR_Z_BIT ? 'Z' : 'z',
		pstate & PSR_C_BIT ? 'C' : 'c',
		pstate & PSR_V_BIT ? 'V' : 'v',
		pstate & PSR_D_BIT ? 'D' : 'd',
		pstate & PSR_A_BIT ? 'A' : 'a',
		pstate & PSR_I_BIT ? 'I' : 'i',
		pstate & PSR_F_BIT ? 'F' : 'f',
		pstate & PSR_PAN_BIT ? '+' : '-',
		pstate & PSR_UAO_BIT ? '+' : '-');
}

void __show_regs(struct pt_regs *regs)
{
	int i, top_reg;
	u64 lr, sp;

	lr = regs->regs[30];
	sp = regs->sp;
	top_reg = 29;

	show_regs_print_info(KERN_DEFAULT);
	print_pstate(regs);

	if (!user_mode(regs)) {
		printf("pc : %pS\n", (void *)regs->pc);
		printf("lr : %pS\n", (void *)lr);
	} else {
		printf("pc : %016llx\n", regs->pc);
		printf("lr : %016llx\n", lr);
	}

	printf("sp : %016llx\n", sp);

	i = top_reg;

	while (i >= 0) {
		printf("x%-2d: %016llx ", i, regs->regs[i]);
		i--;

		if (i % 2 == 0) {
			pr_cont("x%-2d: %016llx ", i, regs->regs[i]);
			i--;
		}

		pr_cont("\n");
	}
}

void show_regs(struct pt_regs * regs)
{
	__show_regs(regs);
	dump_backtrace(regs, NULL);
}

static void tls_thread_flush(void)
{
	write_sysreg(0, tpidr_el0);
}

void flush_thread(void)
{
	fpsimd_flush_thread();
	tls_thread_flush();
}

void release_thread(struct task_struct *dead_task)
{
}

void arch_release_task_struct(struct task_struct *tsk)
{
}

/*
 * src and dst may temporarily have aliased sve_state after task_struct
 * is copied.  We cannot fix this properly here, because src may have
 * live SVE state and dst's thread_info may not exist yet, so tweaking
 * either src's or dst's TIF_SVE is not safe.
 *
 * The unaliasing is done in copy_thread() instead.  This works because
 * dst is not schedulable or traceable until both of these functions
 * have been called.
 */
int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src)
{
	*dst = *src;

	return 0;
}

asmlinkage void ret_from_fork(void) asm("ret_from_fork");

int copy_thread(unsigned long clone_flags, unsigned long stack_start,
		unsigned long stk_sz, struct task_struct *p)
{
	struct pt_regs *childregs = task_pt_regs(p);

	memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));

	/*
	 * Unalias p->thread.sve_state (if any) from the parent task
	 * and disable discard SVE state for p:
	 */
	clear_tsk_thread_flag(p, TIF_SVE);
	p->thread.sve_state = NULL;

	/*
	 * In case p was allocated the same task_struct pointer as some
	 * other recently-exited task, make sure p is disassociated from
	 * any cpu that may have run that now-exited task recently.
	 * Otherwise we could erroneously skip reloading the FPSIMD
	 * registers for p.
	 */
	fpsimd_flush_task_state(p);

	if (likely(!(p->flags & PF_KTHREAD))) {
		*childregs = *current_pt_regs();
		childregs->regs[0] = 0;

		/*
		 * Read the current TLS pointer from tpidr_el0 as it may be
		 * out-of-sync with the saved value.
		 */
		*task_user_tls(p) = read_sysreg(tpidr_el0);

		if (stack_start) {
			childregs->sp = stack_start;
		}

		/*
		 * If a TLS pointer was passed to clone (4th argument), use it
		 * for the new thread.
		 */
		if (clone_flags & CLONE_SETTLS)
			p->thread.uw.tp_value = childregs->regs[3];
	} else {
		memset(childregs, 0, sizeof(struct pt_regs));
		childregs->pstate = PSR_MODE_EL1h;

		if (arm64_get_ssbd_state() == ARM64_SSBD_FORCE_DISABLE)
			childregs->pstate |= PSR_SSBS_BIT;

		p->thread.cpu_context.x19 = stack_start;
		p->thread.cpu_context.x20 = stk_sz;
	}
	p->thread.cpu_context.pc = (unsigned long)ret_from_fork;
	p->thread.cpu_context.sp = (unsigned long)childregs;

	return 0;
}

void tls_preserve_current_state(void)
{
	*task_user_tls(current) = read_sysreg(tpidr_el0);
}

static void tls_thread_switch(struct task_struct *next)
{
	tls_preserve_current_state();

	write_sysreg(0, tpidrro_el0);

	write_sysreg(*task_user_tls(next), tpidr_el0);
}

/*
 * We store our current task in sp_el0, which is clobbered by userspace. Keep a
 * shadow copy so that we can restore this upon entry from userspace.
 *
 * This is *only* for exception entry from EL0, and is not valid until we
 * __switch_to() a user task.
 */
DEFINE_PER_CPU(struct task_struct *, __entry_task);

static void entry_task_switch(struct task_struct *next)
{
	__this_cpu_write(__entry_task, next);
}

/*
 * Thread switching.
 */
struct task_struct *__switch_to(struct task_struct *prev,
				struct task_struct *next)
{
	struct task_struct *last;

	fpsimd_thread_switch(next);
	tls_thread_switch(next);
	contextidr_thread_switch(next);
	entry_task_switch(next);

	/*
	 * Complete any pending TLB or cache maintenance on this CPU in case
	 * the thread migrates to a different CPU.
	 * This full barrier is also required by the membarrier system
	 * call.
	 */
	dsb(ish);

	/* the actual thread switch */
	last = cpu_switch_to(prev, next);

	return last;
}

unsigned long get_wchan(struct task_struct *p)
{
	struct stackframe frame;
	unsigned long stack_page, ret = 0;
	int count = 0;
	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;

	stack_page = (unsigned long)try_get_task_stack(p);
	if (!stack_page)
		return 0;

	frame.fp = thread_saved_fp(p);
	frame.pc = thread_saved_pc(p);

	do {
		if (unwind_frame(p, &frame))
			goto out;
		if (!in_sched_functions(frame.pc)) {
			ret = frame.pc;
			goto out;
		}
	} while (count ++ < 16);

out:
	put_task_stack(p);
	return ret;
}

unsigned long arch_align_stack(unsigned long sp)
{
	return sp & ~0xf;
}

/*
 * Called from setup_new_exec() after (COMPAT_)SET_PERSONALITY.
 */
void arch_setup_new_exec(void)
{
	current->mm->context.flags = 0;
}
