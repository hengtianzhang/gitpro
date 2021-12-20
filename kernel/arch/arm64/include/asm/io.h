#ifndef __ASM_IO_H_
#define __ASM_IO_H_

#include <rtochius/mm_types.h>

#include <asm/pgtable-types.h>
#include <asm/pgtable-prot.h>

#include <asm/base/io.h>

/*
 * I/O memory mapping functions.
 */
extern int __create_iomap_remap(phys_addr_t phys_addr, u64 virt,
					size_t size, pgprot_t prot, int flags);
extern void __iomem *__ioremap(phys_addr_t phys_addr, size_t size, pgprot_t prot);
extern void __iounmap(volatile void __iomem *addr);
extern void __iomem *ioremap_cache(phys_addr_t phys_addr, size_t size);

extern void __iomem *ioremap(phys_addr_t phys_addr, size_t size);
#define ioremap_nocache(addr, size)	__ioremap((addr), (size), __pgprot(PROT_DEVICE_nGnRE))
#define ioremap_wc(addr, size)		__ioremap((addr), (size), __pgprot(PROT_NORMAL_NC))
#define ioremap_wt(addr, size)		__ioremap((addr), (size), __pgprot(PROT_DEVICE_nGnRE))
#define iounmap				__iounmap

extern int arch_ioremap_pud_supported(void);
extern int arch_ioremap_pmd_supported(void);

#endif /* !__ASM_IO_H_ */
