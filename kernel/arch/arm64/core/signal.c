/*
 * Based on arch/arm/kernel/signal.c
 *
 * Copyright (C) 1995-2009 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <base/init.h>

#include <asm/processor.h>

unsigned long __ro_after_init signal_minsigstksz;

/*
 * Determine the stack space required for guaranteed signal devliery.
 * This function is used to populate AT_MINSIGSTKSZ at process startup.
 * cpufeatures setup is assumed to be complete.
 */
void __init minsigstksz_setup(void)
{
}

asmlinkage void do_notify_resume(struct pt_regs *regs,
				 unsigned long thread_flags)
{
	WARN_ON(1);
}
