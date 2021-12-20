#ifndef __RTOCHIUS_BUILD_SALT_H_
#define __RTOCHIUS_BUILD_SALT_H_

#include <base/elfnote.h>

#define RTOCHIUS_ELFNOTE_BUILD_SALT       0x100

#ifdef __ASSEMBLER__

#define BUILD_SALT \
       ELFNOTE(Rtochius, RTOCHIUS_ELFNOTE_BUILD_SALT, .asciz CONFIG_BUILD_SALT)

#else

#define BUILD_SALT \
       ELFNOTE32("Rtochius", RTOCHIUS_ELFNOTE_BUILD_SALT, CONFIG_BUILD_SALT)

#endif

#endif /* !__RTOCHIUS_BUILD_SALT_H_ */
