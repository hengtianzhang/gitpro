kernel_library()

kernel_library_sources(
	init-mm.c pgtable-generic.c mmap.c percpu.c memory.c
	page_alloc.c slab.c maccess.c
)
