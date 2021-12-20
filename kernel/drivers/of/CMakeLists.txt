

kernel_library()

kernel_library_sources(
	base.c property.c device.c address.c
)

kernel_library_sources_ifdef(CONFIG_OF_FLATTREE
	fdt.c fdt_address.c
)

kernel_library_sources_ifdef(CONFIG_OF_RESERVED_MEM
	of_reserved_mem.c
)
