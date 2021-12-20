#include <base/linkage.h>
#include <base/common.h>
#include <base/compiler.h>
#include <base/init.h>

#include <rtochius/irqflags.h>
#include <rtochius/smp.h>
#include <rtochius/stackprotector.h>

#ifdef CONFIG_STACKPROTECTOR
/*
 * Called when gcc's -fstack-protector feature is used, and
 * gcc detects corruption of the on-stack canary value
 */
__visible void __stack_chk_fail(void)
{
	panic("stack-protector: Kernel stack is corrupted in: %pB",
		__builtin_return_address(0));
}
#endif

void panic(const char *fmt, ...)
{
	int this_cpu;
	long len;
	static char buf[1024];
	va_list args;

	/*
	 * Disable local interrupts. This will prevent panic_smp_self_stop
	 * from deadlocking the first cpu that invokes the panic, since
	 * there is nothing to prevent an interrupt handler (that runs
	 * after setting panic_cpu) from invoking panic() again.
	 */
	local_irq_disable();

	this_cpu = raw_smp_processor_id();

	va_start(args, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	if (len && buf[len - 1] == '\n')
		buf[len - 1] = '\0';

	pr_emerg("Kernel panic cpu%d - not syncing: %s\n", this_cpu, buf);
	pr_emerg("---[ end Kernel panic cpu%d - not syncing: %s ]---\n", this_cpu, buf);
	local_irq_enable();
	for (;;);
}
