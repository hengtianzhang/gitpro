/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_IRQ_H_
#define __RTOCHIUS_IRQ_H_

#include <rtochius/irqnr.h>
#include <rtochius/irqreturn.h>

#include <asm/cache.h>
#include <asm/ptrace.h>
#include <asm/irq.h>

/*
 * Return value for chip->irq_set_affinity()
 *
 * IRQ_SET_MASK_OK	- OK, core updates irq_common_data.affinity
 * IRQ_SET_MASK_NOCPY	- OK, chip did update irq_common_data.affinity
 * IRQ_SET_MASK_OK_DONE	- Same as IRQ_SET_MASK_OK for core. Special code to
 *			  support stacked irqchips, which indicates skipping
 *			  all descendent irqchips.
 */
enum {
	IRQ_SET_MASK_OK = 0,
	IRQ_SET_MASK_OK_NOCOPY,
	IRQ_SET_MASK_OK_DONE,
};

struct msi_desc;
struct irq_domain;

/**
 * struct irq_common_data - per irq data shared by all irqchips
 * @state_use_accessors: status information for irq chip functions.
 *			Use accessor functions to deal with it
 * @node:		node index useful for balancing
 * @handler_data:	per-IRQ data for the irq_chip methods
 * @affinity:		IRQ affinity on SMP. If this is an IPI
 *			related irq, then this is the mask of the
 *			CPUs to which an IPI can be sent.
 * @effective_affinity:	The effective IRQ affinity on SMP as some irq
 *			chips do not allow multi CPU destinations.
 *			A subset of @affinity.
 * @msi_desc:		MSI descriptor
 * @ipi_offset:		Offset of first IPI target cpu in @affinity. Optional.
 */
struct irq_common_data {
	unsigned int		state_use_accessors;

	void			*handler_data;
	struct msi_desc		*msi_desc;
	cpumask_var_t		affinity;
};

/**
 * struct irq_data - per irq chip data passed down to chip functions
 * @mask:		precomputed bitmask for accessing the chip registers
 * @irq:		interrupt number
 * @hwirq:		hardware interrupt number, local to the interrupt domain
 * @common:		point to data shared by all irqchips
 * @chip:		low level interrupt hardware access
 * @domain:		Interrupt translation domain; responsible for mapping
 *			between hwirq number and linux irq number.
 * @parent_data:	pointer to parent struct irq_data to support hierarchy
 *			irq_domain
 * @chip_data:		platform-specific per-chip private data for the chip
 *			methods, to allow shared chip implementations
 */
struct irq_data {
	u32			mask;
	unsigned int		irq;
	unsigned long		hwirq;
	struct irq_common_data	*common;
	struct irq_chip		*chip;
	struct irq_domain	*domain;
	struct irq_data		*parent_data;
	void			*chip_data;
};

/*
 * Bit masks for irq_common_data.state_use_accessors
 *
 * IRQD_TRIGGER_MASK		- Mask for the trigger type bits
 * IRQD_SETAFFINITY_PENDING	- Affinity setting is pending
 * IRQD_ACTIVATED		- Interrupt has already been activated
 * IRQD_NO_BALANCING		- Balancing disabled for this IRQ
 * IRQD_PER_CPU			- Interrupt is per cpu
 * IRQD_AFFINITY_SET		- Interrupt affinity was set
 * IRQD_LEVEL			- Interrupt is level triggered
 * IRQD_WAKEUP_STATE		- Interrupt is configured for wakeup
 *				  from suspend
 * IRDQ_MOVE_PCNTXT		- Interrupt can be moved in process
 *				  context
 * IRQD_IRQ_DISABLED		- Disabled state of the interrupt
 * IRQD_IRQ_MASKED		- Masked state of the interrupt
 * IRQD_IRQ_INPROGRESS		- In progress state of the interrupt
 * IRQD_WAKEUP_ARMED		- Wakeup mode armed
 * IRQD_FORWARDED_TO_VCPU	- The interrupt is forwarded to a VCPU
 * IRQD_AFFINITY_MANAGED	- Affinity is auto-managed by the kernel
 * IRQD_IRQ_STARTED		- Startup state of the interrupt
 * IRQD_MANAGED_SHUTDOWN	- Interrupt was shutdown due to empty affinity
 *				  mask. Applies only to affinity managed irqs.
 * IRQD_SINGLE_TARGET		- IRQ allows only a single affinity target
 * IRQD_DEFAULT_TRIGGER_SET	- Expected trigger already been set
 * IRQD_CAN_RESERVE		- Can use reservation mode
 */
