//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef POWER_HH
#define	POWER_HH

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "boards/board.h"

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
