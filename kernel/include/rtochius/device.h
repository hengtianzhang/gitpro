// SPDX-License-Identifier: GPL-2.0
/*
 * device.h - generic, centralized driver model
 *
 * Copyright (c) 2001-2003 Patrick Mochel <mochel@osdl.org>
 * Copyright (c) 2004-2009 Greg Kroah-Hartman <gregkh@suse.de>
 * Copyright (c) 2008-2009 Novell Inc.
 *
 * See Documentation/driver-model/ for more information.
 */

#ifndef __RTOCHIUS_DEVICE_H_
#define __RTOCHIUS_DEVICE_H_

struct device;

struct device_driver {
	const char		*name;

	const struct of_device_id	*of_match_table;

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
};

struct device {
	struct device		*parent;

	const char		*init_name; /* initial name of the device */

	struct device_driver *driver;	/* which driver has allocated this
					   device */

	struct device_node	*of_node; /* associated device tree node */
	struct fwnode_handle	*fwnode; /* firmware device node */

	void		*platform_data;	/* Platform specific data, device
					   core doesn't touch it */
	void		*driver_data;	/* Driver data, set and get with
					   dev_set/get_drvdata */
};

#endif /* !__RTOCHIUS_DEVICE_H_ */
