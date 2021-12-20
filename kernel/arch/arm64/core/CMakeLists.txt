# SPDX-License-Identifier: Apache-2.0

kernel_library()

kernel_library_sources(
	head.S entry.S smccc-call.S traps.c cpuinfo.c init.c setup.c
	ioremap.c process.c cpu_ops.c psci.c cpufeature.c smp.c
	cpu_errata.c signal.c fpsimd.c insn.c irq.c syscall.c
	stacktrace.c
)

set_property(GLOBAL PROPERTY LINKER_SCRIPT_S "${CMAKE_CURRENT_LIST_DIR}/linker.lds.S")
