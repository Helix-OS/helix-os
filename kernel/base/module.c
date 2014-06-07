#include <base/module.h>
#include <base/logger.h>
#include <base/mem/alloc.h>
#include <base/lib/stdbool.h>

list_head_t *mod_list = 0;
int mods_inited = 0;

unsigned long mod_address;

typedef int (*init_func)( );

unsigned hash_string( char *str ){
	unsigned ret = 0, i;

	for ( i = 0; str[i]; i++ ){
		ret ^= str[i] * (i + 1);
		ret *= str[i];
	}

	return ret;
}

/*
 * The sections from the kernel image need to be loaded into a module struct
 * so that symbols in the modules can be defined.
 */
void init_module_system( multiboot_elf_t *elfinfo ){
	module_t *mod;
	Elf32_Ehdr *buf;
	Elf32_Shdr *shdr;
	int i;

	if ( elfinfo && !mods_inited ){
		buf = kmalloc( sizeof( Elf32_Ehdr ) + elfinfo->size * elfinfo->num );
		memset( buf, 0, sizeof( Elf32_Ehdr ));
		memcpy((void *)((unsigned long)buf + sizeof( Elf32_Ehdr )),
				(void *)elfinfo->addr, elfinfo->size * elfinfo->num );

		mod_list = list_create( 0 );

		buf->e_type  = ET_EXEC;
		buf->e_phoff = 0;
		buf->e_shoff = sizeof( Elf32_Ehdr );
		buf->e_ehsize = sizeof( Elf32_Ehdr );
		buf->e_phentsize = 0;
		buf->e_phnum = 0;
		buf->e_shentsize = elfinfo->size;
		buf->e_shnum = elfinfo->num;
		buf->e_shstrndx = elfinfo->shndx;

		mod = kmalloc( sizeof( module_t ));
		memset( mod, 0, sizeof( module_t ));
		mod->name 	= strdup( "base" );
		mod->elf_head 	= buf;
		mod->def_symtab = strdup( ".symtab" );

		mod->depends 	= dlist_create( 0, 0 );
		mod->links 	= dlist_create( 0, 0 );
		mod->symcache 	= hashmap_create( 32 );

		mod_address 	= 0xcf000000;
		list_add_data( mod_list, mod );

		for ( i = 0 ; i < elfinfo->num; i++ ){
			shdr = (Elf32_Shdr *)((unsigned long)buf +
				buf->e_shoff + buf->e_shentsize * i );

			kprintf( "Section: 0x%x\n", shdr->sh_addr );
		}

		kprintf( "Base kernel symbols loaded\n" );
	}

	mods_inited = 1;
}

/** \brief Loads the initial modules listed in "kernel/config/modtab" in the initrd.
 *  On failing to load a module, it continues and tries the next module listed.
 *  @param initmods The initrd object returned from \ref init_initrd.
 */
void load_init_modules( initrd_t *initmods ){
	tar_header_t *tarhead;
	Elf32_Ehdr *elf_buf;
	char *namebuf;
	char *modtab;
	int i, j;
	unsigned size;

	if ( initmods ){
		kprintf( "Initmods at 0x%x\n", initmods );

		tarhead = initrd_get_file( initmods, "kernel/config/modtab" );
		if ( tarhead ){
			modtab = (char *)(tarhead + 1);
			size = initrd_get_size( tarhead );
			namebuf = knew( char[128] );

			for ( i = 0; i < size; ){
				for ( j = 0; modtab[i + j] != '\n' && j < size; j++ )
					namebuf[j] = modtab[i + j];

				namebuf[j] = 0;

				kprintf( "[%s] Have module \"%s\"\n", __func__, namebuf );
				tarhead = initrd_get_file( initmods, namebuf );
				if ( tarhead ){
					elf_buf = (Elf32_Ehdr *)( tarhead + 1 );
					load_module( elf_buf );

				} else {
					kprintf( "[%s] Could not find module \"%s\"\n", __func__, namebuf );
				}

				i += j + 1;
			}

			kfree( namebuf );

		} else {
			kprintf( "[load_init_modules] Could not find modtab file, can't load modules\n" );
		}

	} else {
		kprintf( "[load_init_modules] Was passed a null pointer, Can't load modules\n" );
	}
}

