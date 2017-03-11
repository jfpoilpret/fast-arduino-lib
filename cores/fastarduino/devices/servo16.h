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

#ifndef SERVO16_H
#define SERVO16_H

#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

namespace servo
{
	//TODO Improve by using ICR1 to 20000us and prescaler to 1us
	template<board::Timer TIMER, board::DigitalPin PIN> 
	class Servo16
	{
		using TIMER_TRAIT = board_traits::Timer_trait<TIMER>;

	public:
		Servo16(timer::Timer<TIMER>& timer, uint16_t neutral, uint16_t minimum, uint16_t maximum)
			:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
				neutral_{int16_t(neutral)}, minimum_{int16_t(minimum)}, maximum_{int16_t(maximum)}
		{
			static_assert(TIMER_TRAIT::MAX_PWM >= 0x3FF, "TIMER must be a 16 bits timer");
			out_.set_duty(neutral_);
		}

		inline void set(uint16_t value)
		{
			out_.set_duty(value);
		}

		inline void rotate(int8_t angle)
		{
			uint16_t duty;
			if (angle > 0)
				duty = angle * (maximum_ - neutral_) / (MAX - 0) + neutral_;
			else if (angle < 0)
				duty = angle * (neutral_ - minimum_) / (0 - MIN) + neutral_;
			else
				duty = neutral_;
			out_.set_duty(duty);
		}

	private:
		static const int8_t MAX = 127;
		static const int8_t MIN = -128;

		timer::Timer<TIMER>& timer_;
		analog::PWMOutput<PIN> out_;
		const int16_t neutral_;
		const int16_t minimum_;
		const int16_t maximum_;
	};
}

#endif /* SERVO16_H */

