/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_OF_ADDRESS_H_
#define __RTOCHIUS_OF_ADDRESS_H_

#include <base/errno.h>

#include <rtochius/of.h>

#include <uapi/rtochius/resource.h>
#include <uapi/rtochius/pci_regs.h>

#include <asm/io.h>

struct of_pci_range_parser {
	struct device_node *node;
	const __be32 *range;
	const __be32 *end;
	int np;
	int pna;
};

struct of_pci_range {
	u32 pci_space;
	u64 pci_addr;
	u64 cpu_addr;
	u64 size;
	u32 flags;
};

extern const __be32 *of_get_pci_address(struct device_node *dev, int bar_no,
			       u64 *size, unsigned int *flags);
extern int of_pci_address_to_resource(struct device_node *dev, int bar,
				      struct resource *r);
extern int of_pci_range_to_resource(struct of_pci_range *range,
				    struct device_node *np,
				    struct resource *res);
extern int of_pci_range_parser_init(struct of_pci_range_parser *parser,
			struct device_node *node);
extern int of_pci_dma_range_parser_init(struct of_pci_range_parser *parser,
			struct device_node *node);
extern struct of_pci_range *of_pci_range_parser_one(
					struct of_pci_range_parser *parser,
					struct of_pci_range *range);
#define for_each_of_pci_range(parser, range) \
	for (; of_pci_range_parser_one(parser, range);)

extern u64 of_translate_address(struct device_node *np, const __be32 *addr);

/* Translate a DMA address from device space to CPU space */
extern u64 of_translate_dma_address(struct device_node *dev,
				    const __be32 *in_addr);

/* Extract an address from a device, returns the region size and
 * the address space flags too. The PCI version uses a BAR number
 * instead of an absolute index
 */
extern const __be32 *of_get_address(struct device_node *dev, int index,
			   u64 *size, unsigned int *flags);

extern int of_address_to_resource(struct device_node *dev, int index,
				  struct resource *r);

extern struct device_node *of_find_matching_node_by_address(
					struct device_node *from,
					const struct of_device_id *matches,
					u64 base_address);

void __iomem *of_iomap(struct device_node *node, int index);

extern int of_dma_get_range(struct device_node *np, u64 *dma_addr,
				u64 *paddr, u64 *size);
extern bool of_dma_is_coherent(struct device_node *np);

#endif /* !__RTOCHIUS_OF_ADDRESS_H_ */
