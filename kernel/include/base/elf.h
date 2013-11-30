#ifndef _helix_elf_h
#define _helix_elf_h

#define EI_NIDENT 16

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

typedef struct {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr;

enum {
	SHF_WRITE,
	SHF_ALLOC,
	SHF_EXECINSTR,
	SHF_MASKPROC
};

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

#endif
