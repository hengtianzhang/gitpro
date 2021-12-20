/*
 * FP/SIMD context switching and fault handling
 *
 * Copyright (C) 2012 ARM Ltd.
 * Author: Catalin Marinas <catalin.marinas@arm.com>
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
#include <base/common.h>

#include <rtochius/threads.h>
#include <rtochius/sched.h>

#include <asm/fpsimd.h>
#include <asm/sysreg.h>
#include <asm/cpufeature.h>
#include <asm/exception.h>

/*
 * Enable SVE for EL1.
 * Intended for use by the cpufeatures code during CPU boot.
 */
void sve_kernel_enable(const struct arm64_cpu_capabilities *__always_unused p)
{
	write_sysreg(read_sysreg(CPACR_EL1) | CPACR_EL1_ZEN_EL1EN, CPACR_EL1);
	isb();
}

/*
 * Read the pseudo-ZCR used by cpufeatures to identify the supported SVE
 * vector length.
 *
 * Use only if SVE is present.
 * This function clobbers the SVE vector length.
 */
u64 read_zcr_features(void)
{
	u64 zcr;
	unsigned int vq_max  = 0;

	/*
	 * Set the maximum possible VL, and write zeroes to all other
	 * bits to see if they stick.
	 */
	sve_kernel_enable(NULL);
	write_sysreg_s(ZCR_ELx_LEN_MASK, SYS_ZCR_EL1);

	zcr = read_sysreg_s(SYS_ZCR_EL1);
	zcr &= ~(u64)ZCR_ELx_LEN_MASK; /* find sticky 1s outside LEN field */
	//vq_max = sve_vq_from_vl(sve_get_vl());
	zcr |= vq_max - 1; /* set LEN field to maximum effective value */

	return zcr;
}

void fpsimd_flush_thread(void)
{
}

/*
 * Invalidate live CPU copies of task t's FPSIMD state
 */
void fpsimd_flush_task_state(struct task_struct *t)
{
	t->thread.fpsimd_cpu = NR_CPUS;
}

void fpsimd_thread_switch(struct task_struct *next)
{

}

/*
 * Trapped FP/ASIMD access.
 */
asmlinkage void do_fpsimd_acc(unsigned int esr, struct pt_regs *regs)
{
	/* TODO: implement lazy context saving/restoring */
	WARN_ON(1);
}

asmlinkage void do_sve_acc(unsigned int esr, struct pt_regs *regs)
{
	panic("el0 sve_acc sync\n");
}

/*
 * Raise a SIGFPE for the current process.
 */
asmlinkage void do_fpsimd_exc(unsigned int esr, struct pt_regs *regs)
{
	panic("el0 fpsimd_exc sync\n");
}
