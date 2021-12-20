/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_MM_H_
#define __RTOCHIUS_MM_H_

#include <rtochius/gfp.h>
#include <rtochius/mm_types.h>
#include <rtochius/mmzone.h>
#include <rtochius/memory.h>
#include <rtochius/page.h>

#ifndef __pa_symbol
#define __pa_symbol(x)  __pa(RELOC_HIDE((unsigned long)(x), 0))
#endif

#ifndef page_to_virt
#define page_to_virt(x)	__va(PFN_PHYS(page_to_pfn(x)))
#endif

#ifndef lm_alias
#define lm_alias(x)	__va(__pa_symbol(x))
#endif

/*
 * On some architectures it is expensive to call memset() for small sizes.
 * Those architectures should provide their own implementation of "struct page"
 * zeroing by defining this macro in <asm/pgtable.h>.
 */
#ifndef mm_zero_struct_page
#define mm_zero_struct_page(pp)  ((void)memset((pp), 0, sizeof(struct page)))
#endif

#define page_address(page) page_to_virt(page)

/*
 * Drop a ref, return true if the refcount fell to zero (the page has no users)
 */
static inline int put_page_testzero(struct page *page)
{
	BUG_ON(page_ref_count(page) == 0);
	return page_ref_dec_and_test(page);
}

/*
 * Try to grab a ref unless the page has a refcount of zero, return false if
 * that is the case.
 * This can be called when MMU is off so it must not access
 * any of the virtual mappings.
 */
static inline int get_page_unless_zero(struct page *page)
{
	return page_ref_add_unless(page, 1, 0);
}

/*
 * The atomic page->_mapcount, starts from -1: so that transitions
 * both from it and to it can be tracked, using atomic_inc_and_test
 * and atomic_add_negative(-1).
 */
static inline void page_mapcount_reset(struct page *page)
{
	atomic_set(&(page)->_mapcount, -1);
}

static inline struct page *virt_to_head_page(const void *x)
{
	struct page *page = virt_to_page(x);

	return compound_head(page);
}

static inline unsigned int compound_order(struct page *page)
{
	if (!PageHead(page))
		return 0;
	return page[1].compound_order;
}

static inline void set_compound_order(struct page *page, unsigned int order)
{
	page[1].compound_order = order;
}

#define page_private(page)		((page)->private)
#define set_page_private(page, v)	((page)->private = (v))

static inline void set_page_zone(struct page *page, enum zone_type zone)
{
	page->flags &= ~(ZONEID_MASK << NODES_PGOFF);
	page->flags |= (zone & ZONEID_MASK) << NODES_PGOFF;
}

static inline void set_page_links(struct page *page, enum zone_type zone,
	unsigned long pfn)
{
	set_page_zone(page, zone);
}

static inline int page_zone_id(const struct page *page)
{
	return (page->flags >> NODES_PGOFF) & ZONEID_MASK;
}

static inline struct zone *page_zone(const struct page *page)
{
	return &NODE_DATA()->node_zones[page_zone_id(page)];
}

static inline pg_data_t *page_pgdat(const struct page *page)
{
	return NODE_DATA();
}

extern void mem_print_memory_info(void);
extern void reserve_bootmem_region(phys_addr_t start, phys_addr_t end);
extern void memblock_free_pages(struct page *page, unsigned long pfn,
							unsigned int order);
extern void free_area_init_nodes(void);
extern unsigned long memblock_free_all(void);
extern unsigned long free_reserved_area(void *start, void *end,
					int poison, const char *s);

extern void free_compound_page(struct page *page);
extern void free_unref_page(struct page *page);

extern void arch_free_page(struct page *page, int order);

static inline void __put_page(struct page *page)
{
	if (unlikely(PageCompound(page)))
		free_compound_page(page);
	else
		free_unref_page(page);
}

static inline void get_page(struct page *page)
{
	page = compound_head(page);
	/*
	 * Getting a normal page or the head of a compound page
	 * requires to already have an elevated page->_refcount.
	 */
	BUG_ON(page_ref_count(page) <= 0);
	page_ref_inc(page);
}

static inline void put_page(struct page *page)
{
	page = compound_head(page);

	if (put_page_testzero(page))
		__put_page(page);
}

static inline int put_pagetable_testzero(struct page *page)
{
	page = compound_head(page);

	return put_page_testzero(page);
}

static inline void pgtable_init(void)
{
	pgtable_cache_init();
}

static inline bool pgtable_page_ctor(struct page *page)
{
	__SetPageTable(page);
	return true;
}

static inline void pgtable_page_dtor(struct page *page)
{
	__ClearPageTable(page);
}

typedef int (*pte_fn_t)(pte_t *pte, pgtable_t token, unsigned long addr,
			void *data);
extern int apply_to_page_range(struct mm_struct *mm, unsigned long address,
			       unsigned long size, pte_fn_t fn, void *data);

extern unsigned long total_physpages;

extern unsigned long nr_managed_pages(void);
extern unsigned long nr_zone_free_pages(struct zone *zone);
extern unsigned long nr_free_pages(void);
extern unsigned long nr_zone_percpu_cache_pages(struct zone *zone);
extern unsigned long nr_percpu_cache_pages(int cpu);

static inline void mm_pgtables_bytes_init(struct mm_struct *mm)
{
	atomic_long_set(&mm->pgtables_bytes, 0);
}

static inline unsigned long mm_pgtables_bytes(const struct mm_struct *mm)
{
	return atomic_long_read(&mm->pgtables_bytes);
}

static inline void mm_inc_nr_ptes(struct mm_struct *mm)
{
	atomic_long_add(PTRS_PER_PTE * sizeof(pte_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_ptes(struct mm_struct *mm)
{
	atomic_long_sub(PTRS_PER_PTE * sizeof(pte_t), &mm->pgtables_bytes);
}

static inline void mm_inc_nr_pmds(struct mm_struct *mm)
{
	atomic_long_add(PTRS_PER_PMD * sizeof(pmd_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_pmds(struct mm_struct *mm)
{
	atomic_long_sub(PTRS_PER_PMD * sizeof(pmd_t), &mm->pgtables_bytes);
}

static inline void mm_inc_nr_puds(struct mm_struct *mm)
{
	atomic_long_add(PTRS_PER_PUD * sizeof(pud_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_puds(struct mm_struct *mm)
{
	atomic_long_sub(PTRS_PER_PUD * sizeof(pud_t), &mm->pgtables_bytes);
}

static inline void mm_inc_nr_p4ds(struct mm_struct *mm)
{
	atomic_long_add(PTRS_PER_P4D * sizeof(p4d_t), &mm->pgtables_bytes);
}

static inline void mm_dec_nr_p4ds(struct mm_struct *mm)
{
	atomic_long_sub(PTRS_PER_P4D * sizeof(p4d_t), &mm->pgtables_bytes);
}

extern void unmap_kernel_range(unsigned long addr, unsigned long size);
extern void free_initmem(void);

#endif /* !__RTOCHIUS_MM_H_ */
