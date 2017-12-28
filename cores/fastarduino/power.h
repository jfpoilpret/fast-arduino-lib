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

/// @cond api

/**
 * @file
 * Simple power support for AVR MCU.
 */
#ifndef POWER_HH
#define POWER_HH

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "boards/board.h"

/**
 * Defines simple API to handle AVR power sleep modes.
 */
namespace power
{
	/**
	 * This class contains the API for handling power sleep modes.
	 * It is not aimed for instantiation, as all its methods are static.
	 */
	class Power
	{
	public:
		/// @cond notdocumented
		Power() = delete;
		/// @endcond

		/**
		 * Set the default sleep mode, that will be used by next calls to `Power::sleep()` and
		 * `time::yield()`.
		 * Before this method is called, the default mode is set to `board::SleepMode::IDLE`.
		 * @sa Power::sleep()
		 * @sa time::yield()
		 */
		static void set_default_mode(board::SleepMode mode)
		{
			default_mode_ = mode;
		}

		/**
		 * Enter power sleep mode as defined by `Power::set_default_mode()`.
		 * This method will return only when MCU is awakened (the awakening signals
		 * depend on the selected sleep mode).
		 * 
		 * If you want your program to enter a different sleep mode than the default,
		 * you should call `Power::sleep(board::SleepMode)` instead.
		 * 
		 * @sa power::set_default_mode()
		 * @sa power::sleep(board::SleepMode)
		 */
		static void sleep()
		{
			sleep(default_mode_);
		}

		/**
		 * Enter a specific power sleep mode.
		 * This method will return only when MCU is awakened (the awakening signals
		 * depend on the selected sleep mode).
		 * 
		 * If you want your program to enter the default sleep mode (as defined by 
		 * `Power::set_default_mode()`), you should call `Power::sleep()` instead.
		 * 
		 * @sa power::sleep()
		 */
		static void sleep(board::SleepMode mode)
		{
			set_sleep_mode((uint8_t) mode);
			cli();
			sleep_enable();
			sei();
			sleep_cpu();
			sleep_disable();
		}

	private:
		static board::SleepMode default_mode_;
	};
}

#endif /* POWER_HH */
/// @endcond
