//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * API to handle servomotors.
 */
#ifndef SERVO_H
#define SERVO_H

#include <fastarduino/pulse_timer.h>
#include <fastarduino/pwm.h>
#include <fastarduino/utilities.h>

/**
 * Defines the API for servomotor support.
 */
namespace devices::servo
{
	/**
	 * This template class supports one servomotor connected to a PWM pin.
	 * Servomotors are driven by the width of pulses generated at a specific
	 * frequency (specific to each model, but typically around 50Hz, i.e. one
	 * pulse every 20ms), a specific width matches a specific rotation angle
	 * of the servo.
	 * 
	 * @tparam TIMER_ the type of timer used to handle the connected servomotor;
	 * this must be a `timer::PulseTimer` type.
	 * @tparam PWMPIN_ the `board::PWMPin` to which the servomotor is connected;
	 * this must be a PWM pin, connected to @p TIMER_.
	 * 
	 * @sa timer::PulseTimer
	 */
	template<typename TIMER_, board::PWMPin PWMPIN_> class Servo
	{
	public:
		/** The type of timer used to handle the connected servomotor. */
		using TIMER = TIMER_;
		/** The PWM pin for this PWMOutput. */
		static constexpr const board::PWMPin PWMPIN = PWMPIN_;
		/** The pin to which the servomotor is connected. */
		static constexpr const board::DigitalPin PIN = board_traits::PWMPin_trait<PWMPIN>::ACTUAL_PIN;
		/** The type of counter for @p TIMER_ */
		using TYPE = typename TIMER::TYPE;

	private:
		using CALC = typename TIMER::CALCULATOR;
		using TPRESCALER = typename CALC::PRESCALER;
		static constexpr const TPRESCALER PRESCALER = TIMER::PRESCALER;

	public:
		/**
		 * Create a new servo handler, based on the provided @p timer (which will
		 * provide the frequency for pulse generation), and the additional parameters
		 * for pulse width.
		 * 
		 * @param timer the `timer::PulseTimer` that will handle pulse generation
		 * for this servomotor
		 * @param us_minimum the minimal pulse width in microseconds; this matches
		 * the minimal angle of the servo.
		 * @param us_maximum the maximal pulse width in microseconds: this matches
		 * the maximal angle of the servo.
		 * @param us_neutral the pulse width, in microseconds, that matches the 
		 * 0 angle; when not provided (or `0`), it will be calculated as the
		 * average of @p us_minimum and @p us_maximum.
		 */
		Servo(TIMER& timer, uint16_t us_minimum, uint16_t us_maximum, uint16_t us_neutral = 0)
			: out_{timer}, US_MINIMUM_{us_minimum}, US_MAXIMUM_{us_maximum},
				US_NEUTRAL_{us_neutral ? us_neutral : ((us_maximum + us_minimum) / 2)}, 
				COUNTER_MINIMUM_{counter(US_MINIMUM_)},
				COUNTER_MAXIMUM_{counter(US_MAXIMUM_)}, COUNTER_NEUTRAL_{counter(US_NEUTRAL_)}
		{
		}

		/**
		 * Detach the servomotor from this handler. Concretely, this means that
		 * no pulse will be generated at all, i.e. the servo will not be able to
		 * hold its position anymore.
		 */
		inline void detach() INLINE
		{
			out_.set_duty(0);
		}

		/**
		 * Set the Timer counter that will change the pulse width, hence the servo
		 * angle.
		 * This method is the most optimized way to change the servo angle.
		 * However, it requires preliminary calculation of counter values from
		 * the desired angles. Calculation can be performed by `calculate_counter()`.
		 * 
		 * @param value the new counter value to use for fixing pulse width
		 * 
		 * @sa calculate_counter()
		 * @sa set_pulse()
		 * @sa rotate()
		 */
		inline void set_counter(TYPE value) INLINE
		{
			out_.set_duty(utils::constrain(value, COUNTER_MINIMUM_, COUNTER_MAXIMUM_));
		}

		/**
		 * Set the pulse width in microsecond, hence the servo angle.
		 * This method is less optimized way than `set_counter()` to change the
		 * servo angle.
		 * 
		 * @param pulse_us the new pulse width in microseconds
		 * 
		 * @sa set_counter()
		 * @sa rotate()
		 */
		inline void set_pulse(uint16_t pulse_us)
		{
			// Constrain pulse to min/max and convert pulse to timer counter value
			out_.set_duty(calculate_counter(pulse_us));
		}

		/**
		 * Rotate the servomotor at the given @p angle position.
		 * This method is less optimized than `set_counter()` and more efficient
		 * than `set_pulse()`, but it provides a more friendly API as it uses
		 * real physical parameters.
		 * 
		 * @param angle the new angle, in degrees, to rotate the servo to; it must
		 * be between `-90` and `+90`.
		 */
		inline void rotate(int8_t angle)
		{
			angle = utils::constrain(angle, MIN, MAX);
			TYPE count =
				(angle >= 0 ? utils::map(int32_t(angle), 0L, int32_t(MAX), COUNTER_NEUTRAL_, COUNTER_MAXIMUM_) :
								utils::map(int32_t(angle), int32_t(MIN), 0L, COUNTER_MINIMUM_, COUNTER_NEUTRAL_));
			out_.set_duty(count);
		}

		/**
		 * Calculate the counter value to use with `set_counter()` in order to
		 * generate a pulse of the given width.
		 * This method is `constexpr` hence it can be evaluated at compile-time
		 * if provided with a constant value.
		 * 
		 * @param pulse_us the pulse width, in microseconds, for which to compute
		 * the counter value
		 * @return the counter value to use for a @p pulse_us width
		 * 
		 * @sa set_counter()
		 */
		constexpr TYPE calculate_counter(uint16_t pulse_us) const
		{
			return counter(utils::constrain(pulse_us, US_MINIMUM_, US_MAXIMUM_));
		}

	private:
		static constexpr TYPE counter(uint16_t pulse_us)
		{
			return CALC::PulseTimer_value(PRESCALER, pulse_us);
		}

		static const int8_t MAX = +90;
		static const int8_t MIN = -90;

		analog::PWMOutput<PWMPIN, true> out_;

		const uint16_t US_MINIMUM_;
		const uint16_t US_MAXIMUM_;
		const uint16_t US_NEUTRAL_;
		const TYPE COUNTER_MINIMUM_;
		const TYPE COUNTER_MAXIMUM_;
		const TYPE COUNTER_NEUTRAL_;
	};
}

#endif /* SERVO_H */
/// @endcond
