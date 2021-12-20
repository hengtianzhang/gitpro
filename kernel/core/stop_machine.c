/*
 * kernel/stop_machine.c
 *
 * Copyright (C) 2008, 2005	IBM Corporation.
 * Copyright (C) 2008, 2005	Rusty Russell rusty@rustcorp.com.au
 * Copyright (C) 2010		SUSE Linux Products GmbH
 * Copyright (C) 2010		Tejun Heo <tj@kernel.org>
 *
 * This file is released under the GPLv2 and any later version.
 */
#include <rtochius/stop_machine.h>

int stop_machine(cpu_stop_fn_t fn, void *data, const struct cpumask *cpus)
{
	return 0;
}
