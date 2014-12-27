#ifndef _helix_pitimer_h
#define _helix_pitimer_h
#include <base/stdint.h>
#include <base/stdio.h>
#include <arch/isr.h>
#include <base/logger.h>

#define PIT_FREQ 1193180

void init_pitimer( uint32_t freq );
void register_pitimer_call( void (*call)( ));
void unregister_pitimer_call( void (*call)( ));
void pollusleep( uint32_t );
unsigned long get_tick( );

#endif
