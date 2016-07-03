#include <util/delay_basic.h>

#include "time.hh"
#include "Board.hh"
#include "power.hh"

void (*Time::delay)(uint32_t ms) = Time::default_delay;

void Time::yield()
{
	Power::sleep();
}

inline static void DELAY(uint16_t us)
{
	_delay_loop_2((us * F_CPU) / 4000000L);
}

void Time::default_delay(uint32_t ms)
{
	while (ms--) DELAY(1000);
}
