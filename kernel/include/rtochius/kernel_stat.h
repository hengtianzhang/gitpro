#ifndef __RTOCHIUS_KERNEL_STAT_H_
#define __RTOCHIUS_KERNEL_STAT_H_

#include <base/linkage.h>

/*
 * Values used for system_state. Ordering of the states must not be changed
 * as code checks for <, <=, >, >= STATE.
 */
extern enum system_states {
	SYSTEM_BOOTING,
	SYSTEM_SCHEDULING,
	SYSTEM_RUNNING,
	SYSTEM_HALT,
	SYSTEM_POWER_OFF,
	SYSTEM_RESTART,
	SYSTEM_SUSPEND,
} system_state;

#define COMMAND_LINE_SIZE	2048

extern char boot_command_line[COMMAND_LINE_SIZE];
extern char *saved_command_line;

extern bool rodata_enabled;

extern asmlinkage void start_kernel(void);
extern void setup_arch(char *);

#endif /* !__RTOCHIUS_KERNEL_STAT_H_ */
