/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __OF_OF_PRIVATE_H_
#define __OF_OF_PRIVATE_H_

#define OF_ROOT_NODE_ADDR_CELLS_DEFAULT 1
#define OF_ROOT_NODE_SIZE_CELLS_DEFAULT 1

#include <base/list.h>

extern struct mutex of_mutex;
extern struct list_head aliases_lookup;

/**
 * struct alias_prop - Alias property in 'aliases' node
 * @link:	List node to link the structure in aliases_lookup list
 * @alias:	Alias property name
 * @np:		Pointer to device_node that the alias stands for
 * @id:		Index value from end of alias name
 * @stem:	Alias string without the index
 *
 * The structure represents one alias property of 'aliases' node as
 * an entry in aliases_lookup list.
 */
struct alias_prop {
	struct list_head link;
	const char *alias;
	struct device_node *np;
	int id;
	char stem[0];
};

extern const void *__of_get_property(const struct device_node *np,
				     const char *name, int *lenp);

struct device_node *__of_find_node_by_path(struct device_node *parent,
						const char *path);
struct device_node *__of_find_node_by_full_path(struct device_node *node,
						const char *path);

extern int __of_add_property(struct device_node *np, struct property *prop);
extern int __of_remove_property(struct device_node *np, struct property *prop);
extern int __of_update_property(struct device_node *np,
		struct property *newprop, struct property **oldprop);

/* illegal phandle value (set when unresolved) */
#define OF_PHANDLE_ILLEGAL	0xdeadbeef

extern void *__unflatten_device_tree(const void *blob,
			      struct device_node *dad,
			      struct device_node **mynodes,
			      void *(*dt_alloc)(u64 size, u64 align),
			      bool detached);

#endif /* !__OF_OF_PRIVATE_H_ */
