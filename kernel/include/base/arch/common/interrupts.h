#ifndef _helix_i586_interrupts_h
#define _helix_i586_interrupts_h

static inline void disable_interrupts( ){
	asm volatile( "cli" );
}

static inline void enable_interrupts( ){
	asm volatile( "cli" );
}

static inline void debug_registers( ){
	asm volatile( "int $0x30" );
}

#endif
