#ifndef __UAPI_RTOCHIUS_RESOURCE_H_
#define __UAPI_RTOCHIUS_RESOURCE_H_

#include <base/types.h>

/*
 * Resources are tree-like, allowing
 * nesting etc..
 */
struct resource {
	resource_size_t start;
	resource_size_t end;
	const char *name;
	unsigned long flags;
	unsigned long desc;
	struct resource *parent, *sibling, *child;
};

#define IORESOURCE_IO		0x00000100	/* PCI/ISA I/O ports */
#define IORESOURCE_MEM		0x00000200
#define IORESOURCE_REG		0x00000300	/* Register offsets */
#define IORESOURCE_IRQ		0x00000400
#define IORESOURCE_DMA		0x00000800
#define IORESOURCE_BUS		0x00001000

#define IORESOURCE_PREFETCH	0x00002000	/* No side effects */

#define IORESOURCE_SYSRAM	0x01000000	/* System RAM (modifier) */

/* I/O resource extended types */
#define IORESOURCE_SYSTEM_RAM		(IORESOURCE_MEM|IORESOURCE_SYSRAM)

static inline resource_size_t resource_size(const struct resource *res)
{
	return res->end - res->start + 1;
}

#endif /* !__UAPI_RTOCHIUS_RESOURCE_H_ */
