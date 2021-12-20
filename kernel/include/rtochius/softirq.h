#ifndef __RTOCHIUS_SOFTIRQ_H_
#define __RTOCHIUS_SOFTIRQ_H_

#include <base/compiler.h>

#include <rtochius/preempt.h>
#include <rtochius/percpu.h>

#include <asm/hardirq.h>

DECLARE_PER_CPU_ALIGNED(irq_cpustat_t, irq_stat);	/* defined in asm/hardirq.h */
#define __IRQ_STAT(cpu, member)	(per_cpu(irq_stat.member, cpu))

#ifndef local_softirq_pending
#ifndef local_softirq_pending_ref
#define local_softirq_pending_ref irq_stat.__softirq_pending
#endif
#define local_softirq_pending()	(__this_cpu_read(local_softirq_pending_ref))
#define set_softirq_pending(x)	(__this_cpu_write(local_softirq_pending_ref, (x)))
#define or_softirq_pending(x)	(__this_cpu_or(local_softirq_pending_ref, (x)))
#endif /* local_softirq_pending */

struct softirq_action {
	void	(*action)(struct softirq_action *);
};

enum {
	HI_SOFTIRQ = 0,
	TIMER_SOFTIRQ,
	SCHED_SOFTIRQ,
	HRTIMER_SOFTIRQ, /* Unused, but kept as tools rely on the
			    numbering. Sigh! */
	NR_SOFTIRQS
};

/* map softirq index to softirq name. update 'softirq_to_name' in
 * kernel/softirq.c when adding a new softirq.
 */
extern const char * const softirq_to_name[NR_SOFTIRQS];

extern void _local_bh_enable(void);
extern void __local_bh_enable_ip(unsigned long ip, unsigned int cnt);


static inline void local_bh_enable_ip(unsigned long ip)
{
	__local_bh_enable_ip(ip, SOFTIRQ_DISABLE_OFFSET);
}

static inline void local_bh_enable(void)
{
	__local_bh_enable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

static __always_inline void __local_bh_disable_ip(unsigned long ip, unsigned int cnt)
{
	preempt_count_add(cnt);
	barrier();
}

static inline void local_bh_disable(void)
{
	__local_bh_disable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

asmlinkage void do_softirq(void);
asmlinkage void __do_softirq(void);

static inline void do_softirq_own_stack(void)
{
	__do_softirq();
}

extern void __raise_softirq_irqoff(unsigned int nr);

extern void raise_softirq_irqoff(unsigned int nr);
extern void raise_softirq(unsigned int nr);

extern void open_softirq(int nr, void (*action)(struct softirq_action *));

#endif /* !__RTOCHIUS_SOFTIRQ_H_ */
