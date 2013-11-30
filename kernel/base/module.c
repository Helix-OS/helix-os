#include <base/module.h>
#include <base/logger.h>

void init_module_system( ){
	kprintf( "Woot\n" );
}

void load_init_modules( mhead_t *initmods ){
	mtable_t *table;
	Elf32_Ehdr *elf_buf;
	char *blarg;
	int i, j;

	kprintf( "0x%x: 0x%x\n", initmods->entries, initmods->magic );

	table = (mtable_t *)((unsigned)initmods + initmods->table_offset );
	for ( i = 0; i < initmods->entries; i++ ){
		kprintf( "0x%x: 0x%x, 0x%x, 0x%x\n", table + i, table[i].magic, table[i].offset, table[i].len );
		elf_buf = (Elf32_Ehdr *)((unsigned)initmods + table[i].offset );

		load_module( elf_buf );
	}
}

int load_module( Elf32_Ehdr *elf_obj ){
	char *buf = elf_obj->e_ident;
	int i;

	for ( i = 0; i < EI_NIDENT; i++ )
		kprintf( "0x%x ", buf[i] );

	kprintf( "\n" );
	kprintf( "type: %d\n", elf_obj->e_type );

	return 0;
}
