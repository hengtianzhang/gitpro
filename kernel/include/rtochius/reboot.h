/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_REBOOT_H_
#define __RTOCHIUS_REBOOT_H_

enum reboot_mode {
	REBOOT_COLD = 0,
	REBOOT_WARM,
	REBOOT_HARD,
	REBOOT_SOFT,
	REBOOT_GPIO,
};

extern void machine_halt(void);
extern void machine_power_off(void);

#endif /* !__RTOCHIUS_REBOOT_H_ */
