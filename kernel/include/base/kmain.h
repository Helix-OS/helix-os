#ifndef _helix_kmain_h
#define _helix_kmain_h
#include <base/multiboot.h>

//void kmain( unsigned flags, void *modules, multiboot_elf_t *elfinfo );
void kmain( unsigned flags, void *modules, multiboot_elf_t *elfinfo, char *cmdline );

#endif