enum {
	IRQD_TRIGGER_MASK		= 0xf,
	IRQD_SETAFFINITY_PENDING	= (1 <<  8),
	IRQD_ACTIVATED			= (1 <<  9),
	IRQD_NO_BALANCING		= (1 << 10),
	IRQD_PER_CPU			= (1 << 11),
	IRQD_AFFINITY_SET		= (1 << 12),
	IRQD_LEVEL			= (1 << 13),
	IRQD_WAKEUP_STATE		= (1 << 14),
	IRQD_MOVE_PCNTXT		= (1 << 15),
	IRQD_IRQ_DISABLED		= (1 << 16),
	IRQD_IRQ_MASKED			= (1 << 17),
	IRQD_IRQ_INPROGRESS		= (1 << 18),
	IRQD_WAKEUP_ARMED		= (1 << 19),
	IRQD_FORWARDED_TO_VCPU		= (1 << 20),
	IRQD_AFFINITY_MANAGED		= (1 << 21),
	IRQD_IRQ_STARTED		= (1 << 22),
	IRQD_MANAGED_SHUTDOWN		= (1 << 23),
	IRQD_SINGLE_TARGET		= (1 << 24),
	IRQD_DEFAULT_TRIGGER_SET	= (1 << 25),
	IRQD_CAN_RESERVE		= (1 << 26),
};

#define __irqd_to_state(d)	((d)->common->state_use_accessors)

static inline bool irqd_is_setaffinity_pending(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_SETAFFINITY_PENDING;
}

static inline bool irqd_is_per_cpu(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_PER_CPU;
}

static inline bool irqd_can_balance(struct irq_data *d)
{
	return !(__irqd_to_state(d) & (IRQD_PER_CPU | IRQD_NO_BALANCING));
}

static inline bool irqd_affinity_was_set(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_AFFINITY_SET;
}

static inline void irqd_mark_affinity_was_set(struct irq_data *d)
{
	__irqd_to_state(d) |= IRQD_AFFINITY_SET;
}

static inline bool irqd_trigger_type_was_set(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_DEFAULT_TRIGGER_SET;
}

static inline u32 irqd_get_trigger_type(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_TRIGGER_MASK;
}

/*
 * Must only be called inside irq_chip.irq_set_type() functions or
 * from the DT/ACPI setup code.
 */
static inline void irqd_set_trigger_type(struct irq_data *d, u32 type)
{
	__irqd_to_state(d) &= ~IRQD_TRIGGER_MASK;
	__irqd_to_state(d) |= type & IRQD_TRIGGER_MASK;
	__irqd_to_state(d) |= IRQD_DEFAULT_TRIGGER_SET;
}

static inline bool irqd_is_level_type(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_LEVEL;
}

/*
 * Must only be called of irqchip.irq_set_affinity() or low level
 * hieararchy domain allocation functions.
 */
static inline void irqd_set_single_target(struct irq_data *d)
{
	__irqd_to_state(d) |= IRQD_SINGLE_TARGET;
}

static inline bool irqd_is_single_target(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_SINGLE_TARGET;
}

static inline bool irqd_is_wakeup_set(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_WAKEUP_STATE;
}

static inline bool irqd_can_move_in_process_context(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_MOVE_PCNTXT;
}

static inline bool irqd_irq_disabled(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_IRQ_DISABLED;
}

static inline bool irqd_irq_masked(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_IRQ_MASKED;
}

static inline bool irqd_irq_inprogress(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_IRQ_INPROGRESS;
}

