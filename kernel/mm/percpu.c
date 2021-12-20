
/*
 * Generic SMP percpu area setup.
 *
 * The embedding helper is used because its behavior closely resembles
 * the original non-dynamic generic percpu area setup.  This is
 * important because many archs have addressing restrictions and might
 * fail if the percpu area is located far away from the previous
 * location.  As an added bonus, in non-NUMA cases, embedding is
 * generally a good idea TLB-wise because percpu area can piggy back
 * on the physical linear memory mapping which uses large page
 * mappings on applicable archs.
 */
#define pr_fmt(fmt) "percpu: " fmt

#include <base/init.h>

#include <rtochius/cpumask.h>
#include <rtochius/percpu.h>
#include <rtochius/threads.h>
#include <rtochius/memory.h>

#include <asm/cache.h>
#include <asm/sections.h>

unsigned long __per_cpu_offset[NR_CPUS] __read_mostly;

void __init setup_per_cpu_areas(void)
{
	void *alloc_ptr;
	unsigned long delta = __per_cpu_end - __per_cpu_load;
	unsigned int cpu;
	unsigned long offset;

	for_each_possible_cpu(cpu) {
		alloc_ptr = memblock_alloc_virt(&memblock_kernel, delta, SMP_CACHE_BYTES);
		if (!alloc_ptr)
			panic("percpu alloc failed!\n");
		offset = (unsigned long)alloc_ptr - (unsigned long)__per_cpu_load;
		memcpy(alloc_ptr, __per_cpu_load, delta);
		__per_cpu_offset[cpu] = offset;
	}
}
