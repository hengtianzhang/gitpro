#ifndef __RTOCHIUS_HUGEPAGE_H_
#define __RTOCHIUS_HUGEPAGE_H_

#include <rtochius/mm_types.h>

#include <asm/pgtable.h>

#define HPAGE_PMD_ORDER (HPAGE_PMD_SHIFT-PAGE_SHIFT)
#define HPAGE_PMD_NR (1<<HPAGE_PMD_ORDER)

#define HPAGE_PMD_SHIFT PMD_SHIFT
#define HPAGE_PMD_SIZE	((1UL) << HPAGE_PMD_SHIFT)
#define HPAGE_PMD_MASK	(~(HPAGE_PMD_SIZE - 1))

#define HPAGE_PUD_SHIFT PUD_SHIFT
#define HPAGE_PUD_SIZE	((1UL) << HPAGE_PUD_SHIFT)
#define HPAGE_PUD_MASK	(~(HPAGE_PUD_SIZE - 1))

#define pmd_huge_pte(mm, pmd) ((mm)->pmd_huge_pte)

#endif /* !__RTOCHIUS_HUGEPAGE_H_ */
