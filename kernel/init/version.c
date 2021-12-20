/*
 *  kernel/init/version.c
 *
 *  Copyright (C) 1992  Theodore Ts'o
 *
 *  May be freely distributed as part of Linux.
 */

#include <generated/compile.h>
#include <generated/version.h>

#include <rtochius/build-salt.h>
#include <rtochius/uts.h>

#define version(a) Version_ ## a
#define version_string(a) version(a)

extern int version_string(RTOCHIUS_VERSION_CODE);
int version_string(RTOCHIUS_VERSION_CODE);

/* FIXED STRINGS! Don't touch! */
const char rtochius_banner[] =
	"Rtochius version " RTOCHIUS_VERSION_STRING " (" KERNEL_COMPILE_BY "@"
	KERNEL_COMPILE_HOST ") (" KERNEL_COMPILER ") " UTS_VERSION "\n";

BUILD_SALT;

const struct new_utsname utsname = {
	.sysname 	= UTS_SYSNAME,
	.nodename 	= UTS_NODENAME,
	.release	= RTOCHIUS_VERSION_STRING,
	.version	= UTS_VERSION,
	.machine	= UTS_MACHINE,
	.domainname	= UTS_DOMAINNAME,
};
