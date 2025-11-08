move reqs (inline assembly, limine reqs) to /kernel/include/ports(.h&.c)
with /kernel/include/reqs(.h&.c)


remaking mem_manager with slab


if we have a fs the kernel should no longer load per keyboard_pull and so on,
it should load the module system which searches for all modules which
are all saved in the fs and then they get loaded and everything should work


pci,
simple disk driver for fat32 (block reading)
(sector reader, 512 b)
bps parser , cluster size
