/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_SCHED_IDLE_H_
#define __RTOCHIUS_SCHED_IDLE_H_

/* Attach to any functions which should be considered cpuidle. */
#define __cpuidle	__attribute__((__section__(".cpuidle.text")))

/* Linker adds these: start and end of __cpuidle functions */
extern char __cpuidle_text_start[], __cpuidle_text_end[];

#endif /* !__RTOCHIUS_SCHED_IDLE_H_ */
