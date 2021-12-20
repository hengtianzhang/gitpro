/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Definitions for working with the Flattened Device Tree data format
 *
 * Copyright 2009 Benjamin Herrenschmidt, IBM Corp
 * benh@kernel.crashing.org
 */

#ifndef __RTOCHIUS_OF_FDT_H_
#define __RTOCHIUS_OF_FDT_H_

#include <base/types.h>
#include <base/init.h>
#include <base/errno.h>
#include <base/libfdt.h>

#include <rtochius/of_reserved_mem.h>

/* Definitions used by the flattened device tree */
#define OF_DT_HEADER		0xd00dfeed	/* marker */

extern u64 of_flat_dt_translate_address(unsigned long node);
extern void of_fdt_limit_memory(int limit);

extern bool of_fdt_is_big_endian(const void *blob,
				 unsigned long node);
extern int of_fdt_match(const void *blob, unsigned long node,
			const char *const *compat);

/* TBD: Temporary export of fdt globals - remove when code fully merged */
extern int __initdata dt_root_addr_cells;
extern int __initdata dt_root_size_cells;
extern void *initial_boot_params;

extern void early_init_fdt_scan_reserved_mem(void);
extern void early_init_fdt_reserve_self(void);

/* For scanning the flat device-tree at boot time */
extern int of_scan_flat_dt(int (*it)(unsigned long node, const char *uname,
				     int depth, void *data),
			   void *data);
extern int of_scan_flat_dt_subnodes(unsigned long node,
				    int (*it)(unsigned long node,
					      const char *uname,
					      void *data),
				    void *data);
extern int of_get_flat_dt_subnode_by_name(unsigned long node,
					  const char *uname);
extern unsigned long of_get_flat_dt_root(void);
extern int of_get_flat_dt_size(void);
extern const void *of_get_flat_dt_prop(unsigned long node, const char *name,
				       int *size);
extern int of_flat_dt_is_compatible(unsigned long node, const char *name);
extern int of_flat_dt_match(unsigned long node, const char *const *matches);
extern uint32_t of_get_flat_dt_phandle(unsigned long node);

extern const char *of_flat_dt_get_machine_name(void);
extern const void *of_flat_dt_match_machine(const void *default_match,
		const void * (*get_next_compat)(const char * const**));

extern phys_addr_t phys_initrd_start;
extern unsigned long phys_initrd_size;

extern int early_init_dt_scan_chosen_stdout(void);

/* Early flat tree scan hooks */
extern int early_init_dt_scan_root(unsigned long node, const char *uname,
				   int depth, void *data);

extern u64 dt_mem_next_cell(int s, const __be32 **cellp);

extern int early_init_dt_scan_memory(unsigned long node, const char *uname,
				     int depth, void *data);

extern int early_init_dt_scan_chosen(unsigned long node, const char *uname,
				     int depth, void *data);

extern void early_init_dt_add_memory_arch(u64 base, u64 size);

extern int early_init_dt_reserve_memory_arch(phys_addr_t base, phys_addr_t size,
					     bool no_map);

extern bool early_init_dt_verify(void *params);
extern void early_init_dt_scan_nodes(void);
extern bool early_init_dt_scan(void *params);

/* Other Prototypes */
extern void unflatten_device_tree(void);

#endif /* !__RTOCHIUS_OF_FDT_H_ */
