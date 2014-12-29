#ifndef _helix_pitimer_h
#define _helix_pitimer_h
#include <base/arch/common/timer.h>
#include <base/stdint.h>
#include <base/stdio.h>
#include <arch/isr.h>
#include <base/logger.h>

#define PIT_FREQ 1193180

void pollusleep( uint32_t );

#endif
