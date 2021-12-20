// SPDX-License-Identifier: GPL-2.0
#include <rtochius/mm_types.h>
#include <rtochius/spinlock.h>

#include <asm/pgtable.h>
#include <asm/mmu.h>

#ifndef INIT_MM_CONTEXT
#define INIT_MM_CONTEXT(name)
#endif

struct mm_struct init_mm = {
    .pgd		= swapper_pg_dir,
    .page_table_lock = __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
    INIT_MM_CONTEXT(init_mm)
};
