/*
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
#ifndef __ASM_HARDIRQ_H_
#define __ASM_HARDIRQ_H_

#include <rtochius/threads.h>

#include <asm/cache.h>
#include <asm/smp.h>

#define NR_IPI	7

typedef struct {
	unsigned int __softirq_pending;
	unsigned int ipi_irqs[NR_IPI];
} ____cacheline_aligned irq_cpustat_t;

#define __inc_irq_stat(cpu, member)	__IRQ_STAT(cpu, member)++
#define __get_irq_stat(cpu, member)	__IRQ_STAT(cpu, member)

u64 smp_irq_stat_cpu(unsigned int cpu);
#define arch_irq_stat_cpu	smp_irq_stat_cpu

#define __ARCH_IRQ_EXIT_IRQS_DISABLED	1

static inline void ack_bad_irq(unsigned int irq)
{
	extern unsigned long irq_err_count;
	irq_err_count++;
}

#endif /* !__ASM_HARDIRQ_H_ */
