/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_SYSTEM_MISC_H_
#define __ASM_ARM_SYSTEM_MISC_H_

#ifndef __ASSEMBLY__

#include <rtochius/reboot.h>

extern void (*arm_pm_restart)(enum reboot_mode reboot_mode, const char *cmd);
extern void (*pm_power_off)(void);

extern void __show_regs(struct pt_regs *);

#endif /* !__ASSEMBLY__ */
#endif /* !__ASM_ARM_SYSTEM_MISC_H_ */
