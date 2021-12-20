/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_UTS_H_
#define __RTOCHIUS_UTS_H_

#include <uapi/rtochius/utsname.h>

/*
 * Defines for what uname() should return 
 */
#ifndef UTS_SYSNAME
#define UTS_SYSNAME "Rtochius"
#endif

#ifndef UTS_NODENAME
#define UTS_NODENAME CONFIG_DEFAULT_HOSTNAME /* set by sethostname() */
#endif

#ifndef UTS_DOMAINNAME
#define UTS_DOMAINNAME "(none)"	/* set by setdomainname() */
#endif

extern const struct new_utsname utsname;
extern const char rtochius_banner[];

#endif /* !__RTOCHIUS_UTS_H_ */
