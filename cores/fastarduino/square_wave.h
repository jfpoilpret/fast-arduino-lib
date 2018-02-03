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

#ifndef SQUARE_WAVE_HH
#define SQUARE_WAVE_HH

#include "timer.h"
#include "pwm.h"

namespace timer
{
	template<board::Timer NTIMER_, board::DigitalPin OUTPUT_> class SquareWave
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;
		static constexpr const board::DigitalPin OUTPUT = OUTPUT_;

		using CALC = timer::Calculator<NTIMER>;
		using TIMER = timer::Timer<NTIMER>;
		using PWMPIN = analog::PWMOutput<OUTPUT>;

		SquareWave()
			: timer_{timer::TimerMode::CTC, TIMER::PRESCALER::NO_PRESCALING},
			  output_{timer_, timer::TimerOutputMode::TOGGLE}
		{
			using TRAIT = board_traits::PWMPin_trait<OUTPUT>;
			static_assert(TRAIT::COM == 0, "Only OCnA pin is supported for wave generation");
		}

		TIMER& timer() const
		{
			return timer_;
		}

		void start_frequency(uint32_t frequency)
		{
			const uint32_t period = 1000000UL / 2 / frequency;
			typename TIMER::PRESCALER prescaler = CALC::CTC_prescaler(period);
			start_frequency(prescaler, CALC::CTC_counter(prescaler, period));
		}

		void start_frequency(typename TIMER::PRESCALER prescaler, typename TIMER::TYPE counter)
		{
			timer_.end();
			timer_.set_prescaler(prescaler);
			timer_.begin();
			output_.set_duty(counter);
		}

		void stop()
		{
			timer_.end();
			output_.set_duty(0);
		}

	private:
		TIMER timer_;
		PWMPIN output_;
	};
}

#endif /* SQUARE_WAVE_HH */
