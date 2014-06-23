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
	//memmap_t *map = 0;
	list_head_t *map_list;

	int argc, envc;
	char **argbuf, **envbuf;
	int i;

	map_list = list_create( 0 );

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

		list_add_data( map_list,
			memmap_create( img_phdr->p_vaddr, img_phdr->p_vaddr + img_phdr->p_memsz,
				MEMMAP_TYPE_IMAGE,
				MEMMAP_PERM_READ | MEMMAP_PERM_WRITE | MEMMAP_PERM_EXEC | MEMMAP_PERM_USER ));
					
		memcpy((void *)img_phdr->p_vaddr, (char *)header + img_phdr->p_offset, img_phdr->p_filesz );

		// If the pages are read-only, remap them as such
		if (( img_phdr->p_flags & PF_W ) == 0 ){
			kprintf( "[%s] Remapping pages as read-only...\n", __func__ );
			remap_pages( newdir, img_phdr->p_vaddr, img_phdr->p_vaddr + img_phdr->p_memsz,
				PAGE_USER | PAGE_PRESENT );
		}
	}

	/** Map user heap */
	map_pages( newdir, 0xb0000000, 0xb0001000,
			PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
	list_add_data( map_list,
		memmap_create( 0xb0000000, 0xb0000010,
			MEMMAP_TYPE_IMAGE,
			MEMMAP_PERM_READ | MEMMAP_PERM_WRITE | MEMMAP_PERM_EXEC | MEMMAP_PERM_USER ));

	//create_process( (void (*)())header->e_entry, argv, envp );
	ret = create_process( (void (*)())header->e_entry, argbuf, envbuf, map_list );
	set_page_dir( olddir );

	for ( i = 0; i < argc; i++ )
		kfree( argbuf[i] );

	for ( i = 0; i < envc; i++ )
		kfree( envbuf[i] );

	kfree( argbuf );
	kfree( envbuf );

	return ret;
}
