/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_STOP_MACHINE_H_
#define __RTOCHIUS_STOP_MACHINE_H_

#include <rtochius/cpumask.h>

/*
 * stop_cpu[s]() is simplistic per-cpu maximum priority cpu
 * monopolization mechanism.  The caller can specify a non-sleeping
 * function to be executed on a single or multiple cpus preempting all
 * other processes and monopolizing those cpus until it finishes.
 *
 * Resources for this mechanism are preallocated when a cpu is brought
 * up and requests are guaranteed to be served as long as the target
 * cpus are online.
 */
typedef int (*cpu_stop_fn_t)(void *arg);

/**
 * stop_machine: freeze the machine on all CPUs and run this function
 * @fn: the function to run
 * @data: the data ptr for the @fn()
 * @cpus: the cpus to run the @fn() on (NULL = any online cpu)
 *
 * Description: This causes a thread to be scheduled on every cpu,
 * each of which disables interrupts.  The result is that no one is
 * holding a spinlock or inside any other preempt-disabled region when
 * @fn() runs.
 *
 * This can be thought of as a very heavy write lock, equivalent to
 * grabbing every spinlock in the kernel.
 *
 * Protects against CPU hotplug.
 */
int stop_machine(cpu_stop_fn_t fn, void *data, const struct cpumask *cpus);

#endif /* !__RTOCHIUS_STOP_MACHINE_H_ */
