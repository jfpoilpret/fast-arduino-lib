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

#ifndef TIMER_HH
#define TIMER_HH

#include <avr/interrupt.h>
#include <stddef.h>

#include "utilities.h"
#include "boards/board_traits.h"

#define REGISTER_TIMER_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), HANDLER, CALLBACK)

#define REGISTER_TIMER_ISR_FUNCTION(TIMER_NUM, CALLBACK)	\
REGISTER_ISR_FUNCTION_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), CALLBACK)

#define REGISTER_TIMER_ISR_EMPTY(TIMER_NUM)	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect));

//TODO add data members to save various statuses: output modes A & B, timer mode?
//TODO improve PWM API by adding a specific PWMOutput class, extracted from Timer, used to perform duty cycle changes
namespace timer
{
	enum class TimerOutputMode:uint8_t
	{
		DISCONNECTED,
		TOGGLE,
		NON_INVERTING,
		INVERTING
	};
	
	template<board::Timer TIMER>
	class Timer
	{
	protected:
		using TRAIT = board_traits::Timer_trait<TIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;

	public:
		using TIMER_TYPE = typename TRAIT::TYPE;
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		static constexpr const TIMER_TYPE TIMER_MAX = TRAIT::MAX_COUNTER - 1;
		
		Timer():_mode_A{mode_A(TimerOutputMode::DISCONNECTED)}, _mode_B{mode_B(TimerOutputMode::DISCONNECTED)} {}

		static constexpr bool is_adequate(TIMER_PRESCALER p, uint32_t us)
		{
			return prescaler_is_adequate(prescaler_quotient(p, us));
		}
		static constexpr TIMER_PRESCALER timer_prescaler(uint32_t us)
		{
			return best_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us);
		}
		static constexpr TIMER_TYPE counter(TIMER_PRESCALER prescaler, uint32_t us)
		{
			return (TIMER_TYPE) prescaler_quotient(prescaler, us) - 1;
		}
		static constexpr TIMER_TYPE counter(uint32_t us)
		{
			return (TIMER_TYPE) prescaler_quotient(timer_prescaler(us), us) - 1;
		}

