#include <arch/pitimer.h>

static void (*pitimer_call)( ) = 0;
static unsigned long tick = 0;
static uint32_t current_freq = 0;

static void init_pitimer( uint32_t freq );

// Generic timer interface functions
void init_timer( ){
	init_pitimer( 1000 );
}

void register_timer_call( void (*call)( )){
	pitimer_call = call;
}

void unregister_timer_call( void (*call)( )){
	pitimer_call = 0;
}

unsigned long get_tick( ){
	return tick;
}

// pitimer-specific functions
static void pitimer_stub( ){
	tick++;
	//kprintf( "[pitimer_stub] %d\n", tick );

	if ( pitimer_call )
		pitimer_call( );

}

static void init_pitimer( uint32_t freq ){
	register_interrupt_handler( IRQ0, pitimer_stub );
	current_freq = freq;

	uint32_t divisor = PIT_FREQ / freq;
	uint8_t l = (uint8_t)( divisor & 0xff );
	uint8_t h = (uint8_t)( divisor >> 8 ) & 0xff;

	outb( 0x43, 0x36 );
	outb( 0x40, l );
	outb( 0x40, h );

	kprintf( "[init_pitimer] programmable interrupt controller initialized\n" );
}

void poll_usleep( uint32_t useconds ){
	uint32_t timer = useconds * current_freq / 1000;
	uint32_t start = tick;

	while ( tick - start < timer );
}

