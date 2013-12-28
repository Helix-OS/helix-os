#include <base/elf.h>
#include <base/string.h>

Elf32_Shdr *get_elf_shdr( Elf32_Ehdr *ehdr, unsigned n ){
	Elf32_Shdr *ret;

	if ( n > ehdr->e_shnum )
		ret = 0;
	else
		ret = (Elf32_Shdr *)((unsigned)ehdr + ehdr->e_shoff + ehdr->e_shentsize * n );

	return ret;
}

Elf32_Phdr *get_elf_phdr( Elf32_Ehdr *ehdr, unsigned n ){
	Elf32_Phdr *ret = 0;

	if ( n < ehdr->e_phnum )
		ret = (Elf32_Phdr *)((unsigned)ehdr + ehdr->e_phoff + ehdr->e_phentsize * n );

	return ret;
}

Elf32_Shdr *get_elf_shdr_byname( Elf32_Ehdr *ehdr, char *name ){
	Elf32_Shdr	*ret = 0,
			*move,
			*temp;
	char *shstrtab;
	int i;

	temp = get_elf_shdr( ehdr, ehdr->e_shstrndx );
	//kprintf( "[get_elf_shdr_byname] e_shstrndx: 0x%x\n", temp->sh_addr );

	if ( temp ){
		if ( ehdr->e_type == ET_DYN ){
			shstrtab = (char *)((unsigned)ehdr + temp->sh_offset );

		} else if ( ehdr->e_type == ET_EXEC ){
			shstrtab = (char *)((unsigned)temp->sh_addr );

		}

		for ( i = 0; i < ehdr->e_shnum; i++ ){
			move = get_elf_shdr( ehdr, i );

			if ( strcmp( shstrtab + move->sh_name, name ) == 0 ){
				ret = move;
				break;
			}
		}
	}

	return ret;
}

Elf32_Sym *get_elf_sym( Elf32_Ehdr *ehdr, int n, char *section ){
	Elf32_Sym 	*ret = 0,
			*symtab;
	Elf32_Shdr	*shead;

	if ( ehdr->e_type == ET_DYN ){
		//shead = get_elf_shdr_byname( ehdr, ".symtab" );
		shead = get_elf_shdr_byname( ehdr, section );
		symtab = (Elf32_Sym *)((unsigned)ehdr + shead->sh_offset );

	} else if ( ehdr->e_type == ET_EXEC ){
		shead = get_elf_shdr_byname( ehdr, section );
		symtab = (Elf32_Sym *)( shead->sh_addr );

	}

	if ( !shead->sh_size || !shead->sh_entsize )
		return 0;


	if ( n < shead->sh_size / shead->sh_entsize ){
		ret = (Elf32_Sym *)((unsigned)symtab + n * shead->sh_entsize);
		//kprintf( "[get_elf_sym] ret: 0x%x, \n", ret );
	}

	return ret;
}

char *get_elf_sym_name( Elf32_Ehdr *ehdr, Elf32_Sym *sym, char *section ){
	Elf32_Shdr *shead;
	char *ret = 0,
	     *strtab,
	     tname[ strlen( section ) + 2 ];
	int i;

	// Find the name of the proper string table... Very hackish
	memcpy( &tname, section, strlen( section ) + 1);

	i = 0;
	while ( i < strlen( section ) - 2 ){
		if ( tname[i] == 's' && tname[i+1] == 'y' && tname[i+2] == 'm' ){
			tname[i+1] = 't';
			tname[i+2] = 'r';
			break;
		}
		i++;
	}

	shead = get_elf_shdr_byname( ehdr, tname );

	if ( ehdr->e_type == ET_DYN ){
		strtab = (char *)((unsigned)ehdr + shead->sh_offset );

	} else if ( ehdr->e_type == ET_EXEC ){
		strtab = (char *)( shead->sh_addr );

	}

	ret = strtab + sym->st_name;
	/*
	kprintf( "[get_elf_sym_name] \"%s\": shead: 0x%x, strtab: 0x%x, sym->st_name: 0x%x, returning \"%s\" (0x%x)\n",
			//tname, shead, strtab, sym->st_name, ret, ret );
			0, 0, 0, 0, 0, 0 );
			*/

	return ret;
}

Elf32_Sym *get_elf_sym_byname( Elf32_Ehdr *ehdr, char *name, char *section ){
	char *buf;
	Elf32_Sym 	*ret,
			*sym;
	int i;

	ret = 0;
	for ( i = 0; ; i++ ){
		if ( !( sym = get_elf_sym( ehdr, i, section )))
			break;

		//kprintf( "[get_elf_sym_byname] Trying sym 0x%x\n", sym->st_value );
		buf = get_elf_sym_name( ehdr, sym, section );
		if ( buf && strcmp( buf, name ) == 0 ){
			ret = sym;
			break;
		}
	}
	//kprintf( "\n" );

	return ret;
}

