/*
 *  from linux/kernel/printk.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 * Modified to make sys_syslog() more flexible: added commands to
 * return the last 4k of kernel messages, regardless of whether
 * they've been read or not.  Added option to suppress kernel printk's
 * to the console.  Added hook for sending the console messages
 * elsewhere, in preparation for a serial line console (someday).
 * Ted Ts'o, 2/11/93.
 * Modified for sysctl support, 1/8/97, Chris Horn.
 * Fixed SMP synchronization, 08/08/99, Manfred Spraul
 *     manfred@colorfullife.com
 * Rewrote bits to get rid of console_lock
 *	01Mar01 Andrew Morton
 */
#define pr_fmt(fmt) "printf: " fmt

#include <base/linkage.h>
#include <base/compiler.h>
#include <base/common.h>
#include <base/errno.h>

#include <rtochius/printf.h>
#include <rtochius/sched/clock.h>
#include <rtochius/serial.h>
#include <rtochius/spinlock.h>
#include <rtochius/param.h>

enum log_flags {
	LOG_NEWLINE	= 2,	/* text ended with a newline */
	LOG_PREFIX	= 4,	/* text started with a prefix */
	LOG_CONT	= 8,	/* text is a fragment of a continuation line */
};

struct printf_log {
	u64 ts_nsec;		/* timestamp in nanoseconds */
	u16 len;		/* length of entire record */
	u16 text_len;		/* length of text buffer */
	u16 dict_len;		/* length of dictionary buffer */
	u8 facility;		/* syslog facility */
	u8 flags:5;		/* internal record flags */
	u8 level:3;		/* syslog level */
}
#ifdef CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS
__packed __aligned(4)
#endif
;

/*
 * Default used to be hard-coded at 7, quiet used to be hardcoded at 4,
 * we're now allowing both to be set from kernel config.
 */
#define CONSOLE_LOGLEVEL_DEFAULT CONFIG_MESSAGE_LOGLEVEL_DEFAULT

static int console_printf = CONSOLE_LOGLEVEL_DEFAULT;
#define console_loglevel (console_printf)

static int __init loglevel(char *str)
{
	int newlevel;

	/*
	 * Only update loglevel value when a correct setting was passed,
	 * to prevent blind crashes (when loglevel being set to 0) that
	 * are quite hard to debug
	 */
	if (get_option(&str, &newlevel)) {
		console_loglevel = newlevel;
		return 0;
	}

	return -EINVAL;
}
early_param("loglevel", loglevel);

/*
 * The logbuf_lock protects kmsg buffer, indices, counters.  This can be taken
 * within the scheduler's rq lock. It must be released before calling
 * console_unlock() or anything else that might wake up a process.
 */
DEFINE_RAW_SPINLOCK(logbuf_lock);

/* index and sequence number of the first record stored in the buffer */
static u64 log_first_seq;
static u32 log_first_idx;

/* index and sequence number of the next record to store in the buffer */
static u64 log_next_seq;
static u32 log_next_idx;

/* the next printf record to write to the console */
static u64 console_seq;
static u32 console_idx;

/* the next printf record to read after the last 'clear' command */
static u64 clear_seq;
static u32 clear_idx;

#define PREFIX_MAX		32
#define LOG_LINE_MAX		(1024 - PREFIX_MAX)

#define LOG_LEVEL(v)		((v) & 0x07)
#define LOG_FACILITY(v)		((v) >> 3 & 0xff)

/* record buffer */
#define LOG_ALIGN __alignof__(struct printf_log)
#define __LOG_BUF_LEN (1 << CONFIG_LOG_BUF_SHIFT)
#define LOG_BUF_LEN_MAX (u32)(1 << 31)
static char __log_buf[__LOG_BUF_LEN] __aligned(LOG_ALIGN);
static char *log_buf = __log_buf;
static u32 log_buf_len = __LOG_BUF_LEN;

/* Return log buffer address */
char *log_buf_addr_get(void)
{
	return log_buf;
}

/* Return log buffer size */
u32 log_buf_len_get(void)
{
	return log_buf_len;
}

/* human readable text of the record */
static char *log_text(const struct printf_log *msg)
{
	return (char *)msg + sizeof(struct printf_log);
}

/* optional key/value pair dictionary attached to the record */
static char *log_dict(const struct printf_log *msg)
{
	return (char *)msg + sizeof(struct printf_log) + msg->text_len;
}

