
kernel_sources(
	printf.c panic.c fork.c smp.c cpu.c softirq.c
	extable.c smpboot.c stop_machine.c
)

add_subdirectory(irq)
add_subdirectory(locking)
add_subdirectory(sched)
add_subdirectory(time)
