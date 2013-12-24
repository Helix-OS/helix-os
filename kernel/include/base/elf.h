#ifndef _helix_elf_h
#define _helix_elf_h

#define EI_NIDENT 16

#define ELF32_R_SYM( i ) ((i)>>8)
#define ELF32_R_TYPE( i ) ((unsigned char)(i))
#define ELF32_R_INFO( s, t ) (((s)<<8)+(unsigned char)(t))


typedef unsigned int 	Elf32_Addr;
typedef unsigned short 	Elf32_Half;
typedef unsigned int	Elf32_Off;
typedef signed int	Elf32_Sword;
typedef unsigned int	Elf32_Word;

typedef struct {
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

/** Types possible for e_type */
enum {
	ET_NONE	  = 0,
	ET_REL	  = 1,
	ET_EXEC   = 2,
	ET_DYN	  = 3,
	ET_CORE	  = 4,
	ET_LOPROC = 0xff00,
	ET_HIPROC = 0xffff
};

/** Values possible for e_machine */
enum { 
	EM_NONE,
	EM_M32,
	EM_SPARC,
	EM_386,
	EM_68K,
	EM_88K,
	EM_860,
	EM_MIPS
};

/** Values possible for e_version */
enum {
	EV_NONE,
	EV_CURRENT
};

/** Values for identification */
enum {
	EI_MAG0,
	EI_MAG1,
	EI_MAG2,
	EI_MAG3,
	EI_CLASS,
	EI_DATA,
	EI_VERSION,
	EI_PAD
};

/** Elf magic values */
enum {
	ELFMAG0	= 0x7f,
	ELFMAG1 = 'E',
	ELFMAG2	= 'L',
	ELFMAG3	= 'F'
};

/** Values for elf class */
enum {
	ELFCLASSNONE,
	ELFCLASS32,
	ELFCLASS64
};

/** Values for binary data encoding */
enum {
	ELFDATANONE,
	ELFDATA2LSB,
	ELFDATA2MSG
};

/** Section index values */
enum { 
	SHN_UNDEF,
	SHN_LORESERVE	= 0xff00,
	SHN_LOPROC	= 0xff00,
	SHN_HIPROC	= 0xff1f,
	SHN_ABS		= 0xfff1,
	SHN_COMMON	= 0xfff2,
	SHN_HIRESERVE	= 0xffff
};

/** Section header types */
enum { 
	SHT_NULL,
	SHT_PROGBITS,
	SHT_SYMTAB,
	SHT_STRTAB,
	SHT_RELA,
	SHT_HASH,
	SHT_DYNAMIC,
	SHT_NOTE,
	SHT_NOBITS,
	SHT_REL,
	SHT_SHLIB,
	SHT_DYNSYM,
	SHT_LOPROC =	0x70000000,
	SHT_HIPROC =	0x7fffffff,
	SHT_LOUSER =	0x80000000,
	SHT_HIUSER =	0xffffffff
};

/** Relocation types */
enum {
	R_386_NONE,
	R_386_32,
	R_386_PC32,
	R_386_GOT32,
	R_386_PLT32,
	R_386_COPY,
	R_386_GLOB_DAT,
	R_386_JMP_SLOT,
	R_386_RELATIVE,
	R_386_GOTOFF,
	R_386_GOTPC
}; 

enum {
	SHF_WRITE,
	SHF_ALLOC,
	SHF_EXECINSTR,
	SHF_MASKPROC
};

enum {
	PT_NULL,
	PT_LOAD,
	PT_DYNAMIC,
	PT_INTERP,
	PT_NOTE,
	PT_SHLIB,
	PT_PHDR,
	PT_LOPROC =	0x70000000,
	PT_HIPROC =	0x7fffffff
};

enum {
	PF_X 	= 0x1,
	PF_W 	= 0x2,
	PF_R 	= 0x4,
	PF_MASKPROC = 0xf0000000
};

/* Symbol types */
enum {
	STT_NOTYPE 	= 0,
	STT_OBJECT 	= 1,
	STT_FUNC 	= 2,
	STT_SECTION 	= 3,
	STT_FILE 	= 4,
	STT_LOPROC 	= 13,
	STT_HIPROC 	= 15
};

typedef struct {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr;

typedef struct {
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr;

typedef struct {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} Elf32_Sym;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_append;
} Elf32_Rela;

Elf32_Shdr *get_elf_shdr( Elf32_Ehdr *, unsigned );
Elf32_Shdr *get_elf_shdr_byname( Elf32_Ehdr *, char * );
Elf32_Phdr *get_elf_phdr( Elf32_Ehdr *, unsigned );

Elf32_Sym  *get_elf_sym( Elf32_Ehdr *, int, char * );
char 	   *get_elf_sym_name( Elf32_Ehdr *, Elf32_Sym *, char * );
Elf32_Sym  *get_elf_sym_byname( Elf32_Ehdr *, char *, char * );

#endif