static inline bool irqd_is_wakeup_armed(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_WAKEUP_ARMED;
}

static inline bool irqd_is_forwarded_to_vcpu(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_FORWARDED_TO_VCPU;
}

static inline void irqd_set_forwarded_to_vcpu(struct irq_data *d)
{
	__irqd_to_state(d) |= IRQD_FORWARDED_TO_VCPU;
}

static inline void irqd_clr_forwarded_to_vcpu(struct irq_data *d)
{
	__irqd_to_state(d) &= ~IRQD_FORWARDED_TO_VCPU;
}

static inline bool irqd_affinity_is_managed(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_AFFINITY_MANAGED;
}

static inline bool irqd_is_activated(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_ACTIVATED;
}

static inline void irqd_set_activated(struct irq_data *d)
{
	__irqd_to_state(d) |= IRQD_ACTIVATED;
}

static inline void irqd_clr_activated(struct irq_data *d)
{
	__irqd_to_state(d) &= ~IRQD_ACTIVATED;
}

static inline bool irqd_is_started(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_IRQ_STARTED;
}

static inline bool irqd_is_managed_and_shutdown(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_MANAGED_SHUTDOWN;
}

static inline void irqd_set_can_reserve(struct irq_data *d)
{
	__irqd_to_state(d) |= IRQD_CAN_RESERVE;
}

static inline void irqd_clr_can_reserve(struct irq_data *d)
{
	__irqd_to_state(d) &= ~IRQD_CAN_RESERVE;
}

static inline bool irqd_can_reserve(struct irq_data *d)
{
	return __irqd_to_state(d) & IRQD_CAN_RESERVE;
}

#undef __irqd_to_state

static inline irq_hw_number_t irqd_to_hwirq(struct irq_data *d)
{
	return d->hwirq;
}

/**
 * struct irq_chip - hardware interrupt chip descriptor
 *
 * @parent_device:	pointer to parent device for irqchip
 * @name:		name for /proc/interrupts
 * @irq_startup:	start up the interrupt (defaults to ->enable if NULL)
 * @irq_shutdown:	shut down the interrupt (defaults to ->disable if NULL)
 * @irq_enable:		enable the interrupt (defaults to chip->unmask if NULL)
 * @irq_disable:	disable the interrupt
 * @irq_ack:		start of a new interrupt
 * @irq_mask:		mask an interrupt source
 * @irq_mask_ack:	ack and mask an interrupt source
 * @irq_unmask:		unmask an interrupt source
 * @irq_eoi:		end of interrupt
 * @irq_set_affinity:	Set the CPU affinity on SMP machines. If the force
 *			argument is true, it tells the driver to
 *			unconditionally apply the affinity setting. Sanity
 *			checks against the supplied affinity mask are not
 *			required. This is used for CPU hotplug where the
 *			target CPU is not yet set in the cpu_online_mask.
 * @irq_retrigger:	resend an IRQ to the CPU
 * @irq_set_type:	set the flow type (IRQ_TYPE_LEVEL/etc.) of an IRQ
 * @irq_set_wake:	enable/disable power-management wake-on of an IRQ
 * @irq_bus_lock:	function to lock access to slow bus (i2c) chips
 * @irq_bus_sync_unlock:function to sync and unlock slow bus (i2c) chips
 * @irq_cpu_online:	configure an interrupt source for a secondary CPU
 * @irq_cpu_offline:	un-configure an interrupt source for a secondary CPU
 * @irq_suspend:	function called from core code on suspend once per
 *			chip, when one or more interrupts are installed
 * @irq_resume:		function called from core code on resume once per chip,
 *			when one ore more interrupts are installed
 * @irq_pm_shutdown:	function called from core code on shutdown once per chip
 * @irq_calc_mask:	Optional function to set irq_data.mask for special cases
 * @irq_print_chip:	optional to print special chip info in show_interrupts
 * @irq_request_resources:	optional to request resources before calling
 *				any other callback related to this irq
 * @irq_release_resources:	optional to release resources acquired with
 *				irq_request_resources
 * @irq_compose_msi_msg:	optional to compose message content for MSI
 * @irq_write_msi_msg:	optional to write message content for MSI
 * @irq_get_irqchip_state:	return the internal state of an interrupt
 * @irq_set_irqchip_state:	set the internal state of a interrupt
 * @irq_set_vcpu_affinity:	optional to target a vCPU in a virtual machine
 * @ipi_send_single:	send a single IPI to destination cpus
 * @ipi_send_mask:	send an IPI to destination cpus in cpumask
 * @flags:		chip specific flags
 */
