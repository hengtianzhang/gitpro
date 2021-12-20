#ifndef __RTOCHIUS_PRINTF__H_
#define __RTOCHIUS_PRINTF__H_

#include <base/common.h>

extern char *log_buf_addr_get(void);
extern u32 log_buf_len_get(void);

extern char *kasprintf(gfp_t gfp, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
extern char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);

extern char *kstrdup(const char *s, gfp_t gfp);

#endif /* !__RTOCHIUS_PRINTF__H_ */
