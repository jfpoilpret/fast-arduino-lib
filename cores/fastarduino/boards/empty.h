//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * Target-specific features and pins; this serves as an example, showing all
 * types but without any content.
 * For actual types for a specific target MCU (or Arduino board), please refer
 * to [specific targets support section](@ref supportedboards).
 */

#ifndef BOARDS_EMPTY_HH
#define BOARDS_EMPTY_HH

#include <stddef.h>

/**
 * Defines all types and constants specific to support a specific MCU target.
 * This serves only as an example here; for actual targets, refer to proper header 
 * files in `board` directory, or refer refer to
 * [specific targets support section](@ref supportedboards).
 */
namespace board
{
	/**
	 * Performs special initialization for the target MCU.
	 * This must be called first in your `main()` function, even `sei()`.
	 */
	inline static void init() {}

	/**
	 * Defines all available ports of the target MCU.
	 */
	enum class Port : uint8_t {};

	/**
	 * Defines all available digital input/output pins of the target MCU.
	 */
	enum class DigitalPin : uint8_t {};

	/**
	 * Defines available clocks of the target MCU, used for analog input.
	 */
	enum class AnalogClock : uint8_t {};

	/**
	 * Defines available voltage references of the target MCU, used for analog input.
	 */
	enum class AnalogReference : uint8_t {};
	
	/**
	 * Defines all available analog input pins of the target MCU.
	 */
	enum class AnalogPin : uint8_t {};
	
	/**
	 * Defines all digital output pins of target MCU, capable of PWM output.
	 */
	namespace PWMPin {};
	
	/**
	 * Defines all digital output pins of target MCU, usable as direct external interrupt pins.
	 */
	namespace ExternalInterruptPin {};

	/**
	 * Defines all digital output pins of target MCU, usable as pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin {};

	/**
	 * Defines all USART modules of target MCU. This may be empty e.g. for ATtiny MCU.
	 */
	enum class USART : uint8_t {};
	
	/**
	 * Defines all timers available for target MCU.
	 */
	enum class Timer : uint8_t {};
	
	/**
	 * Defines all available sleep modes for target MCU.
	 */
	enum class SleepMode : uint8_t {};
};

#endif /* BOARDS_EMPTY_HH */
/// @endcond
