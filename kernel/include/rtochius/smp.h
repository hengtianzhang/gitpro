/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SMP_H_
#define __RTOCHIUS_SMP_H_

#include <base/llist.h>
#include <base/list.h>

/*
 *	Generic SMP support
 *		Alan Cox. <alan@redhat.com>
 */
#include <asm/smp.h>

typedef void (*smp_call_func_t)(void *info);
struct __call_single_data {
	struct llist_node llist;
	smp_call_func_t func;
	void *info;
	unsigned int flags;
};

/* Use __aligned() to avoid to use 2 cache lines for 1 csd */
typedef struct __call_single_data call_single_data_t
	__aligned(sizeof(struct __call_single_data));

int smp_call_function_single(int cpuid, smp_call_func_t func, void *info,
			     int wait);

/*
 * Call a function on all processors
 */
int on_each_cpu(smp_call_func_t func, void *info, int wait);

/*
 * Call a function on processors specified by mask, which might include
 * the local one.
 */
void on_each_cpu_mask(const struct cpumask *mask, smp_call_func_t func,
		void *info, bool wait);

int smp_call_function_single_async(int cpu, call_single_data_t *csd);

/*
 * main cross-CPU interfaces, handles INIT, TLB flush, STOP, etc.
 * (defined in asm header):
 */

/*
 * stops all CPUs but the current one:
 */
extern void smp_send_stop(void);

/*
 * sends a 'reschedule' event to another CPU:
 */
extern void smp_send_reschedule(int cpu);

/*
 * Prepare machine for booting other CPUs.
 */
extern void smp_prepare_cpus(unsigned int max_cpus);

/*
 * Bring a CPU up
 */
extern int __cpu_up(unsigned int cpunum, struct task_struct *tidle);

/*
 * Final polishing of CPUs
 */
extern void smp_cpus_done(unsigned int max_cpus);

/*
 * Call a function on all other processors
 */
int smp_call_function(smp_call_func_t func, void *info, int wait);
void smp_call_function_many(const struct cpumask *mask,
			    smp_call_func_t func, void *info, bool wait);

int smp_call_function_any(const struct cpumask *mask,
			  smp_call_func_t func, void *info, int wait);

void kick_all_cpus_sync(void);

/*
 * Generic and arch helpers
 */
void call_function_init(void);
void generic_smp_call_function_single_interrupt(void);
#define generic_smp_call_function_interrupt \
	generic_smp_call_function_single_interrupt

/*
 * Mark the boot cpu "online" so that it can call console drivers in
 * printk() and can access its per-cpu storage.
 */
void smp_prepare_boot_cpu(void);

extern void setup_nr_cpu_ids(void);
extern void smp_init(void);

extern int __boot_cpu_id;

static inline int get_boot_cpu_id(void)
{
	return __boot_cpu_id;
}

#define smp_processor_id() raw_smp_processor_id()

#define get_cpu()		({ preempt_disable(); smp_processor_id(); })
#define put_cpu()		preempt_enable()

void smp_setup_processor_id(void);

/* SMP core functions */
int smpcfd_prepare_cpu(unsigned int cpu);

#endif /* __RTOCHIUS_SMP_H_ */
