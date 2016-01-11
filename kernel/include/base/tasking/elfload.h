#ifndef _helix_elfloader_h
#define _helix_elfloader_h
#include <base/elf.h>

int elfload_from_mem( Elf32_Ehdr *header, char *argv[], char *envp[], int *fds );

#endif
