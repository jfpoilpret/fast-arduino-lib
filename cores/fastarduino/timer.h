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

#include "interrupts.h"
#include "utilities.h"
#include "boards/board_traits.h"

// Generic macros to register ISR on Timer
//=========================================
//TODO rework naming of these API macros
#define REGISTER_TIMER_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), HANDLER, CALLBACK)

#define REGISTER_TIMER_ISR_FUNCTION(TIMER_NUM, CALLBACK)	\
REGISTER_ISR_FUNCTION_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), CALLBACK)

#define REGISTER_TIMER_ISR_EMPTY(TIMER_NUM)	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect));

// Macros to register ISR for PWM on PulseTimer8
//==============================================
#define REGISTER_PULSE_TIMER8_AB_ISR(TIMER_NUM, PRESCALER, PIN_A, PIN_B)	\
REGISTER_PULSE_TIMER_OVF2_ISR_(TIMER_NUM, PRESCALER, PIN_A, PIN_B)			\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A)			\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B)

#define REGISTER_PULSE_TIMER8_A_ISR(TIMER_NUM, PRESCALER, PIN_A)			\
REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_A)					\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A)			\
EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPB_vect))

#define REGISTER_PULSE_TIMER8_B_ISR(TIMER_NUM, PRESCALER, PIN_B)			\
REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_B)					\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B)			\
EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect))

//TODO Add API to explicitly set interrupts we want to enable
//TODO Add API to support Input Capture when available for Timer (Timer1)
namespace timer
{
	// All utility methods go here
	template<board::Timer TIMER>
	struct Calculator
	{
	private:
		using TRAIT = board_traits::Timer_trait<TIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;

	public:
		using TIMER_TYPE = typename TRAIT::TYPE;
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		static constexpr const TIMER_TYPE PWM_MAX = TRAIT::MAX_PWM;
		
