# SPDX-License-Identifier: Apache-2.0

kernel_interface_library()

kernel_interface_library_sources(
	copy_page.S clear_page.S copy_from_user.S copy_to_user.S
	copy_in_user.S clear_user.S uaccess_flushcache.c
)
