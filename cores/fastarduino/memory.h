//   Copyright 2016-2020 Jean-Francois Poilpret
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

/// @cond api

/**
 * @file 
 * Utilities to check memory usage.
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "gpio.h"

/// @cond notdocumented
extern int __heap_start;
extern int* __brkval;
/// @endcond

/**
 * Contains a few utility method to deal with free SRAM memory.
 */
namespace memory
{
	/**
	 * Return the amount of free SRAM memory.
	 */
	int free_mem()
	{
		int v;
		return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	}

	/**
	 * Check if current free SRAM memory is above the provided minimum.
	 * This can be used to ensure the program has enough memory before a big
	 * allocation (dynamic or calling a function needing a big stack).
	 * @param minimum the minimum amount of free memory required
	 * @retval true if free memory is strictly above @p minimum
	 * @retval false if free memory is below @p minimum
	 */
	bool check_mem(int minimum)
	{
		return free_mem() > minimum;
	}

	/**
	 * Check if current free SRAM memory is above the provided minimum.
	 * In case free SRAM memory is under @p minimum, then the LED pin will be set,
	 * otherwise it will be cleared.
	 * @param minimum the minimum amount of free memory required
	 * 
	 * @sa board::DigitalPin::LED
	 */
	void alert_mem(int minimum)
	{
		if (check_mem(minimum))
			gpio::FastPinType<board::DigitalPin::LED>::clear();
		else
			gpio::FastPinType<board::DigitalPin::LED>::set();
	}
}

#endif /* MEMORY_H */
/// @endcond
