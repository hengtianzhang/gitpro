/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_COMPLETION_H_
#define __RTOCHIUS_COMPLETION_H_

/*
 * (C) Copyright 2001 Linus Torvalds
 *
 * Atomic wait-for-completion handler data structures.
 * See kernel/sched/completion.c for details.
 */

/**
 * DECLARE_COMPLETION - declare and initialize a completion structure
 * @work:  identifier for the completion structure
 *
 * This macro declares and initializes a completion structure. Generally used
 * for static declarations. You should use the _ONSTACK variant for automatic
 * variables.
 */
#define DECLARE_COMPLETION(work) struct completion work

struct completion {
	unsigned int done;
};

static inline unsigned long wait_for_completion_timeout(struct completion *x,
						   unsigned long timeout)
{
	return 0;
}

#endif /* !__RTOCHIUS_COMPLETION_H_ */
