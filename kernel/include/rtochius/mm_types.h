#ifndef __RTOCHIUS_MM_TYPES_H_
#define __RTOCHIUS_MM_TYPES_H_

#ifndef __ASSEMBLY__

#include <base/compiler.h>
#include <base/types.h>
#include <base/atomic.h>
#include <base/log2.h>
#include <base/list.h>

#include <rtochius/spinlock.h>

#include <asm/page.h>
#include <asm/pgtable-types.h>
#include <asm/mmu.h>

#ifdef CONFIG_HAVE_ALIGNED_STRUCT_PAGE
#define _struct_page_alignment	__aligned(2 * sizeof(unsigned long))
#else
#define _struct_page_alignment
#endif

struct page {
	unsigned long flags;

	union {
		struct {
			struct list_head lru;
			void 		*mapping; /* Reserved */
			unsigned long	index;  /* Reserved */
			unsigned long private;
		};
		struct {
			struct list_head	slab_list;
			struct page		*page;	 /* Reserved */
			struct kmem_cache *slab;
			void *freelist;
			unsigned inuse;
		};
		struct {	/* Tail pages of compound page */
			unsigned long compound_head;	/* Bit zero is set */
			unsigned long compound_dtor;	 /* Reserved */
			unsigned char compound_order;
			unsigned long compound_mapcount; /* Reserved */
		};
	};

	union {
		atomic_t _mapcount;

		unsigned int page_type;
	};

	/* Usage count. *DO NOT USE DIRECTLY*. See page_ref.h */
	atomic_t _refcount;
} _struct_page_alignment;

#include <rtochius/page-flags-layout.h>
#include <rtochius/page-flags.h>
#include <rtochius/page_ref.h>

/*
 * Used for sizing the vmemmap region on some architectures
 */
#define STRUCT_PAGE_MAX_SHIFT	(order_base_2(sizeof(struct page)))

struct mm_struct {
	pgd_t *pgd;
	/* Architecture-specific MM context */
	mm_context_t context;

	pgtable_t pmd_huge_pte; /* protected by page_table_lock */

	atomic_long_t pgtables_bytes;

	spinlock_t page_table_lock; /* Protects page tables and some counters */

	/*
		* An operation with batched TLB flushing is going on. Anything
		* that can move process memory needs to flush the TLB when
		* moving a PROT_NONE or PROT_NUMA mapped page.
		*/
	atomic_t tlb_flush_pending;
};

extern struct mm_struct init_mm;

/*
 * No scalability reason to split PUD locks yet, but follow the same pattern
 * as the PMD locks to make it easier if we decide to.  The VM should not be
 * considered ready to switch to split PUD locks yet; there may be places
 * which need to be converted from page_table_lock.
 */
static inline spinlock_t *pud_lockptr(struct mm_struct *mm, pud_t *pud)
{
	return &mm->page_table_lock;
}

static inline spinlock_t *pmd_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return &mm->page_table_lock;
}

/*
 * We use mm->page_table_lock to guard all pagetable pages of the mm.
 */
static inline spinlock_t *pte_lockptr(struct mm_struct *mm, pmd_t *pmd)
{
	return &mm->page_table_lock;
}

static inline spinlock_t *pud_lock(struct mm_struct *mm, pud_t *pud)
{
	spinlock_t *ptl = pud_lockptr(mm, pud);

	spin_lock(ptl);
	return ptl;
}

static inline spinlock_t *pmd_lock(struct mm_struct *mm, pmd_t *pmd)
{
	spinlock_t *ptl = pmd_lockptr(mm, pmd);
	spin_lock(ptl);
	return ptl;
}

static inline void init_tlb_flush_pending(struct mm_struct *mm)
{
	atomic_set(&mm->tlb_flush_pending, 0);
}

static inline void inc_tlb_flush_pending(struct mm_struct *mm)
{
	atomic_inc(&mm->tlb_flush_pending);
	/*
	 * The only time this value is relevant is when there are indeed pages
	 * to flush. And we'll only flush pages after changing them, which
	 * requires the PTL.
	 *
	 * So the ordering here is:
	 *
	 *	atomic_inc(&mm->tlb_flush_pending);
	 *	spin_lock(&ptl);
	 *	...
	 *	set_pte_at();
	 *	spin_unlock(&ptl);
	 *
	 *				spin_lock(&ptl)
	 *				mm_tlb_flush_pending();
	 *				....
	 *				spin_unlock(&ptl);
	 *
	 *	flush_tlb_range();
	 *	atomic_dec(&mm->tlb_flush_pending);
	 *
	 * Where the increment if constrained by the PTL unlock, it thus
	 * ensures that the increment is visible if the PTE modification is
	 * visible. After all, if there is no PTE modification, nobody cares
	 * about TLB flushes either.
	 *
	 * This very much relies on users (mm_tlb_flush_pending() and
	 * mm_tlb_flush_nested()) only caring about _specific_ PTEs (and
	 * therefore specific PTLs), because with SPLIT_PTE_PTLOCKS and RCpc
	 * locks (PPC) the unlock of one doesn't order against the lock of
	 * another PTL.
	 *
	 * The decrement is ordered by the flush_tlb_range(), such that
	 * mm_tlb_flush_pending() will not return false unless all flushes have
	 * completed.
	 */
}

static inline void dec_tlb_flush_pending(struct mm_struct *mm)
{
	/*
	 * See inc_tlb_flush_pending().
	 *
	 * This cannot be smp_mb__before_atomic() because smp_mb() simply does
	 * not order against TLB invalidate completion, which is what we need.
	 *
	 * Therefore we must rely on tlb_flush_*() to guarantee order.
	 */
	atomic_dec(&mm->tlb_flush_pending);
}

static inline bool mm_tlb_flush_pending(struct mm_struct *mm)
{
	/*
	 * Must be called after having acquired the PTL; orders against that
	 * PTLs release and therefore ensures that if we observe the modified
	 * PTE we must also observe the increment from inc_tlb_flush_pending().
	 *
	 * That is, it only guarantees to return true if there is a flush
	 * pending for _this_ PTL.
	 */
	return atomic_read(&mm->tlb_flush_pending);
}

static inline bool mm_tlb_flush_nested(struct mm_struct *mm)
{
	/*
	 * Similar to mm_tlb_flush_pending(), we must have acquired the PTL
	 * for which there is a TLB flush pending in order to guarantee
	 * we've seen both that PTE modification and the increment.
	 *
	 * (no requirement on actually still holding the PTL, that is irrelevant)
	 */
	return atomic_read(&mm->tlb_flush_pending) > 1;
}

struct vm_area_struct {
	struct mm_struct *vm_mm;	/* The address space we belong to. */
	unsigned long vm_flags;		/* Flags, see mm.h. */
};

#endif /* !__ASSEMBLY__ */
#endif /* !__RTOCHIUS_MM_TYPES_H_ */
