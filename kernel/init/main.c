/*
 *  linux/init/main.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  GK 2/5/95  -  Changed to support mounting root fs via NFS
 *  Added initrd & change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Moan early if gcc is old, avoiding bogus kernels - Paul Gortmaker, May '96
 *  Simplified starting of init:  Michael A. Griffith <grif@acm.org>
 */
#include <base/compiler.h>
#include <base/linkage.h>
#include <base/init.h>
#include <base/types.h>

#include <rtochius/kernel_stat.h>
#include <rtochius/uts.h>
#include <rtochius/slab.h>
#include <rtochius/mm.h>
#include <rtochius/sched/init.h>
#include <rtochius/sched/task_stack.h>
#include <rtochius/smp.h>
#include <rtochius/cpu.h>
#include <rtochius/irqflags.h>
#include <rtochius/param.h>
#include <rtochius/memory.h>
#include <rtochius/extable.h>
#include <rtochius/stackprotector.h>
#include <rtochius/radix-tree.h>

#include <asm/mmu.h>

enum system_states system_state __read_mostly;

bool rodata_enabled __ro_after_init = true;

#if 0
static void mark_readonly(void)
{
	if (rodata_enabled) {
		mark_rodata_ro();
	} else
		pr_info("Kernel memory protection disabled.\n");
}
#endif

/* Untouched command line saved by arch-specific code. */
char __initdata boot_command_line[COMMAND_LINE_SIZE];

/* Untouched saved command line (eg. for /proc) */
char *saved_command_line;

/*
 * We need to store the untouched command line for future reference.
 * We also need to store the touched command line since the parameter
 * parsing is performed in place, and we should allow a component to
 * store reference of name/value for future reference.
 */
static void __init setup_command_line(void)
{
	saved_command_line =
		memblock_alloc_virt(&memblock_kernel, strlen(boot_command_line) + 1, SMP_CACHE_BYTES);
	strcpy(saved_command_line, boot_command_line);
}

static int __init do_early_param(char *param, char *val,
					const char *unused, void *arg)
{
	const struct obs_kernel_param *p;

	for (p = __setup_start; p < __setup_end; p++) {
		if (parameq(param, p->str))
			if (p->setup_func(val) != 0)
				pr_warn("Malformed early option '%s'\n", param);
	}
	/* We accept everything at this stage. */
	return 0;
}

void __init parse_early_options(char *cmdline)
{
	parse_args("early options", cmdline, NULL, do_early_param);
}

/* Arch code calls this early on, or if not, just before other parsing. */
void __init parse_early_param(void)
{
	static int done __initdata;
	static char tmp_cmdline[COMMAND_LINE_SIZE] __initdata;

	if (done)
		return;

	/* All fall through to do_early_param. */
	strlcpy(tmp_cmdline, boot_command_line, COMMAND_LINE_SIZE);
	parse_early_options(tmp_cmdline);
	done = 1;
}

extern initcall_entry_t __initcall_start[];
extern initcall_entry_t __initcall0_start[];
extern initcall_entry_t __initcall1_start[];
extern initcall_entry_t __initcall2_start[];
extern initcall_entry_t __initcall3_start[];
extern initcall_entry_t __initcall4_start[];
extern initcall_entry_t __initcall5_start[];
extern initcall_entry_t __initcall6_start[];
extern initcall_entry_t __initcall7_start[];
extern initcall_entry_t __initcall_end[];

#if 0
static initcall_entry_t *initcall_levels[] __initdata = {
	__initcall0_start,
	__initcall1_start,
	__initcall2_start,
	__initcall3_start,
	__initcall4_start,
	__initcall5_start,
	__initcall6_start,
	__initcall7_start,
	__initcall_end,
};

/* Keep these in sync with initcalls in include/linux/init.h */
static const char *initcall_level_names[] __initdata = {
	"early",
	"core",
	"postcore",
	"arch",
	"subsys",
	"device",
	"userver",
	"late",
};
#endif

static __init void mm_init(void)
{
	free_area_init_nodes();
	mem_print_memory_info();
	kmem_cache_init();
	pgtable_init();
}

asmlinkage __visible void __init start_kernel(void)
{
	set_task_stack_end_magic(&init_task);

	smp_setup_processor_id();

	local_irq_disable();

	boot_cpu_init();
	pr_notice("%s", rtochius_banner);
	setup_arch(boot_command_line);

	boot_init_stack_canary();
	setup_command_line();
	setup_nr_cpu_ids();
	setup_per_cpu_areas();
	smp_prepare_boot_cpu();	/* arch-specific boot-cpu hooks */

	pr_notice("Kernel command line: %s\n", boot_command_line);
	parse_early_options(boot_command_line);

	sort_main_extable();
	mm_init();

	/*
	 * Set up the scheduler prior starting any interrupts (such as the
	 * timer interrupt). Full topology setup happens at smp_init()
	 * time - but meanwhile we still have a functioning scheduler.
	 */
	sched_init();
	/*
	 * Disable preemption - early bootup scheduling is extremely
	 * fragile until we cpu_idle() for the first time.
	 */
	preempt_disable();
	if (WARN(!irqs_disabled(),
		 "Interrupts were enabled *very* early, fixing it\n"))
		local_irq_disable();
	radix_tree_init();

	call_function_init();
	WARN(!irqs_disabled(), "Interrupts were enabled early\n");

}
