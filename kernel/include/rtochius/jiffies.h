/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_JIFFIES_H_
#define __RTOCHIUS_JIFFIES_H_

#include <asm/cache.h>

#define msecs_to_jiffies(x) (x)
#define HZ		CONFIG_HZ
#define USEC_PER_SEC 1
#define time_before(a,b)	((a) - (b))

#ifndef __jiffy_arch_data
#define __jiffy_arch_data
#endif

extern u64 __cacheline_aligned_in_smp jiffies_64;
extern u64 volatile __cacheline_aligned_in_smp __jiffy_arch_data jiffies;
#endif /* !__RTOCHIUS_JIFFIES_H_ */
