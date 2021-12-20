#ifndef __RTOCHIUS_KBUILD_H_
#define __RTOCHIUS_KBUILD_H_

#include <base/stddef.h>

#define DEFINE(sym, val) \
        asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

#define OFFSET(sym, str, mem) \
	DEFINE(sym, offsetof(struct str, mem))

#define COMMENT(x) \
	asm volatile("\n->#" x)

#endif /* !__RTOCHIUS_KBUILD_H_ */
