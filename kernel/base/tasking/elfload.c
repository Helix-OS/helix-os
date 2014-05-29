#include <base/tasking/task.h>
#include <base/tasking/elfload.h>

int elfload_from_mem( Elf32_Ehdr *header ){
	int ret = 0;
	Elf32_Half phindex;
	Elf32_Phdr *img_phdr;
	page_dir_t *newdir,
		   *olddir;

	olddir = get_current_page_dir( );
	newdir = clone_page_dir( olddir );
	set_page_dir( newdir );

	for ( phindex = 0; phindex < header->e_phnum; phindex++ ){
		img_phdr = get_elf_phdr( header, phindex );
		kprintf( "[%s] Have program header %d, vaddr: 0x%x, offset: 0x%x\n",
				__func__, phindex, img_phdr->p_vaddr, img_phdr->p_offset );

		// TODO: Add address space checking to prevent kernel space from being overwritten by malicious files
		map_pages( newdir, img_phdr->p_vaddr, img_phdr->p_vaddr + img_phdr->p_memsz,
				PAGE_USER | PAGE_WRITEABLE | PAGE_PRESENT );
		memcpy((void *)img_phdr->p_vaddr, (char *)header + img_phdr->p_offset, img_phdr->p_filesz );
	}

	create_thread( (void (*)())header->e_entry );

	set_page_dir( olddir );

	return ret;
}
