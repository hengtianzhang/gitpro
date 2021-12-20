/*
 * Contains CPU specific errata definitions
 *
 * Copyright (C) 2014 ARM Ltd.
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
#include <base/types.h>

#include <rtochius/arm-smccc.h>

#include <asm/cpu.h>
#include <asm/cputype.h>
#include <asm/cpufeature.h>
#include <asm/cache.h>

static bool
has_mismatched_cache_type(const struct arm64_cpu_capabilities *entry,
			  int scope)
{
	u64 mask = arm64_ftr_reg_ctrel0.strict_mask;
	u64 sys = arm64_ftr_reg_ctrel0.sys_val & mask;
	u64 ctr_raw, ctr_real;

	WARN_ON(scope != SCOPE_LOCAL_CPU || preemptible());

	/*
	 * We want to make sure that all the CPUs in the system expose
	 * a consistent CTR_EL0 to make sure that applications behaves
	 * correctly with migration.
	 *
	 * If a CPU has CTR_EL0.IDC but does not advertise it via CTR_EL0 :
	 *
	 * 1) It is safe if the system doesn't support IDC, as CPU anyway
	 *    reports IDC = 0, consistent with the rest.
	 *
	 * 2) If the system has IDC, it is still safe as we trap CTR_EL0
	 *    access on this CPU via the ARM64_HAS_CACHE_IDC capability.
	 *
	 * So, we need to make sure either the raw CTR_EL0 or the effective
	 * CTR_EL0 matches the system's copy to allow a secondary CPU to boot.
	 */
	ctr_raw = read_cpuid_cachetype() & mask;
	ctr_real = read_cpuid_effective_cachetype() & mask;

	return (ctr_real != sys) && (ctr_raw != sys);
}

static void
cpu_enable_trap_ctr_access(const struct arm64_cpu_capabilities *__unused)
{
	u64 mask = arm64_ftr_reg_ctrel0.strict_mask;

	/* Trap CTR_EL0 access on this CPU, only if it has a mismatch */
	if ((read_cpuid_cachetype() & mask) !=
	    (arm64_ftr_reg_ctrel0.sys_val & mask))
		sysreg_clear_set(sctlr_el1, SCTLR_EL1_UCT, 0);
}

const struct arm64_cpu_capabilities arm64_errata[] = {
	{
		.desc = "Mismatched cache type (CTR_EL0)",
		.capability = ARM64_MISMATCHED_CACHE_TYPE,
		.matches = has_mismatched_cache_type,
		.type = ARM64_CPUCAP_LOCAL_CPU_ERRATUM,
		.cpu_enable = cpu_enable_trap_ctr_access,
	},
	{
	}
};
