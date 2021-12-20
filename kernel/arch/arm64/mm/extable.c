// SPDX-License-Identifier: GPL-2.0
/*
 * Based on arch/arm/mm/extable.c
 */

#include <rtochius/extable.h>

int fixup_exception(struct pt_regs *regs)
{
	const struct exception_table_entry *fixup;

	fixup = search_exception_tables(regs->pc);
	if (fixup)
		regs->pc = (unsigned long)&fixup->fixup + fixup->fixup;

	return fixup != NULL;
}
