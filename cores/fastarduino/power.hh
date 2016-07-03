#ifndef POWER_HH
#define	POWER_HH

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "Board.hh"

class Power
{
public:
	static void set_default_mode(Board::SleepMode mode)
	{
		if (mode != Board::SleepMode::DEFAULT_MODE)
			_default_mode = mode;
	}
	static void sleep()
	{
		sleep(_default_mode);
	}
	
	static void sleep(Board::SleepMode mode)
	{
		set_sleep_mode((uint8_t) mode);
		cli();
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
	
private:
	static Board::SleepMode _default_mode;
};

#endif	/* POWER_HH */
