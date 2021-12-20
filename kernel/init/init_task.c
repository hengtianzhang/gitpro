// SPDX-License-Identifier: GPL-2.0
#include <rtochius/sched.h>
#include <rtochius/mm_types.h>

#include <asm/memory.h>

/* Attach to the init_task data structure for proper alignment */
#define __init_task_data __attribute__((__section__(".data..init_task")))

/* Attach to the thread_info data structure for proper alignment */
#define __init_thread_info __attribute__((__section__(".data..init_thread_info")))

extern unsigned long init_stack[THREAD_SIZE / sizeof(unsigned long)];

#define INIT_TASK_COMM "swapper"

/*
 * Set up the first task table, touch at your own risk!. Base=0,
 * limit=0x1fffff (=2MB)
 */
struct task_struct init_task = {
	.thread_info	= INIT_THREAD_INFO(init_task),
	.state		= 0,
	.stack		= init_stack,
	.mm		= &init_mm,
	.comm		= INIT_TASK_COMM,
	.thread = INIT_THREAD,
};
