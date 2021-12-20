/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Mutexes: blocking mutual exclusion locks
 *
 * started by Ingo Molnar:
 *
 *  Copyright (C) 2004, 2005, 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 *
 * This file contains the main data structure and API definitions.
 */
#ifndef __RTOCHIUS_MUTEX_H_
#define __RTOCHIUS_MUTEX_H_

#include <base/list.h>

#include <rtochius/spinlock.h>

struct mutex {
	atomic_long_t		owner;
	spinlock_t		wait_lock;
	struct list_head	wait_list;
};

#define __MUTEX_INITIALIZER(lockname) \
		{ .owner = ATOMIC_LONG_INIT(0) \
		, .wait_lock = __SPIN_LOCK_UNLOCKED(lockname.wait_lock) \
		, .wait_list = LIST_HEAD_INIT(lockname.wait_list) }

#define DEFINE_MUTEX(mutexname) \
	struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#define mutex_init(mutex)

#define mutex_lock(lock)

#define mutex_unlock(lock)

#endif /* !__RTOCHIUS_MUTEX_H_ */
