#ifndef _helix_common_timer_h
#define _helix_common_timer_h
#include <base/stdint.h>
#include <base/stdio.h>
#include <base/logger.h>

void init_timer( );

// TODO: have the register and unregister functions use the list interface
void register_timer_call( void (*call)( ));
void unregister_timer_call( void (*call)( ));
unsigned long get_tick( );

#endif
