/* CPU control.
 * (C) 2001, 2002, 2003, 2004 Rusty Russell
 *
 * This code is licenced under the GPL.
 */
#define pr_fmt(fmt) "cpu: " fmt

#include <base/cache.h>
#include <base/init.h>
#include <base/list.h>

#include <rtochius/cpu.h>
#include <rtochius/smp.h>
#include <rtochius/cpumask.h>
#include <rtochius/device.h>

struct cpumask __cpu_possible_mask __read_mostly;
struct cpumask __cpu_present_mask __read_mostly;
struct cpumask __cpu_online_mask __read_mostly;

/*
 * cpu_bit_bitmap[] is a special, "compressed" data structure that
 * represents all NR_CPUS bits binary values of 1<<nr.
 *
 * It is used by cpumask_of() to get a constant address to a CPU
 * mask value that has a single bit set only.
 */

/* cpu_bit_bitmap[0] is empty - so we can back into it */
#define MASK_DECLARE_1(x)	[x+1][0] = (1UL << (x))
#define MASK_DECLARE_2(x)	MASK_DECLARE_1(x), MASK_DECLARE_1(x+1)
#define MASK_DECLARE_4(x)	MASK_DECLARE_2(x), MASK_DECLARE_2(x+2)
#define MASK_DECLARE_8(x)	MASK_DECLARE_4(x), MASK_DECLARE_4(x+4)

const unsigned long cpu_bit_bitmap[BITS_PER_LONG+1][BITS_TO_LONGS(NR_CPUS)] = {

	MASK_DECLARE_8(0),	MASK_DECLARE_8(8),
	MASK_DECLARE_8(16),	MASK_DECLARE_8(24),
#if BITS_PER_LONG > 32
	MASK_DECLARE_8(32),	MASK_DECLARE_8(40),
	MASK_DECLARE_8(48),	MASK_DECLARE_8(56),
#endif
};

int __boot_cpu_id;

/*
 * Activate the first processor.
 */
void __init boot_cpu_init(void)
{
	int cpu = smp_processor_id();

	reset_cpu_all_mask();

	/* Mark the boot cpu "present", "online" etc for SMP and UP case */
	set_cpu_possible(cpu, true);
	set_cpu_present(cpu, true);
	set_cpu_online(cpu, true);

	__boot_cpu_id = cpu;
}

static DEFINE_PER_CPU(struct device *, cpu_sys_devices);

struct device *get_cpu_device(unsigned cpu)
{
	if (cpu < nr_cpu_ids && cpu_possible(cpu))
		return per_cpu(cpu_sys_devices, cpu);
	else
		return NULL;
}

/*
 * register_cpu - Setup a sysfs device for a CPU.
 * @cpu - cpu->hotpluggable field set to 1 will generate a control file in
 *	  sysfs for this CPU.
 * @num - CPU number to use when creating the device.
 *
 * Initialize and register the CPU device.
 */
int register_cpu(struct cpu *cpu, int num)
{
	cpu->node_id = num;
	memset(&cpu->dev, 0x00, sizeof(struct device));
	cpu->dev.of_node = of_get_cpu_node(num, NULL);

	per_cpu(cpu_sys_devices, num) = &cpu->dev;

	return 0;
}

struct cpuhp_step {
	const char		*name;
	union {
		int		(*single)(unsigned int cpu);
		int		(*multi)(unsigned int cpu,
					 struct hlist_node *node);
	} startup;
	union {
		int		(*single)(unsigned int cpu);
		int		(*multi)(unsigned int cpu,
					 struct hlist_node *node);
	} teardown;
	struct hlist_head	list;
	bool			cant_stop;
	bool			multi_instance;
};

static struct cpuhp_step cpuhp_hp_states[] = {
	[CPUHP_OFFLINE] = {
		.name			= "offline",
		.startup.single		= NULL,
		.teardown.single	= NULL,
	},
	[CPUHP_AP_IRQ_GIC_STARTING] = {
		.name 			= "gic-irq",
		.startup.single 	= NULL,
	},
	[CPUHP_AP_ARM_ARCH_TIMER_STARTING] = {
		.name 			= "arm-timer",
		.startup.single 	= NULL,
	},
	[CPUHP_HRTIMERS_PREPARE] = {
		.name 			= "hrtimer-pre",
		.startup.single 	= NULL,
	},
	/* CPU is fully up and running. */
	[CPUHP_ONLINE] = {
		.name			= "online",
		.startup.single		= NULL,
		.teardown.single	= NULL,
	},
};

static struct cpuhp_step *cpuhp_get_step(enum cpuhp_state state)
{
	return cpuhp_hp_states + state;
}

int __cpuhp_setup_state(enum cpuhp_state state,
			const char *name, bool invoke,
			int (*startup)(unsigned int cpu),
			int (*teardown)(unsigned int cpu),
			bool multi_instance)
{
	/* (Un)Install the callbacks for further cpu hotplug operations */
	struct cpuhp_step *sp;

	if (state >= CPUHP_ONLINE)
		return -1;

	if (!startup)
		return -1;

	sp = cpuhp_get_step(state);

	sp->startup.single = startup;
	sp->teardown.single = teardown;
	sp->multi_instance = multi_instance;

	return 0;
}

/**
 * notify_cpu_starting(cpu) - Invoke the callbacks on the starting CPU
 * @cpu: cpu that just started
 *
 * It must be called by the arch code on the new cpu, before the new cpu
 * enables interrupts and before the "boot" cpu returns from __cpu_up().
 */
void notify_cpu_starting(unsigned int cpu)
{
	enum cpuhp_state state;

	for (state = CPUHP_OFFLINE; state < CPUHP_ONLINE; state++) {
		struct cpuhp_step *cs = cpuhp_get_step(state);
		if (cs->startup.single) {
			if (cs->startup.single(cpu))
				pr_info("Cpu%d starting %s Failed!\n", cpu, cs->name);
		}
	}
}
