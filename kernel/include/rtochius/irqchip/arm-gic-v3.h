/*
 * Copyright (C) 2013, 2014 ARM Limited, All Rights Reserved.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __RTOCHIUS_IRQCHIP_ARM_GIC_V3_H_
#define __RTOCHIUS_IRQCHIP_ARM_GIC_V3_H_

static inline bool gic_enable_sre(void)
{
	return false;
}

#endif /* !__RTOCHIUS_IRQCHIP_ARM_GIC_V3_H_ */