/* get record by index; idx must point to valid msg */
static struct printf_log *log_from_idx(u32 idx)
{
	struct printf_log *msg = (struct printf_log *)(log_buf + idx);

	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer.
	 */
	if (!msg->len)
		return (struct printf_log *)log_buf;
	return msg;
}

/* get next record; idx must point to valid msg */
static u32 log_next(u32 idx)
{
	struct printf_log *msg = (struct printf_log *)(log_buf + idx);

	/* length == 0 indicates the end of the buffer; wrap */
	/*
	 * A length == 0 record is the end of buffer marker. Wrap around and
	 * read the message at the start of the buffer as *this* one, and
	 * return the one after that.
	 */
	if (!msg->len) {
		msg = (struct printf_log *)log_buf;
		return msg->len;
	}
	return idx + msg->len;
}

/*
 * Check whether there is enough free space for the given message.
 *
 * The same values of first_idx and next_idx mean that the buffer
 * is either empty or full.
 *
 * If the buffer is empty, we must respect the position of the indexes.
 * They cannot be reset to the beginning of the buffer.
 */
static int logbuf_has_space(u32 msg_size, bool empty)
{
	u32 free;

	if (log_next_idx > log_first_idx || empty)
		free = max(log_buf_len - log_next_idx, log_first_idx);
	else
		free = log_first_idx - log_next_idx;

	/*
	 * We need space also for an empty header that signalizes wrapping
	 * of the buffer.
	 */
	return free >= msg_size + sizeof(struct printf_log);
}

static int log_make_free_space(u32 msg_size)
{
	while (log_first_seq < log_next_seq &&
	       !logbuf_has_space(msg_size, false)) {
		/* drop old messages until we have enough contiguous space */
		log_first_idx = log_next(log_first_idx);
		log_first_seq++;
	}

	if (clear_seq < log_first_seq) {
		clear_seq = log_first_seq;
		clear_idx = log_first_idx;
	}

	/* sequence numbers are equal, so the log buffer is empty */
	if (logbuf_has_space(msg_size, log_first_seq == log_next_seq))
		return 0;

	return -ENOMEM;
}

/* compute the message size including the padding bytes */
static u32 msg_used_size(u16 text_len, u16 dict_len, u32 *pad_len)
{
	u32 size;

	size = sizeof(struct printf_log) + text_len + dict_len;
	*pad_len = (-size) & (LOG_ALIGN - 1);
	size += *pad_len;

	return size;
}

/*
 * Define how much of the log buffer we could take at maximum. The value
 * must be greater than two. Note that only half of the buffer is available
 * when the index points to the middle.
 */
#define MAX_LOG_TAKE_PART 4
static const char trunc_msg[] = "<truncated>";

static u32 truncate_msg(u16 *text_len, u16 *trunc_msg_len,
			u16 *dict_len, u32 *pad_len)
{
	/*
	 * The message should not take the whole buffer. Otherwise, it might
	 * get removed too soon.
	 */
	u32 max_text_len = log_buf_len / MAX_LOG_TAKE_PART;
	if (*text_len > max_text_len)
		*text_len = max_text_len;
	/* enable the warning message */
	*trunc_msg_len = strlen(trunc_msg);
	/* disable the "dict" completely */
	*dict_len = 0;
	/* compute the size again, count also the warning message */
	return msg_used_size(*text_len + *trunc_msg_len, 0, pad_len);
}

/* insert record into the buffer, discard old ones, update heads */
static int log_store(int facility, int level,
		     enum log_flags flags, u64 ts_nsec,
		     const char *dict, u16 dict_len,
		     const char *text, u16 text_len)
{
	struct printf_log *msg;
	u32 size, pad_len;
	u16 trunc_msg_len = 0;

	/* number of '\0' padding bytes to next message */
	size = msg_used_size(text_len, dict_len, &pad_len);

	if (log_make_free_space(size)) {
		/* truncate the message if it is too long for empty buffer */
		size = truncate_msg(&text_len, &trunc_msg_len,
				    &dict_len, &pad_len);
		/* survive when the log buffer is too small for trunc_msg */
		if (log_make_free_space(size))
			return 0;
	}

	if (log_next_idx + size + sizeof(struct printf_log) > log_buf_len) {
		/*
		 * This message + an additional empty header does not fit
		 * at the end of the buffer. Add an empty header with len == 0
		 * to signify a wrap around.
		 */
		memset(log_buf + log_next_idx, 0, sizeof(struct printf_log));
		log_next_idx = 0;
	}

	/* fill message */
	msg = (struct printf_log *)(log_buf + log_next_idx);
	memcpy(log_text(msg), text, text_len);
	msg->text_len = text_len;
	if (trunc_msg_len) {
		memcpy(log_text(msg) + text_len, trunc_msg, trunc_msg_len);
		msg->text_len += trunc_msg_len;
	}
	memcpy(log_dict(msg), dict, dict_len);
	msg->dict_len = dict_len;
	msg->facility = facility;
	msg->level = level & 7;
	msg->flags = flags & 0x1f;
	if (ts_nsec > 0)
		msg->ts_nsec = ts_nsec;
	else
		msg->ts_nsec = local_clock();
	memset(log_dict(msg) + dict_len, 0, pad_len);
	msg->len = size;

	/* insert message */
	log_next_idx += msg->len;
	log_next_seq++;

	return msg->text_len;
}

