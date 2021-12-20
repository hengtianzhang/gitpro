/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTOCHIUS_PARAM_H_
#define __RTOCHIUS_PARAM_H_

struct obs_kernel_param {
	const char *str;
	int (*setup_func)(char *);
};

#define __setup_param(str, unique_id, fn)		\
	static const char __setup_str_##unique_id[] __initconst	\
		__aligned(1) = str;						\
	static struct obs_kernel_param __setup_##unique_id	\
		__used __section(.init.setup)			\
		__attribute__((aligned((sizeof(long)))))		\
		= {  __setup_str_##unique_id, fn}

#define early_param(str, fn)					\
	__setup_param(str, fn, fn)

extern const struct obs_kernel_param __setup_start[], __setup_end[];

extern bool parameqn(const char *a, const char *b, size_t n);

extern bool parameq(const char *a, const char *b);

extern char *parse_args(const char *doing,
			char *args,
			void *arg,
			int (*unknown)(char *param, char *val,
					const char *doing, void *arg));

extern void parse_early_options(char *cmdline);
extern void parse_early_param(void);

typedef int (*initcall_t)(void);
typedef initcall_t initcall_entry_t;

#define ___define_initcall(fn, id, __sec) \
	static initcall_t __initcall_##fn##id __used \
		__attribute__((__section__(#__sec ".init"))) = fn;
#define __define_initcall(fn, id) ___define_initcall(fn, id, .initcall##id)

#define early_initcall(fn)		__define_initcall(fn, 0)
#define core_initcall(fn)		__define_initcall(fn, 1)
#define postcore_initcall(fn)	__define_initcall(fn, 2)
#define arch_initcall(fn)		__define_initcall(fn, 3)
#define subsys_initcall(fn)		__define_initcall(fn, 4)
#define device_initcall(fn)		__define_initcall(fn, 5)
#define userver_initcall(fn)	__define_initcall(fn, 6)
#define late_initcall(fn)		__define_initcall(fn, 7)

#endif /* !__RTOCHIUS_PARAM_H_ */
