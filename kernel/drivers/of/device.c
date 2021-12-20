// SPDX-License-Identifier: GPL-2.0
#include <rtochius/of.h>
#include <rtochius/of_device.h>

#include "of_private.h"

/**
 * of_match_device - Tell if a struct device matches an of_device_id list
 * @ids: array of of device match structures to search in
 * @dev: the of device structure to match against
 *
 * Used by a driver to check whether an platform_device present in the
 * system is in its list of supported devices.
 */
const struct of_device_id *of_match_device(const struct of_device_id *matches,
					   const struct device *dev)
{
	if ((!matches) || (!dev->of_node))
		return NULL;
	return of_match_node(matches, dev->of_node);
}

const void *of_device_get_match_data(const struct device *dev)
{
	const struct of_device_id *match;

	match = of_match_device(dev->driver->of_match_table, dev);
	if (!match)
		return NULL;

	return match->data;
}

static ssize_t of_device_get_modalias(struct device *dev, char *str, ssize_t len)
{
	const char *compat;
	char *c;
	struct property *p;
	ssize_t csize;
	ssize_t tsize;

	if ((!dev) || (!dev->of_node))
		return -ENODEV;

	/* Name & Type */
	/* %p eats all alphanum characters, so %c must be used here */
	csize = snprintf(str, len, "of:N%pn%c%s", dev->of_node, 'T',
			 of_node_get_device_type(dev->of_node));
	tsize = csize;
	len -= csize;
	if (str)
		str += csize;

	of_property_for_each_string(dev->of_node, "compatible", p, compat) {
		csize = strlen(compat) + 1;
		tsize += csize;
		if (csize > len)
			continue;

		csize = snprintf(str, len, "C%s", compat);
		for (c = str; c; ) {
			c = strchr(c, ' ');
			if (c)
				*c++ = '_';
		}
		len -= csize;
		str += csize;
	}

	return tsize;
}

/**
 * of_device_modalias - Fill buffer with newline terminated modalias string
 */
ssize_t of_device_modalias(struct device *dev, char *str, ssize_t len)
{
	ssize_t sl = of_device_get_modalias(dev, str, len - 2);
	if (sl < 0)
		return sl;
	if (sl > len - 2)
		return -ENOMEM;

	str[sl++] = '\n';
	str[sl] = 0;
	return sl;
}
