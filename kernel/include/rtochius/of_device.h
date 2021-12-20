/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_OF_DEVICE_H_
#define __RTOCHIUS_OF_DEVICE_H_

#include <rtochius/of.h>
#include <rtochius/cpu.h>
#include <rtochius/device.h>

extern const struct of_device_id *of_match_device(
	const struct of_device_id *matches, const struct device *dev);

extern const void *of_device_get_match_data(const struct device *dev);

extern ssize_t of_device_modalias(struct device *dev, char *str, ssize_t len);

static inline struct device_node *of_cpu_device_node_get(int cpu)
{
	struct device *cpu_dev;
	cpu_dev = get_cpu_device(cpu);
	if (!cpu_dev)
		return of_get_cpu_node(cpu, NULL);
	return of_node_get(cpu_dev->of_node);
}

#endif /* !__RTOCHIUS_OF_DEVICE_H_ */
