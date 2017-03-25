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

#ifndef SERVO_H
#define SERVO_H

#include <fastarduino/pulse_timer.h>
#include <fastarduino/pwm.h>
#include <fastarduino/utilities.h>

namespace devices
{
namespace servo
{
	// This must be used with TIMER = timer::PulseTimer<...>
	template<typename TIMER, board::DigitalPin PIN> 
	class Servo
	{
		using CALC = typename TIMER::CALCULATOR;
		using TPRESCALER = typename CALC::TIMER_PRESCALER;
		static constexpr const TPRESCALER PRESCALER = TIMER::PRESCALER;

	public:
		using TYPE = typename TIMER::TIMER_TYPE;

		Servo(	TIMER& timer, uint16_t us_minimum, uint16_t us_maximum, 
				uint16_t us_neutral = 0)
			:	out_{timer}, 
				US_MINIMUM_{us_minimum}, 
				US_MAXIMUM_{us_maximum}, 
				US_NEUTRAL_{us_neutral ? us_neutral : ((us_maximum + us_minimum) / 2)},
				COUNTER_MINIMUM_{counter(US_MINIMUM_)},
				COUNTER_MAXIMUM_{counter(US_MAXIMUM_)},
				COUNTER_NEUTRAL_{counter(US_NEUTRAL_)} {}

		inline void detach() INLINE
		{
			out_.set_duty(0);
		}

		inline void set_counter(TYPE value) INLINE
		{
			out_.set_duty(utils::constrain(value, COUNTER_MINIMUM_, COUNTER_MAXIMUM_));
		}

		inline void set_pulse(uint16_t pulse_us)
		{
			// Constrain pulse to min/max and convert pulse to timer counter value
			out_.set_duty(counter(utils::constrain(pulse_us, US_MINIMUM_, US_MAXIMUM_)));
		}

		inline void rotate(int8_t angle)
		{
			angle = utils::constrain(angle, MIN, MAX);
			TYPE count = (	angle >= 0 ? 
							utils::map(int32_t(angle), 0L, int32_t(MAX), COUNTER_NEUTRAL_, COUNTER_MAXIMUM_) :
							utils::map(int32_t(angle), int32_t(MIN), 0L, COUNTER_MINIMUM_, COUNTER_NEUTRAL_));
			out_.set_duty(count);
		}

	private:
		static constexpr TYPE counter(uint16_t pulse_us)
		{
			return CALC::PulseTimer_value(PRESCALER, pulse_us);
		}

		static const int8_t MAX = +90;
		static const int8_t MIN = -90;

		analog::PWMOutput<PIN> out_;

		const uint16_t US_MINIMUM_;
		const uint16_t US_MAXIMUM_;
		const uint16_t US_NEUTRAL_;
		const TYPE COUNTER_MINIMUM_;
		const TYPE COUNTER_MAXIMUM_;
		const TYPE COUNTER_NEUTRAL_;
	};
}
}

#endif /* SERVO_H */
