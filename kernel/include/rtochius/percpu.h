/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_PERCPU_H_
#define __RTOCHIUS_PERCPU_H_

#include <rtochius/preempt.h>
#include <rtochius/cpumask.h>

#include <asm/percpu.h>

extern void setup_per_cpu_areas(void);

#endif /* !__RTOCHIUS_PERCPU_H_ */