struct irq_chip {
	struct device	*parent_device;
	const char	*name;
	unsigned int	(*irq_startup)(struct irq_data *data);
	void		(*irq_shutdown)(struct irq_data *data);
	void		(*irq_enable)(struct irq_data *data);
	void		(*irq_disable)(struct irq_data *data);

	void		(*irq_ack)(struct irq_data *data);
	void		(*irq_mask)(struct irq_data *data);
	void		(*irq_mask_ack)(struct irq_data *data);
	void		(*irq_unmask)(struct irq_data *data);
	void		(*irq_eoi)(struct irq_data *data);

	int		(*irq_set_affinity)(struct irq_data *data, const struct cpumask *dest, bool force);
	int		(*irq_retrigger)(struct irq_data *data);
	int		(*irq_set_type)(struct irq_data *data, unsigned int flow_type);
	int		(*irq_set_wake)(struct irq_data *data, unsigned int on);

	void		(*irq_bus_lock)(struct irq_data *data);
	void		(*irq_bus_sync_unlock)(struct irq_data *data);

	void		(*irq_cpu_online)(struct irq_data *data);
	void		(*irq_cpu_offline)(struct irq_data *data);

	void		(*irq_suspend)(struct irq_data *data);
	void		(*irq_resume)(struct irq_data *data);
	void		(*irq_pm_shutdown)(struct irq_data *data);

	void		(*irq_calc_mask)(struct irq_data *data);

	void		(*irq_print_chip)(struct irq_data *data);
	int		(*irq_request_resources)(struct irq_data *data);
	void		(*irq_release_resources)(struct irq_data *data);

	void		(*irq_compose_msi_msg)(struct irq_data *data, struct msi_msg *msg);
	void		(*irq_write_msi_msg)(struct irq_data *data, struct msi_msg *msg);

	int		(*irq_get_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool *state);
	int		(*irq_set_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool state);

	int		(*irq_set_vcpu_affinity)(struct irq_data *data, void *vcpu_info);

	void		(*ipi_send_single)(struct irq_data *data, unsigned int cpu);
	void		(*ipi_send_mask)(struct irq_data *data, const struct cpumask *dest);

	unsigned long	flags;
};

/*
 * irq_chip specific flags
 *
 * IRQCHIP_SET_TYPE_MASKED:	Mask before calling chip.irq_set_type()
 * IRQCHIP_EOI_IF_HANDLED:	Only issue irq_eoi() when irq was handled
 * IRQCHIP_MASK_ON_SUSPEND:	Mask non wake irqs in the suspend path
 * IRQCHIP_ONOFFLINE_ENABLED:	Only call irq_on/off_line callbacks
 *				when irq enabled
 * IRQCHIP_SKIP_SET_WAKE:	Skip chip.irq_set_wake(), for this irq chip
 * IRQCHIP_ONESHOT_SAFE:	One shot does not require mask/unmask
 * IRQCHIP_EOI_THREADED:	Chip requires eoi() on unmask in threaded mode
 * IRQCHIP_SUPPORTS_LEVEL_MSI	Chip can provide two doorbells for Level MSIs
 */
enum {
	IRQCHIP_SET_TYPE_MASKED		= (1 <<  0),
	IRQCHIP_EOI_IF_HANDLED		= (1 <<  1),
	IRQCHIP_MASK_ON_SUSPEND		= (1 <<  2),
	IRQCHIP_ONOFFLINE_ENABLED	= (1 <<  3),
	IRQCHIP_SKIP_SET_WAKE		= (1 <<  4),
	IRQCHIP_ONESHOT_SAFE		= (1 <<  5),
	IRQCHIP_EOI_THREADED		= (1 <<  6),
	IRQCHIP_SUPPORTS_LEVEL_MSI	= (1 <<  7),
};

