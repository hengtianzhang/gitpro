/*
 * Stack tracing support
 *
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <base/common.h>
#include <base/errno.h>

#include <rtochius/sched.h>
#include <rtochius/sched/task.h>

#include <asm/stacktrace.h>
#include <asm/stack_pointer.h>

/*
 * AArch64 PCS assigns the frame pointer to x29.
 *
 * A simple function prologue looks like this:
 * 	sub	sp, sp, #0x10
 *   	stp	x29, x30, [sp]
 *	mov	x29, sp
 *
 * A simple function epilogue looks like this:
 *	mov	sp, x29
 *	ldp	x29, x30, [sp]
 *	add	sp, sp, #0x10
 */
int unwind_frame(struct task_struct *tsk, struct stackframe *frame)
{
	unsigned long fp = frame->fp;

	if (fp & 0xf)
		return -EINVAL;

	if (!tsk)
		tsk = current;

	if (!on_accessible_stack(tsk, fp, NULL))
		return -EINVAL;

	frame->fp = READ_ONCE(*(unsigned long *)(fp));
	frame->pc = READ_ONCE(*(unsigned long *)(fp + 8));

	/*
	 * Frames created upon entry from EL0 have NULL FP and PC values, so
	 * don't bother reporting these. Frames created by __noreturn functions
	 * might have a valid FP even if PC is bogus, so only terminate where
	 * both are NULL.
	 */
	if (!frame->fp && !frame->pc)
		return -EINVAL;

	return 0;
}

void walk_stackframe(struct task_struct *tsk, struct stackframe *frame,
		     int (*fn)(struct stackframe *, void *), void *data)
{
	while (1) {
		int ret;

		if (fn(frame, data))
			break;
		ret = unwind_frame(tsk, frame);
		if (ret < 0)
			break;
	}
}
