//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * PWM API.
 */
#ifndef PWM_H
#define PWM_H

#include "boards/board.h"
#include "boards/board_traits.h"
#include "gpio.h"
#include "timer.h"

namespace board
{
	template<PWMPin PWM> constexpr DigitalPin PWM_PIN() INLINE;
	/**
	 * Convert an `PWMPin` to the matching `DigitalPin`.
	 */
	template<PWMPin PWM> constexpr DigitalPin PWM_PIN()
	{
		return board_traits::PWMPin_trait<PWM>::ACTUAL_PIN;
	}
};

namespace analog
{
	using timer::TimerOutputMode;

	/**
	 * Construct a new handler for a PWM output pin.
	 * @tparam PWMPIN_ the `board::PWMPin` to use as PWM output
	 * @tparam PULSED_ whether to use a `timer::PulseTimer` instead of a 
	 * `timer::Timer`; this is useful when e.g. you want to use a PWM pin to
	 * manage a servo motor, where pulses shall be limited to a few ms but 
	 * triggered every few dozen ms.
	 */
	template<board::PWMPin PWMPIN_, bool PULSED_ = false> class PWMOutput
	{
	public:
		/** The PWM pin for this PWMOutput. */
		static constexpr const board::PWMPin PWMPIN = PWMPIN_;
		/**
		 * Whether this PWMOutput uses a `timer::PulseTimer` instead of a 
		 * `timer::Timer`.
		 */
		static constexpr const bool PULSED = PULSED_;

	private:
		using TRAIT = board_traits::PWMPin_trait<PWMPIN>;
		using TIMER_TRAIT = board_traits::Timer_trait<TRAIT::TIMER>;

	public:
		PWMOutput(const PWMOutput&) = delete;
		PWMOutput& operator=(const PWMOutput&) = delete;
		
		/** The digital pin for this PWMOutput. */
		static constexpr const board::DigitalPin PIN = TRAIT::ACTUAL_PIN;

		/// @cond notdocumented
		static constexpr const uint8_t COM = TRAIT::COM;
		/// @endcond

		/**
		 * The actual `timer::Timer` type associated to this `PWMOutput`.
		 */
		using TIMER = timer::Timer<TRAIT::TIMER>;

		/**
		 * Construct a new PWM output pin, connected to @p timer, using 
		 * @p output_mode.
		 * This will not compile if `PIN` is not a PWM pin or its is not
		 * connectable to @p timer.
		 * @param timer the timer to which this PWM pin must be connected
		 * @param output_mode the connection output mode to use; note this is not
		 * used in pulsed mode, i.e. when @p PULSED template argument is `true`.
		 */
		PWMOutput(TIMER& timer, TimerOutputMode output_mode = TimerOutputMode::NON_INVERTING) : timer_{timer}
		{
			// Initialize pin as output
			gpio::FastPinType<PIN>::set_mode(gpio::PinMode::OUTPUT);
			if (TIMER_TRAIT::IS_16BITS || (!PULSED))
				// Set com mode for pin
				timer_.template set_output_mode<COM>(output_mode);
		}

		/**
		 * Change the connection output mode of this PWM pin to its timer.
		 * @param output_mode the new connection output mode to use; note this 
		 * is not used in pulsed mode, i.e. when @p PULSED template argument is
		 * `true`.
		 */
		void set_output_mode(TimerOutputMode output_mode)
		{
			if (TIMER_TRAIT::IS_16BITS || (!PULSED)) timer_.template set_output_mode<COM>(output_mode);
		}

		/**
		 * The type (either `uint8_t` or `uint16_t`) of values acceptable for 
		 * `duty` in `set_duty()`.
		 * @sa set_duty()
		 * @sa MAX
		 */
		using TYPE = typename TIMER_TRAIT::TYPE;

		/**
		 * The maximum acceptable value for `duty` in `set_duty()`.
		 * @sa set_duty()
		 * @sa TYPE
		 */
		static constexpr const TYPE MAX = TIMER_TRAIT::MAX_PWM;

		/**
		 * Set the duty cycle for this PWM pin, from `0` (0% duty cycle) to
		 * `MAX` (100%), any value above `MAX` will be used as 100%.
		 * @param duty new duty cycle for this PWM pin
		 * @sa TYPE
		 * @sa MAX
		 */
		void set_duty(TYPE duty)
		{
			timer_.template set_max<COM>(duty);
		}

	private:
		TIMER& timer_;
	};
}

#endif /* PWM_H */
/// @endcond
