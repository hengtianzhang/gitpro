#ifndef __RTOCHIUS_MMAP_H_
#define __RTOCHIUS_MMAP_H_

#include <uapi/rtochius/mmap.h>

#include <asm/pgtable-types.h>

extern pgprot_t vm_get_page_prot(unsigned long vm_flags);

#endif /* !__RTOCHIUS_MMAP_H_ */
