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

#ifndef PULSE_TIMER_HH
#define PULSE_TIMER_HH

#include <avr/interrupt.h>
#include <stddef.h>

#include "interrupts.h"
#include "utilities.h"
#include "boards/board_traits.h"
#include "timer.h"
#include "gpio.h"

#define REGISTER_PULSE_TIMER_OVF2_ISR_(TIMER_NUM, PRESCALER, PIN_A, COM_A, PIN_B, COM_B)	\
ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))														\
{																							\
	const board::Timer T = CAT(board::Timer::TIMER, TIMER_NUM);								\
	using TT = board_traits::Timer_trait<T>;												\
	static_assert(!TT::IS_16BITS ,"TIMER_NUM must be an 8 bits Timer");						\
	using PINTA = board_traits::Timer_COM_trait<T, COM_A>;									\
	static_assert(PIN_A == PINTA::PIN_OCR, "PIN_A must be connected to TIMER_NUM OCxA");	\
	using PINTB = board_traits::Timer_COM_trait<T, COM_B>;									\
	static_assert(PIN_B == PINTB::PIN_OCR, "PIN_B must be connected to TIMER_NUM OCxB");	\
	using PT = timer::PulseTimer8<T , PRESCALER>;											\
	if (CALL_HANDLER_RETURN_(PT, &PT::overflow, bool)())									\
	{																						\
		if (PINTA::OCR != 0)																\
			gpio::FastPinType< PIN_A >::set();												\
		if (PINTB::OCR != 0)																\
			gpio::FastPinType< PIN_B >::set();												\
	}																						\
}

#define REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN, COM_NUM)				\
ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))													\
{																						\
	const board::Timer T = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TT = board_traits::Timer_trait<T>;											\
	static_assert(!TT::IS_16BITS ,"TIMER_NUM must be an 8 bits Timer");					\
	using PINT = board_traits::Timer_COM_trait<T, COM_NUM>;								\
	static_assert(PIN == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");\
	using PT = timer::PulseTimer8<T , PRESCALER>;										\
	if (CALL_HANDLER_RETURN_(PT, &PT::overflow, bool)())								\
		gpio::FastPinType< PIN >::set();												\
}

#define REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, COM_NUM, COMP, PIN)					\
ISR(CAT3(TIMER, TIMER_NUM, COMP))														\
{																						\
	const board::Timer T = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using PINT = board_traits::Timer_COM_trait<T, COM_NUM>;								\
	static_assert(PIN == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");\
	gpio::FastPinType< PIN >::clear();													\
}

// Macros to register ISR for PWM on PulseTimer8
//==============================================
#define REGISTER_PULSE_TIMER8_AB_ISR(TIMER_NUM, PRESCALER, PIN_A, PIN_B)	\
REGISTER_PULSE_TIMER_OVF2_ISR_(TIMER_NUM, PRESCALER, PIN_A, 0, PIN_B, 1)	\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A)			\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B)

#define REGISTER_PULSE_TIMER8_A_ISR(TIMER_NUM, PRESCALER, PIN_A)			\
REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_A, 0)				\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A)			\
EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPB_vect))

#define REGISTER_PULSE_TIMER8_B_ISR(TIMER_NUM, PRESCALER, PIN_B)			\
REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_B, 1)				\
REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B)			\
EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect))

namespace timer
{
	// Timer specialized in emitting pulses with accurate width, according to a slow frequency; this is typically
	// useful for controlling servos, which need a pulse with a width range from ~1000us to ~2000us, send every 
	// 20ms, ie with a 50Hz frequency.
	// This implementation ensures a good pulse width precision for 16-bits timer.
	template<board::Timer TIMER, typename Calculator<TIMER>::TIMER_PRESCALER PRESCALER_>
	class PulseTimer16: public Timer<TIMER>
	{
		using PARENT = Timer<TIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(TRAIT::IS_16BITS, "TIMER must be a 16 bits timer");
		
	public:
		using CALCULATOR = Calculator<TIMER>;
		using TIMER_PRESCALER = typename CALCULATOR::TIMER_PRESCALER;
		static constexpr const TIMER_PRESCALER PRESCALER = PRESCALER_;
		
		PulseTimer16(uint16_t pulse_frequency)
			:	Timer<TIMER>{TCCRA(), TCCRB()}
		{
			TRAIT::ICR = CALCULATOR::PWM_ICR_counter(PRESCALER, pulse_frequency);
		}
				
