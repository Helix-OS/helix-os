#include <base/tasking/task.h>
#include <base/tasking/elfload.h>
#include <base/string.h>
#include <base/kstd.h>

int elfload_from_mem( Elf32_Ehdr *header, char *argv[], char *envp[] ){
	int ret = 0;
	Elf32_Half phindex;
	Elf32_Phdr *img_phdr;
	page_dir_t *newdir,
		   *olddir;

	int argc, envc;
	char **argbuf, **envbuf;
	int i;

	for ( argc = 0; argv[argc]; argc++ );
	for ( envc = 0; argv[envc]; envc++ );

	argbuf = knew( char *[argc + 1]);
	envbuf = knew( char *[envc + 1]);

	for ( i = 0; i < argc; i++ )
		argbuf[i] = strdup( argv[i] );
	argbuf[argc] = 0;

	for ( i = 0; i < envc; i++ )
		envbuf[i] = strdup( envp[i] );
	envbuf[envc] = 0;

	olddir = get_current_page_dir( );
	newdir = clone_page_dir( get_kernel_page_dir( ));
	set_page_dir( newdir );

	for ( phindex = 0; phindex < header->e_phnum; phindex++ ){
		img_phdr = get_elf_phdr( header, phindex );
		kprintf( "[%s] Have program header %d at 0x%x, vaddr: 0x%x, offset: 0x%x\n",
				__func__, phindex, img_phdr, img_phdr->p_vaddr, img_phdr->p_offset );

		// TODO: Add address space checking to prevent kernel space from being overwritten by malicious files
		map_pages( newdir, img_phdr->p_vaddr, img_phdr->p_vaddr + img_phdr->p_memsz,
				PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
		memcpy((void *)img_phdr->p_vaddr, (char *)header + img_phdr->p_offset, img_phdr->p_filesz );
	}

	//create_process( (void (*)())header->e_entry, argv, envp );
	create_process( (void (*)())header->e_entry, argbuf, envbuf );
	set_page_dir( olddir );

	for ( i = 0; i < argc; i++ )
		kfree( argbuf[i] );

	for ( i = 0; i < envc; i++ )
		kfree( envbuf[i] );

	kfree( argbuf );
	kfree( envbuf );

	return ret;
}
