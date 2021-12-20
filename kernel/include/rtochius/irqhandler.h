/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_IRQHANDLER_H_
#define __RTOCHIUS_IRQHANDLER_H_

/*
 * Interrupt flow handler typedefs are defined here to avoid circular
 * include dependencies.
 */

struct irq_desc;
struct irq_data;
typedef	void (*irq_flow_handler_t)(struct irq_desc *desc);
typedef	void (*irq_preflow_handler_t)(struct irq_data *data);

#endif /* !__RTOCHIUS_IRQHANDLER_H_ */