	private:
		static constexpr uint8_t TCCRA()
		{
			// If 16 bits, use ICR1 FastPWM
			return TRAIT::F_PWM_ICR_TCCRA;
		}
		static constexpr uint8_t TCCRB()
		{
			// If 16 bits, use ICR1 FastPWM and prescaler forced to best fit all pulse frequency
			return TRAIT::F_PWM_ICR_TCCRB | TRAIT::TCCRB_prescaler(PRESCALER);
		}
	};
	
	// Timer specialized in emitting pulses with accurate width, according to a slow frequency; this is typically
	// useful for controlling servos, which need a pulse with a width range from ~1000us to ~2000us, send every 
	// 20ms, ie with a 50Hz frequency.
	// This implementation ensures a good pulse width precision for 8-bits timers.
	template<board::Timer TIMER, typename Calculator<TIMER>::TIMER_PRESCALER PRESCALER_>
	class PulseTimer8: public Timer<TIMER>
	{
		using PARENT = Timer<TIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(!TRAIT::IS_16BITS, "TIMER must be an 8 bits timer");
		
	public:
		using CALCULATOR = Calculator<TIMER>;
		using TIMER_PRESCALER = typename CALCULATOR::TIMER_PRESCALER;
		static constexpr const TIMER_PRESCALER PRESCALER = PRESCALER_;

		PulseTimer8(uint16_t pulse_frequency)
			:	Timer<TIMER>{TCCRA(), TCCRB(), TIMSK()}, 
				MAX{OVERFLOW_COUNTER(pulse_frequency)}
		{
			// If 8 bits timer, then we need ISR on Overflow and Compare A/B
			interrupt::register_handler(*this);
		}
				
		bool overflow()
		{
			if (++count_ == MAX) count_ = 0;
			return !count_;
		}

	private:
		static constexpr uint8_t TCCRA()
		{
			// If 8 bits, use CTC/TOV ISR
			return 0;
		}
		static constexpr uint8_t TCCRB()
		{
			// If 8 bits, use CTC/TOV ISR with prescaler forced best fit max pulse width
			return TRAIT::TCCRB_prescaler(PRESCALER);
		}
		static constexpr uint8_t TIMSK()
		{
			return TRAIT::TIMSK_MASK(uint8_t(
				TimerInterrupt::OVERFLOW | TimerInterrupt::OUTPUT_COMPARE_A | TimerInterrupt::OUTPUT_COMPARE_B));
		}
		static constexpr uint8_t OVERFLOW_COUNTER(uint16_t pulse_frequency)
		{
			return F_CPU / 256UL / _BV(uint8_t(PRESCALER)) / pulse_frequency;
		}
		
	private:
		const uint8_t MAX;
		uint8_t count_;
	};

	// Unified API for PulseTimer whatever the timer bits size (no need to use PulseTimer8 or PulseTimer16)
	template<	board::Timer TIMER, typename timer::Calculator<TIMER>::TIMER_PRESCALER PRESCALER, 
				typename T = typename board_traits::Timer_trait<TIMER>::TYPE>
	class PulseTimer: public timer::Timer<TIMER>
	{
	public:
		PulseTimer(UNUSED uint16_t pulse_frequency):timer::Timer<TIMER>{0, 0} {}
		inline void begin() {}
		inline void _begin() {}
	};

	template<board::Timer TIMER, typename timer::Calculator<TIMER>::TIMER_PRESCALER PRESCALER>
	class PulseTimer<TIMER, PRESCALER, uint8_t>: public timer::PulseTimer8<TIMER, PRESCALER>
	{
	public:
		PulseTimer(uint16_t pulse_frequency):timer::PulseTimer8<TIMER, PRESCALER>{pulse_frequency} {}
	};

	template<board::Timer TIMER, typename timer::Calculator<TIMER>::TIMER_PRESCALER PRESCALER>
	class PulseTimer<TIMER, PRESCALER, uint16_t>: public timer::PulseTimer16<TIMER, PRESCALER>
	{
	public:
		PulseTimer(uint16_t pulse_frequency):timer::PulseTimer16<TIMER, PRESCALER>{pulse_frequency} {}
	};
}

#endif /* PULSE_TIMER_HH */