/*
 * The header file for kernel/core/irq/irqdesc.c
 */
#include <rtochius/irqdesc.h>

#ifndef NR_IRQS_LEGACY
# define NR_IRQS_LEGACY 0
#endif

#ifndef ARCH_IRQ_INIT_FLAGS
# define ARCH_IRQ_INIT_FLAGS	0
#endif

#define IRQ_DEFAULT_INIT_FLAGS	ARCH_IRQ_INIT_FLAGS

extern void irq_lock_sparse(void);
extern void irq_unlock_sparse(void);

extern int arch_probe_nr_irqs(void);
extern int early_irq_init(void);

int generic_handle_irq(unsigned int irq);

/*
 * Convert a HW interrupt number to a logical one using a IRQ domain,
 * and handle the result interrupt number. Return -EINVAL if
 * conversion failed. Providing a NULL domain indicates that the
 * conversion has already been done.
 */
int __handle_domain_irq(struct irq_domain *domain, unsigned int hwirq,
			bool lookup, struct pt_regs *regs);

static inline int handle_domain_irq(struct irq_domain *domain,
				    unsigned int hwirq, struct pt_regs *regs)
{
	return __handle_domain_irq(domain, hwirq, true, regs);
}

void irq_free_descs(unsigned int irq, unsigned int cnt);
static inline void irq_free_desc(unsigned int irq)
{
	irq_free_descs(irq, 1);
}

unsigned int arch_dynirq_lower_bound(unsigned int from);

int __irq_alloc_descs(int irq, unsigned int from, unsigned int cnt,
		      const struct irq_affinity_desc *affinity);

/* use macros to avoid needing export.h for THIS_MODULE */
#define irq_alloc_descs(irq, from, cnt)	\
	__irq_alloc_descs(irq, from, cnt, NULL)

#define irq_alloc_desc()			\
	irq_alloc_descs(-1, 0, 1)

#define irq_alloc_desc_at(at)		\
	irq_alloc_descs(at, at, 1)

#define irq_alloc_desc_from(from)		\
	irq_alloc_descs(-1, from, 1)

#define irq_alloc_descs_from(from, cnt)	\
	irq_alloc_descs(-1, from, cnt)

extern int irq_set_percpu_devid_partition(unsigned int irq,
					  const struct cpumask *affinity);
extern int irq_set_percpu_devid(unsigned int irq);
extern int irq_get_percpu_devid_partition(unsigned int irq,
					  struct cpumask *affinity);

/*
 * The header file for kernel/core/irq/handle.c
 */
extern void handle_bad_irq(struct irq_desc *desc);

extern irqreturn_t no_action(int cpl, void *dev_id);

extern void __irq_wake_thread(struct irq_desc *desc, struct irqaction *action);

extern irqreturn_t __handle_irq_event_percpu(struct irq_desc *desc, unsigned int *flags);
extern irqreturn_t handle_irq_event_percpu(struct irq_desc *desc);
extern irqreturn_t handle_irq_event(struct irq_desc *desc);

/*
 * Registers a generic IRQ handling function as the top-level IRQ handler in
 * the system, which is generally the first C code called from an assembly
 * architecture-specific interrupt handler.
 *
 * Returns 0 on success, or -EBUSY if an IRQ handler has already been
 * registered.
 */
extern int set_handle_irq(void (*handle_irq)(struct pt_regs *));

/*
 * Allows interrupt handlers to find the irqchip that's been registered as the
 * top-level IRQ handler.
 */
extern void (*handle_arch_irq)(struct pt_regs *);

/*
 * The header file for kernel/core/irq/handle.c
 */
/* Dummy irq-chip implementations: */
extern struct irq_chip no_irq_chip;
extern struct irq_chip dummy_irq_chip;
























#endif /* !__RTOCHIUS_IRQ_H_ */
