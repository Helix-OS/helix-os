#include <base/module.h>
#include <base/logger.h>
#include <base/mem/alloc.h>

module_t *mod_list = 0;
int mods_inited = 0;

unsigned long mod_address;

typedef int (*init_func)( );

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

		mod_list 	= mod;
		mod_address 	= 0xcf000000;

		for ( i = 0 ; i < elfinfo->num; i++ ){
			shdr = (Elf32_Shdr *)((unsigned long)buf +
				buf->e_shoff + buf->e_shentsize * i );

			kprintf( "Section: 0x%x\n", shdr->sh_addr );
		}

		kprintf( "Base kernel symbols loaded\n" );
	}

	mods_inited = 1;
}

void load_init_modules( mhead_t *initmods ){
	mtable_t *table;
	Elf32_Ehdr *elf_buf;
	int i;

	kprintf( "0x%x: 0x%x\n", initmods->entries, initmods->magic );

	table = (mtable_t *)((unsigned)initmods + initmods->table_offset );
	for ( i = 0; i < initmods->entries; i++ ){
		kprintf( "0x%x: 0x%x, 0x%x, 0x%x\n",
				table + i, table[i].magic, table[i].offset, table[i].len );

		elf_buf = (Elf32_Ehdr *)((unsigned)initmods + table[i].offset );

		load_module( elf_buf );
	}
}

module_t *get_module( char *name ){
	module_t 	*ret = 0,
			*move;

	for ( move = mod_list; move; move = move->next ){
		if ( strcmp( move->name, name ) == 0 ){
			ret = move;
			break;
		}
	}

	return ret;
}

void add_module( module_t *mod ){
	module_t *move;

	for ( move = mod_list; move->next; move = move->next );
	move->next = mod;
}

void dump_modules( ){
	module_t *move;

	move = mod_list;
	for ( ; move; move = move->next )
		kprintf( "\"%s\" at 0x%x\n", move->name, move->address );
}

void *get_real_symbol_address( module_t *mod, char *name ){
	Elf32_Sym 	*sym;
	void 		*ret = 0;

	if ( mod && mod->elf_head ){
		sym = get_elf_sym_byname( mod->elf_head, name, mod->def_symtab );
		if ( sym && sym->st_shndx != SHN_UNDEF )
			ret = (void *)( mod->address + sym->st_value );
		else
			kprintf( "[get_real_symbol_address] have undefined symbol \"%s\" (sym: 0x%x:0x%x, %s)\n",
					name, sym, sym? sym->st_shndx: 0, mod->def_symtab );
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
	module_t 	*new_mod,
			*depmod;
	int 		i,
			j,
			flags,
			could_load = 0;
	unsigned 	*calc,
			*addr;
	unsigned long 	end_addr;
	extern unsigned *kernel_dir;
	init_func 	mod_init;

	if ( !( buf[0] == 0x7f && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F' )){
		kprintf( "Invalid ELF object, could not load.\n" );
		return -1;
	}

	if ( elf_obj->e_type != ET_DYN ){
		kprintf( "Cannot load non-dynamic elf object as module\n" );
		return -1;
	}

	new_mod = kmalloc( sizeof( module_t ));
	memset( new_mod, 0, sizeof( module_t ));
	new_mod->address = mod_address;
	new_mod->elf_head = elf_obj;
	new_mod->def_symtab = strdup( ".dynsym" );

	for ( i = 0; i < elf_obj->e_phnum; i++ ){
		phdr = get_elf_phdr( elf_obj, i );
		if ( phdr && phdr->p_type == PT_LOAD ){
			kprintf( "Loading %d bytes from 0x%x to 0x%x\n", phdr->p_filesz, phdr->p_offset,
					phdr->p_vaddr + new_mod->address );

			end_addr = phdr->p_vaddr + phdr->p_memsz + 
				( PAGE_SIZE - ( phdr->p_vaddr + phdr->p_memsz )) % PAGE_SIZE % PAGE_SIZE;
			kprintf( "[load_module] end_addr: 0x%x\n", end_addr );

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
			kprintf( "Have relocation symbol of type R_386_RELATIVE: 0x%x\n", calc );
			*calc = *calc + new_mod->address;
		} 
	}

	kprintf( "[load_module] kprintf: 0x%x\n", get_real_symbol_address( get_module( "base" ), "kprintf" ));

	provides = get_real_symbol_address( new_mod, "provides" );
	if ( !provides ){
		kprintf( "[load_module] Could not load module, does not have \"provides\" symbol defined.\n" );
		kfree( new_mod );
		return -1;
	}
	kprintf( "Provides: 0x%x:%s\n", provides, *provides );
	new_mod->name = *provides;

	depends = get_real_symbol_address( new_mod, "depends" );
	if ( !depends ){
		kprintf( "[load_module] Could not load module, does not have \"depends\" symbol defined.\n" );
		kfree( new_mod );
		return -1;
	}
	kprintf( "depends: 0x%x:%s\n", depends, depends[0] );

	kprintf( "Resolving symbols for %s...  ", *provides );

	for ( i = 0; shdr && i < shdr->sh_size / shdr->sh_entsize; i++ ){
		rel = (Elf32_Rel *)((unsigned)elf_obj + shdr->sh_offset + shdr->sh_entsize * i );

		if ( ELF32_R_TYPE( rel->r_info ) == R_386_32 ||
				ELF32_R_TYPE( rel->r_info ) == R_386_PC32 ){

			j = ELF32_R_SYM( rel->r_info );
			if ( j )
				sym = get_elf_sym( elf_obj, j, new_mod->def_symtab );

			temp = get_elf_sym_name( elf_obj, sym, new_mod->def_symtab );

			//kprintf( "symbol j: %d: %s(0x%x):", j, temp, temp );
			addr = get_real_symbol_address( new_mod, temp );
			if ( !addr ){
				//kprintf( "[load_module] dumping current modules:\n" );
				//dump_modules( mod_list );
				for ( j = 0; depends[j] && !addr; j++ ){
					//kprintf( "[load_module] Trying next dependancy module \"%s\"\n", depends[j] );
					depmod = get_module( depends[j] );
					addr = get_real_symbol_address( depmod, temp );
				}

				if ( !addr ){
					kprintf( "[load_module] module \"%s\":"
						 "Could not resolve symbol \"%s\"\n", *provides, temp );
					could_load = 0;
					break;
				}
			} 
			
			if ( addr ){
				calc = (unsigned *)( new_mod->address + rel->r_offset );
				if ( ELF32_R_TYPE( rel->r_info ) == R_386_32 ){
					//kprintf( "[load_module] Have relocation symbol of type R_386_32: 0x%x\n", calc );
					*calc = (unsigned)addr;
				} else if ( ELF32_R_TYPE( rel->r_info ) == R_386_PC32 ){
					//kprintf( "[load_module] Have relocation symbol of type R_386_PC32: 0x%x\n", calc );
					*calc = ((unsigned)addr - new_mod->address) + *calc - rel->r_offset;
				}

				//kprintf( "[load_module] Set relocation address to 0x%x\n", *calc );
				could_load = 1;

			}
		} 
	}

	if ( could_load ){
		mod_init = (init_func)get_real_symbol_address( new_mod, "init" );
		if ( !mod_init ){
			kprintf( "[load_module] module \"%s\" has no init function\n", *provides );
			return -1;
		}

		kprintf( "[load_module] Launching init function at 0x%x\n", mod_init );
		mod_init( );

		//new_mod->name = strdup( *provides );
		add_module( new_mod );
	} else {
		kfree( new_mod );
		return -1;
	}

	dump_modules( mod_list );

	return 0;
}


