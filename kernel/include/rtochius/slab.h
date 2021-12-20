/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Written by Mark Hemment, 1996 (markhe@nextd.demon.co.uk).
 *
 * (C) SGI 2006, Christoph Lameter
 * 	Cleaned up and restructured to ease the addition of alternative
 * 	implementations of SLAB allocators.
 * (C) Linux Foundation 2008-2013
 *      Unified interface for all slab allocators
 */

#ifndef __RTOCHIUS_SLAB_H_
#define	__RTOCHIUS_SLAB_H_

#include <rtochius/gfp.h>
#include <rtochius/slab_def.h>
#include <rtochius/memory.h>

#define SLAB_RED_ZONE		((slab_flags_t)0x00000400U)	/* DEBUG: Red zone objs in a cache */
#define SLAB_POISON			((slab_flags_t)0x00000800U)	/* DEBUG: Poison objects */
#define SLAB_HWCACHE_ALIGN	((slab_flags_t)0x00002000U)	/* Align objs on cache lines */
#define SLAB_CACHE_DMA		((slab_flags_t)0x00004000U)	/* Use GFP_DMA memory */
#define SLAB_STORE_USER		((slab_flags_t)0x00010000U)	/* DEBUG: Store the last owner for bug hunting */
#define SLAB_PANIC			((slab_flags_t)0x00040000U)	/* Panic if kmem_cache_create() fails */

/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= \
				(unsigned long)ZERO_SIZE_PTR)

/*
 * Setting ARCH_SLAB_MINALIGN in arch headers allows a different alignment.
 * Intended for arches that get misalignment faults even for 64 bit integer
 * aligned buffers.
 */
#ifndef ARCH_SLAB_MINALIGN
#define ARCH_SLAB_MINALIGN __alignof__(unsigned long long)
#endif

void kmem_cache_init(void);
int slab_is_available(void);

struct kmem_cache *kmem_cache_create(const char *name, size_t size,
		size_t align, unsigned long flags,
		void (*ctor)(struct kmem_cache *, void *));
void kmem_cache_destroy(struct kmem_cache *s);
void *kmem_cache_alloc(struct kmem_cache *, gfp_t);
void kmem_cache_free(struct kmem_cache *, void *);

unsigned int kmem_cache_size(struct kmem_cache *);
const char *kmem_cache_name(struct kmem_cache *);
int kmem_ptr_validate(struct kmem_cache *cachep, const void *ptr);

void kfree(const void *);
size_t ksize(const void *);

void *__kmalloc(size_t size, gfp_t flags);

static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size)) {
		if (size > PAGE_SIZE / 2)
			return (void *)__get_free_pages(flags,
							get_order(size));

		if (!(flags & GFP_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc(s, flags);
		}
	}
	return __kmalloc(size, flags);
}

/*
 * Shortcuts
 */
static inline void *kmem_cache_zalloc(struct kmem_cache *k, gfp_t flags)
{
	return kmem_cache_alloc(k, flags | __GFP_ZERO);
}

/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
	return kmalloc(size, flags | __GFP_ZERO);
}

/**
 * kmalloc_array - allocate memory for an array.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;
	if (__builtin_constant_p(n) && __builtin_constant_p(size))
		return kmalloc(bytes, flags);
	return __kmalloc(bytes, flags);
}

/**
 * kcalloc - allocate memory for an array. The memory is set to zero.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{
	return kmalloc_array(n, size, flags | __GFP_ZERO);
}

#endif /* !__RTOCHIUS_SLAB_H_ */
