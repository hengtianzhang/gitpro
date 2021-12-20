#ifndef __RTOCHIUS_MEMORY_H_
#define __RTOCHIUS_MEMORY_H_

#include <base/string.h>
#include <base/memblock.h>
#include <base/overflow.h>

#include <asm/pgtable.h>
#include <asm/memory.h>

extern struct memblock memblock_kernel;

static inline void *memblock_alloc_virt(struct memblock *mb, phys_addr_t size,  phys_addr_t align)
{
	void *virt;
	phys_addr_t addr;

	addr = memblock_alloc(mb, size, align);
	if (!addr)
		return NULL;

	virt = phys_to_virt(addr);
	memset(virt, 0, size);
	return virt;
}

static inline void *memblock_calloc_virt(struct memblock *mb, size_t n, size_t size, phys_addr_t align)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;
	
	return memblock_alloc_virt(mb, bytes, align);
}

#ifdef CONFIG_MEMTEST
extern void early_memtest(phys_addr_t start, phys_addr_t end);
#else
static inline void early_memtest(phys_addr_t start, phys_addr_t end)
{
}
#endif

#endif /* !__RTOCHIUS_MEMORY_H_ */