module_t *get_module( char *name ){
	module_t 	*ret = 0,
			*mod;
	list_node_t  	*move = mod_list->base;

	foreach_in_list( move ){
		mod = move->data;

		if ( strcmp( mod->name, name ) == 0 ){
			ret = mod;
			break;
		}
	}

	return ret;
}

void dump_modules( ){
	list_node_t *move = mod_list->base;
	module_t *mod, *depmod;
	int i, alloced;

	foreach_in_list( move ){
		mod = move->data;
		kprintf( "\"%s\" at 0x%x", mod->name, mod->address );

		if ( dlist_used( mod->depends )){
			kprintf( ", depends on " );

			alloced = dlist_allocated( mod->depends );
			for ( i = 0; i < alloced; i++ ){
				depmod = dlist_get( mod->depends, i );
				if ( depmod )
					kprintf( "%s, ", depmod->name );
			}
		}

		kprintf( "\n" );
	}
}

void *get_real_symbol_address( module_t *mod, char *name ){
	Elf32_Sym 	*sym;
	void 		*ret = 0;
	unsigned 	nhash = hash_string( name );

	if ( mod && mod->elf_head ){
		if (( ret = hashmap_get( mod->symcache, nhash )) == 0 ){
			sym = get_elf_sym_byname( mod->elf_head, name, mod->def_symtab );

			if ( sym && sym->st_shndx != SHN_UNDEF ){
				ret = (void *)( mod->address + sym->st_value );
				hashmap_add( mod->symcache, nhash, ret );
			}
		}
	}

	return ret;
}

bool module_load_depends( module_t *mod, char **depends ){
	bool ret = true;
	module_t *depmod;
	int i;

	for ( i = 0; depends[i]; i++ ){
		depmod = get_module( depends[i] );
		if ( !depmod ){
			ret = false;
			break;
		}

		dlist_add( mod->depends, depmod );
	}

	return ret;
}

void module_link_depends( module_t *mod ){
	module_t *depmod;
	int i,
	    alloced;

	alloced = dlist_allocated( mod->depends );
	for ( i = 0; i < alloced; i++ ){
		depmod = dlist_get( mod->depends, i );

		if ( depmod )
			dlist_add( depmod->links, mod );
	}
}

bool module_link_symbol( module_t *mod, char *name, Elf32_Rel *rel ){
	bool ret = false;
	unsigned *addr,
		 *calc;
	unsigned i,
		 alloced;
	module_t *depmod;

	addr = get_real_symbol_address( mod, name );

	if ( !addr ){

		alloced = dlist_allocated( mod->depends );
		for ( i = 0; i < alloced && !addr; i++ ){
			depmod = dlist_get( mod->depends, i );
			if ( depmod )
				addr = get_real_symbol_address( depmod, name );
		}
	}

	if ( addr ){
		calc = (unsigned *)( mod->address + rel->r_offset );

		if ( ELF32_R_TYPE( rel->r_info ) == R_386_32 ){
			*calc = (unsigned)addr;
		} else if ( ELF32_R_TYPE( rel->r_info ) == R_386_PC32 ){
			*calc = ((unsigned)addr - mod->address) + *calc - rel->r_offset;
		}

		ret = true;
	}

	return ret;
}