static bool suppress_message_printing(int level)
{
	return (level > console_loglevel);
}

static size_t print_syslog(unsigned int level, char *buf)
{
	/* TODO service task comm */
	return sprintf(buf, "<%u>", level);
}

static size_t print_time(u64 ts, char *buf)
{
	unsigned long rem_nsec = do_div(ts, 1000000000);

	return sprintf(buf, "[%5lu.%06lu] ",
		       (unsigned long)ts, rem_nsec / 1000);
}

static size_t print_prefix(const struct printf_log *msg, bool syslog,
			   bool time, char *buf)
{
	size_t len = 0;

	if (syslog)
		len = print_syslog((msg->facility << 3) | msg->level, buf);
	if (time)
		len += print_time(msg->ts_nsec, buf + len);
	return len;
}

static size_t msg_print_text(const struct printf_log *msg, bool syslog,
			     bool time, char *buf, size_t size)
{
	const char *text = log_text(msg);
	size_t text_size = msg->text_len;
	size_t len = 0;
	char prefix[PREFIX_MAX];
	const size_t prefix_len = print_prefix(msg, syslog, time, prefix);

	do {
		const char *next = memchr(text, '\n', text_size);
		size_t text_len;

		if (next) {
			text_len = next - text;
			next++;
			text_size -= next - text;
		} else {
			text_len = text_size;
		}

		if (buf) {
			if (prefix_len + text_len + 1 >= size - len)
				break;

			memcpy(buf + len, prefix, prefix_len);
			len += prefix_len;
			memcpy(buf + len, text, text_len);
			len += text_len;
			buf[len++] = '\n';
		} else {
			/* SYSLOG_ACTION_* buffer size only calculation */
			len += prefix_len + text_len + 1;
		}

		text = next;
	} while (text);

	return len;
}

/*
 * Continuation lines are buffered, and not committed to the record buffer
 * until the line is complete, or a race forces it. The line fragments
 * though, are printed immediately to the consoles to ensure everything has
 * reached the console in case of a kernel crash.
 */
static struct cont {
	char buf[LOG_LINE_MAX];
	size_t len;			/* length == 0 means unused buffer */
	struct task_struct *owner;	/* task of first print*/
	u64 ts_nsec;			/* time of first print */
	u8 level;			/* log level of first message */
	u8 facility;			/* log facility of first message */
	enum log_flags flags;		/* prefix, newline flags */
} cont;

static void cont_flush(void)
{
	if (cont.len == 0)
		return;

	log_store(cont.facility, cont.level, cont.flags, cont.ts_nsec,
		  NULL, 0, cont.buf, cont.len);
	cont.len = 0;
}

static bool cont_add(int facility, int level, enum log_flags flags, const char *text, size_t len)
{
	/* If the line gets too long, split it up in separate records. */
	if (cont.len + len > sizeof(cont.buf)) {
		cont_flush();
		return false;
	}

	if (!cont.len) {
		cont.facility = facility;
		cont.level = level;
		cont.owner = current;
		cont.ts_nsec = local_clock();
		cont.flags = flags;
	}

	memcpy(cont.buf + cont.len, text, len);
	cont.len += len;

	/* The original flags come from the first line,
	 * but later continuations can add a newline.
	 */
	if (flags & LOG_NEWLINE) {
		cont.flags |= LOG_NEWLINE;
		cont_flush();
	}

	return true;
}

