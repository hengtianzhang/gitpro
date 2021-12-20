/*
 * Common SMP CPU bringup/teardown functions
 */
#include <rtochius/smpboot.h>

struct smpboot_thread_data {
	unsigned int			cpu;
	unsigned int			status;
	struct smp_hotplug_thread	*ht;
};

/**
 * smpboot_register_percpu_thread - Register a per_cpu thread related
 * 					    to hotplug
 * @plug_thread:	Hotplug thread descriptor
 *
 * Creates and starts the threads on all online cpus.
 */
int smpboot_register_percpu_thread(struct smp_hotplug_thread *plug_thread)
{
	return 0;
}

/**
 * smpboot_unregister_percpu_thread - Unregister a per_cpu thread related to hotplug
 * @plug_thread:	Hotplug thread descriptor
 *
 * Stops all threads on all possible cpus.
 */
void smpboot_unregister_percpu_thread(struct smp_hotplug_thread *plug_thread)
{
}

