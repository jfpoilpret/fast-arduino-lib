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
 * SquareWave API, an API to generate square waves of any frequency.
 */
#ifndef SQUARE_WAVE_HH
#define SQUARE_WAVE_HH

#include "timer.h"
#include "pwm.h"

namespace timer
{
	/**
	 * Simple API to generate a square wave to an output pin.
	 * This uses a Timer and the Ouput pin OCxA of that Timer.
	 * 
	 * This class can be useful in many situations where you need to generate
	 * a square wave of a given frequency (or several frequencies) on an output
	 * pin. For example, it can be used to produce "music" with a buzzer.
	 * This is what `devices::audio::ToneGenerator` and `devices::audio::TonePlayer`
	 * do.
	 * 
	 * @tparam NTIMER_ the AVR timer to use for the underlying Timer
	 * @tparam OUTPUT_ the `board::PWMPin` connected to this SquareWave generator;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * 
	 * @sa devices::audio::ToneGenerator
	 * @sa devices::audio::TonePlayer
	 */
	template<board::Timer NTIMER_, board::PWMPin OUTPUT_> class SquareWave
	{
	public:
		/** The AVR timer used for the underlying Timer. */
		static constexpr const board::Timer NTIMER = NTIMER_;
		/** The board::PWMPin connected to this SquareWave generator. */
		static constexpr const board::PWMPin OUTPUT = OUTPUT_;
		/** The pin to which this SquareWave generator is connected. */
		static constexpr const board::DigitalPin PIN = board_traits::PWMPin_trait<OUTPUT>::ACTUAL_PIN;

		/** The timer::Calculator type for the underlying timer. */
		using CALC = timer::Calculator<NTIMER>;
		/** The type of underlying timer::Timer used by this SquareWave generator. */
		using TIMER = timer::Timer<NTIMER>;
		/** The analog::PWMOutput type for the @p OUTPUT board::PWMPin. */
		using PWMPIN = analog::PWMOutput<OUTPUT>;

		/**
		 * Instantiate a SquareWave generator.
		 * This will create the underlying Timer with the proper arguments.
		 */
		SquareWave()
			: timer_{timer::TimerMode::CTC, TIMER::PRESCALER::NO_PRESCALING},
			  output_{timer_, timer::TimerOutputMode::TOGGLE}
		{
			using TRAIT = board_traits::PWMPin_trait<OUTPUT>;
			static_assert(TRAIT::COM == 0, "Only OCnA pin is supported for wave generation");
		}

		/**
		 * Return the underlying timer::Timer of this SquareWave generator.
		 */
		const TIMER& timer() const
		{
			return timer_;
		}

		/**
		 * Return the underlying timer::Timer of this SquareWave generator.
		 */
		TIMER& timer()
		{
			return timer_;
		}

		/**
		 * Start producing, on the ouput pin, a square wave with the specified
		 * frequency.
		 * 
		 * This method performs heavy calculation to set the proper attributes for
		 * the underlying Timer.
		 * If possible, you should prefer the other start_frequency() that directly
		 * takes a prescaler and a counter value (those can be pre-calculated at
		 * compile-time).
		 * @param frequency the frequency, in Hz, of the square wave t generate
		 * 
		 * @sa start_frequency(typename TIMER::PRESCALER, typename TIMER::TYPE)
		 */
		void start_frequency(uint32_t frequency)
		{
			const uint32_t period = ONE_SECOND / 2 / frequency;
			typename TIMER::PRESCALER prescaler = CALC::CTC_prescaler(period);
			start_frequency(prescaler, CALC::CTC_counter(prescaler, period));
		}

		/**
		 * Start producing, on the output pin, a square wave, which frequency
		 * matches the specified @p prescaler and @p counter arguments.
		 * This method performs no heavy calculation and should be preferred
		 * over `start_frequency(uint32_t)` if the used frequency is known at
		 * compile-time.
		 * @param prescaler the prescaler to use on the underlying Timer, in 
		 * order to produce the desired frequency. It can be calculated by
		 * `CALC::CTC_prescaler()`
		 * @param counter the counter value to use on the underlying Timer, in
		 * order to produce the desired frequency. It can be calculated by
		 * `CALC::CTC_counter()`
		 * 
		 * @sa start_frequency(uint32_t)
		 * @sa timer::Calculator<NTIMER_>::CTC_prescaler()
		 * @sa timer::Calculator<NTIMER_>::CTC_counter()
		 */
		void start_frequency(typename TIMER::PRESCALER prescaler, typename TIMER::TYPE counter)
		{
			timer_.end();
			timer_.set_prescaler(prescaler);
			timer_.begin();
			output_.set_duty(counter);
		}

		/**
		 * Stop square wave generation.
		 */
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
/// @endcond
