/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_OF_RESERVED_MEM_H_
#define __RTOCHIUS_OF_RESERVED_MEM_H_

#include <base/types.h>

#include <rtochius/device.h>

struct of_phandle_args;
struct reserved_mem_ops;

struct reserved_mem {
	const char			*name;
	unsigned long			fdt_node;
	unsigned long			phandle;
	const struct reserved_mem_ops	*ops;
	phys_addr_t			base;
	phys_addr_t			size;
	void				*priv;
};

struct reserved_mem_ops {
	int	(*device_init)(struct reserved_mem *rmem,
			       struct device *dev);
	void	(*device_release)(struct reserved_mem *rmem,
				  struct device *dev);
};

#ifdef CONFIG_OF_RESERVED_MEM
int early_init_dt_alloc_reserved_memory_arch(phys_addr_t size,
					     phys_addr_t align,
					     phys_addr_t start,
					     phys_addr_t end,
					     bool nomap,
					     phys_addr_t *res_base);

void fdt_reserved_mem_save_node(unsigned long node, const char *uname,
			       phys_addr_t base, phys_addr_t size);
void fdt_init_reserved_mem(void);
struct reserved_mem *of_reserved_mem_lookup(struct device_node *np);
#else
static inline void fdt_reserved_mem_save_node(unsigned long node,
		const char *uname, phys_addr_t base, phys_addr_t size) { }
static inline void fdt_init_reserved_mem(void) { }
static inline struct reserved_mem *of_reserved_mem_lookup(struct device_node *np)
{
	return NULL;
}
#endif /* CONFIG_OF_RESERVED_MEM */

#endif /* !__RTOCHIUS_OF_RESERVED_MEM_H_ */