int load_module( Elf32_Ehdr *elf_obj ){
	char 		*buf = (char *)elf_obj->e_ident,
			**depends,
			*temp,
			**provides;
	Elf32_Phdr 	*phdr;
	Elf32_Shdr 	*shdr;
	Elf32_Rel 	*rel;
	Elf32_Sym 	*sym;
	module_t 	*new_mod;
	int 		i,
			j,
			flags;
	unsigned 	*calc;
	unsigned long 	end_addr;
	extern unsigned *kernel_dir;
	init_func 	mod_init;

	if ( !( buf[0] == 0x7f && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F' )){
		kprintf( "Invalid ELF object, could not load.\n" );
		goto error;
	}

	if ( elf_obj->e_type != ET_DYN ){
		kprintf( "Cannot load non-dynamic elf object as module\n" );
		goto error;
	}

	new_mod = kmalloc( sizeof( module_t ));
	memset( new_mod, 0, sizeof( module_t ));

	new_mod->depends = dlist_create( 0, 0 );
	new_mod->links = dlist_create( 0, 0 );
	new_mod->symcache = hashmap_create( 32 );

	new_mod->address = mod_address;
	new_mod->elf_head = elf_obj;
	new_mod->def_symtab = strdup( ".dynsym" );

	for ( i = 0; i < elf_obj->e_phnum; i++ ){
		phdr = get_elf_phdr( elf_obj, i );
		if ( phdr && phdr->p_type == PT_LOAD ){

			end_addr = phdr->p_vaddr + phdr->p_memsz + 
				( PAGE_SIZE - ( phdr->p_vaddr + phdr->p_memsz )) % PAGE_SIZE % PAGE_SIZE;

			flags = ((phdr->p_flags & PF_W)? PAGE_WRITEABLE : 0 ) | PAGE_PRESENT;
			map_pages( kernel_dir, 	new_mod->address + ( phdr->p_vaddr & ~(PAGE_SIZE - 1)),
						new_mod->address + end_addr, flags );

			memcpy((void *)( new_mod->address + phdr->p_vaddr ),
					(void *)((unsigned)elf_obj + phdr->p_offset ),
					phdr->p_filesz );
			mod_address = new_mod->address + end_addr;
		}
	}

	// TODO: - Implement proper relocation
	// 	 - don't assume a .rel.dyn section exists
	// 	 - possibly move to a separate function
	shdr = get_elf_shdr_byname( elf_obj, ".rel.dyn" );

	for ( i = 0; shdr && i < shdr->sh_size / shdr->sh_entsize; i++ ){
		rel = (Elf32_Rel *)((unsigned)elf_obj + shdr->sh_offset + shdr->sh_entsize * i );
		if ( ELF32_R_TYPE( rel->r_info ) == R_386_RELATIVE ){
			calc = (unsigned *)( new_mod->address + rel->r_offset );
			*calc = *calc + new_mod->address;
		} 
	}

	provides = get_real_symbol_address( new_mod, "provides" );
	if ( !provides ){
		kprintf( "[load_module] Could not load module, does not have \"provides\" symbol defined.\n" );
		goto error_free;
	}

	//kprintf( "Provides: 0x%x:%s\n", provides, *provides );
	new_mod->name = *provides;
	kprintf( "[%s] Loading module \"%s\"\n", __func__, new_mod->name );

	depends = get_real_symbol_address( new_mod, "depends" );
	if ( !depends ){
		kprintf( "[load_module] Could not load module, does not have \"depends\" symbol defined.\n" );
		goto error_free;
	}

	//kprintf( "depends: 0x%x:%s\n", depends, depends[0] );

	if ( module_load_depends( new_mod, depends ) == false ){
		kprintf( "[%s] Could not load module, could not resolve all dependancies\n", __func__ );
		goto error_free;
	}

	//kprintf( "Resolving symbols for %s...  ", *provides );

	for ( i = 0; shdr && i < shdr->sh_size / shdr->sh_entsize; i++ ){
		rel = (Elf32_Rel *)((unsigned)elf_obj + shdr->sh_offset + shdr->sh_entsize * i );

		if ( ELF32_R_TYPE( rel->r_info ) == R_386_32 ||
				ELF32_R_TYPE( rel->r_info ) == R_386_PC32 ){

			j = ELF32_R_SYM( rel->r_info );
			if ( j )
				sym = get_elf_sym( elf_obj, j, new_mod->def_symtab );

			temp = get_elf_sym_name( elf_obj, sym, new_mod->def_symtab );

			if ( module_link_symbol( new_mod, temp, rel ) == false ){
				kprintf( "[load_module] module \"%s\":"
					 "Could not resolve symbol \"%s\"\n", *provides, temp );

				goto error_free;
			}
		} 
	}

	mod_init = (init_func)get_real_symbol_address( new_mod, "init" );
	if ( !mod_init ){
		kprintf( "[load_module] module \"%s\" has no init function\n", *provides );
		goto error_free;
	}

	kprintf( "[load_module] Launching init function at 0x%x\n", mod_init );
	mod_init( );

	module_link_depends( new_mod );
	list_add_data( mod_list, new_mod );

	dump_modules( mod_list );

	return 0;

error_free:
	hashmap_free( new_mod->symcache );
	dlist_free( new_mod->depends );
	dlist_free( new_mod->links );
	kfree( new_mod->depends );
	kfree( new_mod->links );
	kfree( new_mod );

error:
	return -1;

}