static size_t log_output(int facility, int level, enum log_flags lflags, const char *dict, size_t dictlen, char *text, size_t text_len)
{
	/*
	 * If an earlier line was buffered, and we're a continuation
	 * write from the same process, try to add it to the buffer.
	 */
	if (cont.len) {
		if (cont.owner == current && (lflags & LOG_CONT)) {
			if (cont_add(facility, level, lflags, text, text_len))
				return text_len;
		}
		/* Otherwise, make sure it's flushed */
		cont_flush();
	}

	/* Skip empty continuation lines that couldn't be added - they just flush */
	if (!text_len && (lflags & LOG_CONT))
		return 0;

	/* If it doesn't end in a newline, try to buffer the current line */
	if (!(lflags & LOG_NEWLINE)) {
		if (cont_add(facility, level, lflags, text, text_len))
			return text_len;
	}

	/* Store it in the record log */
	return log_store(facility, level, lflags, 0, dict, dictlen, text, text_len);
}

/* Must be called under logbuf_lock. */
static int vprintk_store(int facility, int level,
		  const char *dict, size_t dictlen,
		  const char *fmt, va_list args)
{
	static char textbuf[LOG_LINE_MAX];
	char *text = textbuf;
	size_t text_len;
	enum log_flags lflags = 0;

	/*
	 * The printf needs to come first; we need the syslog
	 * prefix which might be passed-in as a parameter.
	 */
	text_len = vscnprintf(text, sizeof(textbuf), fmt, args);

	/* mark and strip a trailing newline */
	if (text_len && text[text_len-1] == '\n') {
		text_len--;
		lflags |= LOG_NEWLINE;
	}

	/* strip kernel syslog prefix and extract log level or control flags */
	if (facility == 0) {
		int kern_level;

		while ((kern_level = printf_get_level(text)) != 0) {
			switch (kern_level) {
			case '0' ... '7':
				level = kern_level - '0';
				/* fallthrough */
			case 'd':	/* KERN_DEFAULT */
				lflags |= LOG_PREFIX;
				break;
			case 'c':	/* KERN_CONT */
				lflags |= LOG_CONT;
			}

			text_len -= 2;
			text += 2;
		}
	}

	if (level == LOGLEVEL_DEFAULT)
		level = console_loglevel;

	if (dict)
		lflags |= LOG_PREFIX|LOG_NEWLINE;

	return log_output(facility, level, lflags,
			  dict, dictlen, text, text_len);
}

#define CONSOLE_EXT_LOG_MAX	8192

static void console_write(void)
{
	static char text[LOG_LINE_MAX + PREFIX_MAX];

	for (;;) {
		struct printf_log *msg;
		size_t len;

		if (console_seq < log_first_seq) {
			len = sprintf(text,
				      "** %llu printk messages dropped **\n",
				      log_first_seq - console_seq);

			/* messages are gone, move to first one */
			console_seq = log_first_seq;
			console_idx = log_first_idx;
		} else {
			len = 0;
		}
skip:
		if (console_seq == log_next_seq)
			break;

		msg = log_from_idx(console_idx);
		if (suppress_message_printing(msg->level)) {
			/*
			 * Skip record we have buffered and already printed
			 * directly to the console when we received it, and
			 * record that has level above the console loglevel.
			 */
			console_idx = log_next(console_idx);
			console_seq++;
			goto skip;
		}

		len += msg_print_text(msg,
				0,
				1, text + len, sizeof(text) - len);
		console_idx = log_next(console_idx);
		console_seq++;
		earlycon_write(text, len);
	}
}

static asmlinkage int vprintk_emit(int facility, int level,
			    const char *dict, size_t dictlen,
			    const char *fmt, va_list args)
{
	unsigned long flags;
	int printed_len;
	u64 curr_log_seq;

	raw_spin_lock_irqsave(&logbuf_lock, flags);
	curr_log_seq = log_next_seq;
	printed_len = vprintk_store(facility, level, dict, dictlen, fmt, args);
	raw_spin_unlock_irqrestore(&logbuf_lock, flags);

	/*
	 * Disable preemption to avoid being preempted while holding
	 * console_sem which would prevent anyone from printing to
	 * console
	 */
	raw_spin_lock_irqsave(&logbuf_lock, flags);
	if (earlycon_device_available())
		console_write();
	raw_spin_unlock_irqrestore(&logbuf_lock, flags);

	return printed_len;
}

static int vprintk_default(const char *fmt, va_list args)
{
	int r;

	r = vprintk_emit(0, LOGLEVEL_DEFAULT, NULL, 0, fmt, args);

	return r;
}

static __printf(1, 0) int vprintf_func(const char *fmt, va_list args)
{

	return vprintk_default(fmt, args);
}

asmlinkage __visible int printf(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintf_func(fmt, args);
	va_end(args);

	return r;
}