		// Calculations for Compare mode
		static constexpr TIMER_PRESCALER CTC_prescaler(uint32_t us)
		{
			return best_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us);
		}
		static constexpr uint32_t CTC_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler));
		}
		static constexpr TIMER_TYPE CTC_counter(TIMER_PRESCALER prescaler, uint32_t us)
		{
			return (TIMER_TYPE) prescaler_quotient(prescaler, us) - 1;
		}
		static constexpr bool is_adequate_for_CTC(TIMER_PRESCALER p, uint32_t us)
		{
			return prescaler_is_adequate(prescaler_quotient(p, us));
		}

		// Calculations for Fast PWM mode 8 bits or 10 bits)
		static constexpr TIMER_PRESCALER FastPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (PWM_MAX + 1UL));
		}
		static constexpr uint16_t FastPWM_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / (PWM_MAX + 1UL);
		}
		
		// Calculations for Phase Correct PWM mode 8 bits or 10 bits)
		static constexpr TIMER_PRESCALER PhaseCorrectPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (2UL * PWM_MAX));
		}
		static constexpr uint16_t PhaseCorrectPWM_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / (2UL * PWM_MAX);
		}
	
		// Calculations for 8bits/16 bits PulseTimer class
		static constexpr TIMER_PRESCALER PulseTimer_prescaler(uint16_t max_pulse_width_us, uint16_t pulse_frequency)
		{
			return TRAIT::IS_16BITS ? PWM_ICR_prescaler(pulse_frequency) : CTC_prescaler(max_pulse_width_us);
		}
		static constexpr TIMER_TYPE PulseTimer_value(TIMER_PRESCALER prescaler, uint16_t period_us)
		{
			return CTC_counter(prescaler, period_us);
		}

		// Calculations for 16 bits ICR-based Fast PWM mode
		static constexpr TIMER_PRESCALER PWM_ICR_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * 16384UL);
		}
		static constexpr uint16_t PWM_ICR_frequency(TIMER_PRESCALER prescaler, uint16_t counter)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / counter;
		}
		static constexpr uint16_t PWM_ICR_counter(TIMER_PRESCALER prescaler, uint16_t pwm_frequency)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / pwm_frequency;
		}
		
	private:
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
		
	};

	enum class TimerOutputMode:uint8_t
	{
		DISCONNECTED,
		TOGGLE,
		NON_INVERTING,
		INVERTING
	};
	
	enum class TimerMode:uint8_t
	{
		NORMAL,
		CTC,
		FAST_PWM,
		PHASE_CORRECT_PWM
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
		static constexpr const TIMER_TYPE PWM_MAX = TRAIT::MAX_PWM;
		
		// Constructor to create a general-purpose timer
		Timer(TimerMode timer_mode)
			:_tccra{timer_mode_TCCRA(timer_mode)}, _tccrb{timer_mode_TCCRB(timer_mode)} {}

		inline void set_timer_mode(TimerMode timer_mode)
		{
			utils::set_mask(_tccra, 0xFF & ~TRAIT::COM_MASK, timer_mode_TCCRA(timer_mode));
			_tccrb = timer_mode_TCCRB(timer_mode);
		}
		
		inline void begin(TIMER_PRESCALER prescaler)
		{
			synchronized _begin(prescaler);
		}
		inline void _begin(TIMER_PRESCALER prescaler)
		{
			TRAIT::TCCRA = _tccra;
			TRAIT::TCCRB = _tccrb | TRAIT::TCCRB_prescaler(prescaler);
			TRAIT::OCRA = 0;
			TRAIT::TCNT = 0;
			TRAIT::TIMSK = 0;
		}
		
		inline void begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
		{
			synchronized _begin(prescaler, max);
		}
		inline void _begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
		{
			TRAIT::TCCRA = _tccra;
			TRAIT::TCCRB = _tccrb | TRAIT::TCCRB_prescaler(prescaler);
			// Set timer counter compare match
			TRAIT::OCRA = max;
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = _BV(OCIE0A);
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

		template<uint8_t COM>
		inline void set_output_mode(TimerOutputMode mode)
		{
			static_assert(COM < TRAIT::COM_COUNT, "COM must exist for TIMER");
			using COM_TRAIT = board_traits::Timer_COM_trait<TIMER, COM>;
			utils::set_mask(_tccra, COM_TRAIT::COM_MASK, convert_COM<COM>(mode));
		}
		
		template<uint8_t COM>
		inline void set_max(TIMER_TYPE max)
		{
			static_assert(COM < TRAIT::COM_COUNT, "COM must exist for TIMER");
			using COM_TRAIT = board_traits::Timer_COM_trait<TIMER, COM>;
			synchronized
			{
				if (max)
					utils::set_mask((volatile uint8_t&) TRAIT::TCCRA, COM_TRAIT::COM_MASK, _tccra);
				else
					utils::set_mask((volatile uint8_t&) TRAIT::TCCRA, COM_TRAIT::COM_MASK, 
						convert_COM<COM>(TimerOutputMode::DISCONNECTED));
				COM_TRAIT::OCR = max;
			}
		}

	protected:
		Timer(uint8_t tccra, uint8_t tccrb):_tccra{tccra}, _tccrb{tccrb} {}

		template<uint8_t COM>
		static constexpr uint8_t convert_COM(TimerOutputMode output_mode)
		{
			using COM_TRAIT = board_traits::Timer_COM_trait<TIMER, COM>;
			return (output_mode == TimerOutputMode::TOGGLE ? COM_TRAIT::COM_TOGGLE :
					output_mode == TimerOutputMode::INVERTING ? COM_TRAIT::COM_SET :
					output_mode == TimerOutputMode::NON_INVERTING ? COM_TRAIT::COM_CLEAR :
					COM_TRAIT::COM_NORMAL);
		}

		static constexpr uint8_t timer_mode_TCCRA(TimerMode timer_mode)
		{
			return (timer_mode == TimerMode::CTC ? TRAIT::CTC_TCCRA :
					timer_mode == TimerMode::FAST_PWM ? TRAIT::F_PWM_TCCRA :
					timer_mode == TimerMode::PHASE_CORRECT_PWM ? TRAIT::PC_PWM_TCCRA :
					0);
		}
		static constexpr uint8_t timer_mode_TCCRB(TimerMode timer_mode)
		{
			return (timer_mode == TimerMode::CTC ? TRAIT::CTC_TCCRB :
					timer_mode == TimerMode::FAST_PWM ? TRAIT::F_PWM_TCCRB :
					timer_mode == TimerMode::PHASE_CORRECT_PWM ? TRAIT::PC_PWM_TCCRB :
					0);
		}
		
		uint8_t _tccra;
		uint8_t _tccrb;
	};
}

#endif /* TIMER_HH */
