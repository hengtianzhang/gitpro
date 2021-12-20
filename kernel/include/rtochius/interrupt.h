/* SPDX-License-Identifier: GPL-2.0 */
/* interrupt.h */
#ifndef __RTOCHIUS_INTERRUPT_H_
#define __RTOCHIUS_INTERRUPT_H_

#include <rtochius/cpumask.h>

/*
 * We want to know which function is an entrypoint of a hardirq or a softirq.
 */
#define __irq_entry		 __attribute__((__section__(".irqentry.text")))
#define __softirq_entry  \
	__attribute__((__section__(".softirqentry.text")))

/*
 * It is safe to do non-atomic ops on ->hardirq_context,
 * because NMI handlers may not preempt and the ops are
 * always balanced, so the interrupted value of ->hardirq_context
 * will always be restored.
 */
#define __irq_enter()					\
	do {						\
		preempt_count_add(HARDIRQ_OFFSET);	\
	} while (0)

/*
 * Enter irq context (on NO_HZ, update jiffies):
 */
extern void irq_enter(void);
extern void irq_exit(void);

extern cpumask_var_t irq_default_affinity;


/**
 * struct irq_affinity_desc - Interrupt affinity descriptor
 * @mask:	cpumask to hold the affinity assignment
 * @is_managed: 1 if the interrupt is managed internally
 */
struct irq_affinity_desc {
	struct cpumask	mask;
	unsigned int	is_managed : 1;
};

#endif /* !__RTOCHIUS_INTERRUPT_H_ */
