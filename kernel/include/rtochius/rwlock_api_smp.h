#ifndef __RTOCHIUS_RWLOCK_API_SMP_H_
#define __RTOCHIUS_RWLOCK_API_SMP_H_

#ifndef __RTOCHIUS_SPINLOCK_API_SMP_H_
# error "please don't include this file directly"
#endif

/*
 * include/linux/rwlock_api_smp.h
 *
 * spinlock API declarations on SMP (and debug)
 * (implemented in kernel/spinlock.c)
 *
 * portions Copyright 2005, Red Hat, Inc., Ingo Molnar
 * Released under the General Public License (GPL).
 */

void __lockfunc _raw_read_lock(rwlock_t *lock);
void __lockfunc _raw_write_lock(rwlock_t *lock);
void __lockfunc _raw_read_lock_bh(rwlock_t *lock);
void __lockfunc _raw_write_lock_bh(rwlock_t *lock);
void __lockfunc _raw_read_lock_irq(rwlock_t *lock);
void __lockfunc _raw_write_lock_irq(rwlock_t *lock);
unsigned long __lockfunc _raw_read_lock_irqsave(rwlock_t *lock);
unsigned long __lockfunc _raw_write_lock_irqsave(rwlock_t *lock);
int __lockfunc _raw_read_trylock(rwlock_t *lock);
int __lockfunc _raw_write_trylock(rwlock_t *lock);
void __lockfunc _raw_read_unlock(rwlock_t *lock);
void __lockfunc _raw_write_unlock(rwlock_t *lock);
void __lockfunc _raw_read_unlock_bh(rwlock_t *lock);
void __lockfunc _raw_write_unlock_bh(rwlock_t *lock);
void __lockfunc _raw_read_unlock_irq(rwlock_t *lock);
void __lockfunc _raw_write_unlock_irq(rwlock_t *lock);
void __lockfunc
_raw_read_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
void __lockfunc
_raw_write_unlock_irqrestore(rwlock_t *lock, unsigned long flags);

static inline int __raw_read_trylock(rwlock_t *lock)
{
	preempt_disable();
	if (do_raw_read_trylock(lock)) {
		rwlock_acquire_read(&lock->dep_map, 0, 1, _RET_IP_);
		return 1;
	}
	preempt_enable();
	return 0;
}

static inline int __raw_write_trylock(rwlock_t *lock)
{
	preempt_disable();
	if (do_raw_write_trylock(lock)) {
		rwlock_acquire(&lock->dep_map, 0, 1, _RET_IP_);
		return 1;
	}
	preempt_enable();
	return 0;
}

/*
 * If lockdep is enabled then we use the non-preemption spin-ops
 * even on CONFIG_PREEMPT, because lockdep assumes that interrupts are
 * not re-enabled during lock-acquire (which the preempt-spin-ops do):
 */
static inline void __raw_read_lock(rwlock_t *lock)
{
	preempt_disable();
	rwlock_acquire_read(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_read_trylock, do_raw_read_lock);
}

static inline unsigned long __raw_read_lock_irqsave(rwlock_t *lock)
{
	unsigned long flags;

	local_irq_save(flags);
	preempt_disable();
	rwlock_acquire_read(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED_FLAGS(lock, do_raw_read_trylock, do_raw_read_lock,
			     do_raw_read_lock_flags, &flags);
	return flags;
}

static inline void __raw_read_lock_irq(rwlock_t *lock)
{
	local_irq_disable();
	preempt_disable();
	rwlock_acquire_read(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_read_trylock, do_raw_read_lock);
}

static inline void __raw_read_lock_bh(rwlock_t *lock)
{
	__local_bh_disable_ip(_RET_IP_, SOFTIRQ_LOCK_OFFSET);
	rwlock_acquire_read(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_read_trylock, do_raw_read_lock);
}

static inline unsigned long __raw_write_lock_irqsave(rwlock_t *lock)
{
	unsigned long flags;

	local_irq_save(flags);
	preempt_disable();
	rwlock_acquire(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED_FLAGS(lock, do_raw_write_trylock, do_raw_write_lock,
			     do_raw_write_lock_flags, &flags);
	return flags;
}

static inline void __raw_write_lock_irq(rwlock_t *lock)
{
	local_irq_disable();
	preempt_disable();
	rwlock_acquire(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_write_trylock, do_raw_write_lock);
}

static inline void __raw_write_lock_bh(rwlock_t *lock)
{
	__local_bh_disable_ip(_RET_IP_, SOFTIRQ_LOCK_OFFSET);
	rwlock_acquire(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_write_trylock, do_raw_write_lock);
}

static inline void __raw_write_lock(rwlock_t *lock)
{
	preempt_disable();
	rwlock_acquire(&lock->dep_map, 0, 0, _RET_IP_);
	LOCK_CONTENDED(lock, do_raw_write_trylock, do_raw_write_lock);
}


static inline void __raw_write_unlock(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_write_unlock(lock);
	preempt_enable();
}

static inline void __raw_read_unlock(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_read_unlock(lock);
	preempt_enable();
}

static inline void
__raw_read_unlock_irqrestore(rwlock_t *lock, unsigned long flags)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_read_unlock(lock);
	local_irq_restore(flags);
	preempt_enable();
}

static inline void __raw_read_unlock_irq(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_read_unlock(lock);
	local_irq_enable();
	preempt_enable();
}

static inline void __raw_read_unlock_bh(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_read_unlock(lock);
	__local_bh_enable_ip(_RET_IP_, SOFTIRQ_LOCK_OFFSET);
}

static inline void __raw_write_unlock_irqrestore(rwlock_t *lock,
					     unsigned long flags)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_write_unlock(lock);
	local_irq_restore(flags);
	preempt_enable();
}

static inline void __raw_write_unlock_irq(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_write_unlock(lock);
	local_irq_enable();
	preempt_enable();
}

static inline void __raw_write_unlock_bh(rwlock_t *lock)
{
	rwlock_release(&lock->dep_map, 1, _RET_IP_);
	do_raw_write_unlock(lock);
	__local_bh_enable_ip(_RET_IP_, SOFTIRQ_LOCK_OFFSET);
}

#endif /* !__RTOCHIUS_RWLOCK_API_SMP_H_ */