		//FIXME prescaler is different for FastPWM and Phase Correct PWM
		static constexpr TIMER_PRESCALER PWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * TRAIT::MAX_COUNTER);
		}
		//FIXME frequency is different for FastPWM and Phase Correct PWM
		static constexpr uint16_t PWM_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / TRAIT::MAX_COUNTER;
		}
		
		inline void begin_CTC(TIMER_PRESCALER prescaler, TIMER_TYPE max)
		{
			synchronized _begin_CTC(prescaler, max);
		}
		inline void _begin_CTC(TIMER_PRESCALER prescaler, TIMER_TYPE max)
		{
			// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
			TRAIT::TCCRA = TRAIT::CTC_TCCRA;
			// Don't force output compare (FOCA & FOCB), Clock Select according to prescaler
			TRAIT::TCCRB = TRAIT::CTC_TCCRB | TRAIT::TCCRB_prescaler(prescaler);
			// Set timer counter compare match
			TRAIT::OCRA = max;
			// Reset timer counter
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = _BV(OCIE0A);
		}
		
		//TODO maybe put in non-template parent class?
		inline void set_output_modes(TimerOutputMode output_mode_A, TimerOutputMode output_mode_B)
		{
			_mode_A = mode_A(output_mode_A);
			_mode_B = mode_B(output_mode_B);
		}
		
		inline void begin_FastPWM(TIMER_PRESCALER prescaler)
		{
			synchronized _begin_FastPWM(prescaler);
		}
		inline void _begin_FastPWM(TIMER_PRESCALER prescaler)
		{
			TRAIT::TCCRA = TRAIT::F_PWM_TCCRA | _mode_A | _mode_B;
			TRAIT::TCCRB = TRAIT::F_PWM_TCCRB | TRAIT::TCCRB_prescaler(prescaler);
			// Reset timer counter
			TRAIT::TCNT = 0;
			TRAIT::OCRA = 0;
			TRAIT::OCRB = 0;
			// Clear timer interrupt mode
			TRAIT::TIMSK = 0;
		}
		
		inline void begin_PhaseCorrectPWM(TIMER_PRESCALER prescaler)
		{
			synchronized _begin_PhaseCorrectPWM(prescaler);
		}
		inline void _begin_PhaseCorrectPWM(TIMER_PRESCALER prescaler)
		{
			TRAIT::TCCRA = TRAIT::PC_PWM_TCCRA | _mode_A | _mode_B;
			TRAIT::TCCRB = TRAIT::PC_PWM_TCCRB | TRAIT::TCCRB_prescaler(prescaler);
			// Reset timer counter
			TRAIT::TCNT = 0;
			TRAIT::OCRA = 0;
			TRAIT::OCRB = 0;
			// Clear timer interrupt mode
			TRAIT::TIMSK = 0;
		}
		
		inline void suspend()
		{
			synchronized _suspend();
		}
		inline void _suspend()
		{
			// Clear timer interrupt mode
			TRAIT::TIMSK = 0;
		}
		inline void resume()
		{
			synchronized _resume();
		}
		inline void _resume()
		{
			// Reset timer counter
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = _BV(OCIE0A);
		}
		inline bool is_suspended()
		{
			return TRAIT::TIMSK == 0;
		}
		
		inline void end()
		{
			synchronized _end();
		}
		inline void _end()
		{
			// Stop timer
			TRAIT::TCCRB = 0;
			// Clear timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = 0;
		}
		
		inline void set_max_A(TIMER_TYPE max)
		{
			synchronized
			{
				if (max)
					TRAIT::TCCRA = (TRAIT::TCCRA & ~TRAIT::COM_MASK_A) | (_mode_A & TRAIT::COM_MASK_A);
				else
					TRAIT::TCCRA = (TRAIT::TCCRA & ~TRAIT::COM_MASK_A) | (mode_A(TimerOutputMode::DISCONNECTED) & TRAIT::COM_MASK_A);
				TRAIT::OCRA = max;
			}
		}
		inline void set_max_B(TIMER_TYPE max)
		{
			synchronized
			{
				if (max)
					TRAIT::TCCRA = (TRAIT::TCCRA & ~TRAIT::COM_MASK_B) | (_mode_B & TRAIT::COM_MASK_B);
				else
					TRAIT::TCCRA = (TRAIT::TCCRA & ~TRAIT::COM_MASK_B) | (mode_B(TimerOutputMode::DISCONNECTED) & TRAIT::COM_MASK_B);
				TRAIT::OCRB = max;
			}
		}

	private:
		static constexpr uint8_t mode_A(TimerOutputMode output_mode)
		{
			return (output_mode == TimerOutputMode::TOGGLE ? TRAIT::COM_TOGGLE_A :
					output_mode == TimerOutputMode::INVERTING ? TRAIT::COM_SET_A :
					output_mode == TimerOutputMode::NON_INVERTING ? TRAIT::COM_CLEAR_A :
					TRAIT::COM_NORMAL_A);
		}
		static constexpr uint8_t mode_B(TimerOutputMode output_mode)
		{
			return (output_mode == TimerOutputMode::TOGGLE ? TRAIT::COM_TOGGLE_B :
					output_mode == TimerOutputMode::INVERTING ? TRAIT::COM_SET_B :
					output_mode == TimerOutputMode::NON_INVERTING ? TRAIT::COM_CLEAR_B :
					TRAIT::COM_NORMAL_B);
		}
		
		static constexpr uint32_t prescaler_quotient(TIMER_PRESCALER p, uint32_t us)
		{
			return (F_CPU / 1000000UL * us) / _BV(uint8_t(p));
		}

		static constexpr uint32_t prescaler_remainder(TIMER_PRESCALER p, uint32_t us)
		{
			return (F_CPU / 1000000UL * us) % _BV(uint8_t(p));
		}

		static constexpr bool prescaler_is_adequate(uint32_t quotient)
		{
			return quotient > 1 and quotient < TRAIT::MAX_COUNTER;
		}

		static constexpr TIMER_PRESCALER best_prescaler_in_2(TIMER_PRESCALER p1, TIMER_PRESCALER p2, uint32_t us)
		{
			return (!prescaler_is_adequate(prescaler_quotient(p1, us)) ? p2 :
					!prescaler_is_adequate(prescaler_quotient(p2, us)) ? p1 :
					prescaler_remainder(p1, us) < prescaler_remainder(p2, us) ? p1 :
					prescaler_remainder(p1, us) > prescaler_remainder(p2, us) ? p2 :
					prescaler_quotient(p1, us) > prescaler_quotient(p2, us) ? p1 : p2);
		}

		static constexpr TIMER_PRESCALER best_prescaler(const TIMER_PRESCALER* begin, const TIMER_PRESCALER* end, uint32_t us)
		{
			return (begin + 1 == end ? *begin : best_prescaler_in_2(*begin, best_prescaler(begin + 1 , end, us), us));
		}

		template<size_t N>
		static constexpr TIMER_PRESCALER best_prescaler(const TIMER_PRESCALER(&prescalers)[N], uint32_t us)
		{
			return best_prescaler(prescalers, prescalers + N, us);
		}
		
		static constexpr bool prescaler_is_adequate_for_frequency(TIMER_PRESCALER p, uint32_t freq)
		{
			return (F_CPU / (uint32_t) _BV(uint8_t(p)) > freq);
		}
		
		static constexpr TIMER_PRESCALER best_frequency_prescaler_in_2(TIMER_PRESCALER p1, TIMER_PRESCALER p2, uint32_t freq)
		{
			return prescaler_is_adequate_for_frequency(p2, freq) ? p2 : p1;
		}

		static constexpr TIMER_PRESCALER best_frequency_prescaler(const TIMER_PRESCALER* begin, const TIMER_PRESCALER* end, uint32_t freq)
		{
			return (begin + 1 == end ? *begin : 
					best_frequency_prescaler_in_2(*begin, best_frequency_prescaler(begin + 1 , end, freq), freq));
		}

		template<size_t N>
		static constexpr TIMER_PRESCALER best_frequency_prescaler(const TIMER_PRESCALER(&prescalers)[N], uint32_t freq)
		{
			return best_frequency_prescaler(prescalers, prescalers + N, freq);
		}
		
		//TODO check if it can be optimized with only one byte with both modes?
		uint8_t _mode_A;
		uint8_t _mode_B;
	};
}

#endif /* TIMER_HH */
