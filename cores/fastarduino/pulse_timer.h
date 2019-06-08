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
 * PulseTimer API.
 */
#ifndef PULSE_TIMER_HH
#define PULSE_TIMER_HH

#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include <stddef.h>
#include "interrupts.h"
#include "utilities.h"
#include "timer.h"
#include "gpio.h"
#include "types_traits.h"

// Macros to register ISR for PWM on PulseTimer8
//==============================================
/**
 * Register all necessary ISR (Tnterrupt Service Routines) for a timer::PulseTimer
 * to work properly, when both its PWM pins are connected.
 * 
 * Note: this is necessary only for PulseTimer based on an 8-bits Timer. If you are
 * using a 16-bits based PulseTimer, then you don't need to use this macro.
 * 
 * @param TIMER_NUM the timer number (as defined in MCU datasheet)
 * @param PRESCALER the prescaler value used to instantiate the PulseTimer template
 * @param PIN_A the board::PWMPin connected to first PWM pin of the PulseTimer;
 * this is used for control only, to avoid bugs due to code typos.
 * @param PIN_B the board::PWMPin connected to second PWM pin of the PulseTimer;
 * this is used for control only, to avoid bugs due to code typos.
 * 
 * @sa timer::PulseTimer
 */
#define REGISTER_PULSE_TIMER8_AB_ISR(TIMER_NUM, PRESCALER, PIN_A, PIN_B)                            \
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))                                                          \
	{                                                                                               \
		timer::isr_handler_pulse::pulse_timer_overflow<TIMER_NUM, PRESCALER, PIN_A, 0, PIN_B, 1>(); \
	}                                                                                               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                        \
	{                                                                                               \
		timer::isr_handler_pulse::pulse_timer_compare<TIMER_NUM, 0, PIN_A>();                       \
	}                                                                                               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPB_vect))                                                        \
	{                                                                                               \
		timer::isr_handler_pulse::pulse_timer_compare<TIMER_NUM, 1, PIN_B>();                       \
	}

/**
 * Register all necessary ISR (Tnterrupt Service Routines) for a timer::PulseTimer
 * to work properly, when only its first PWM pins is connected.
 * 
 * Note: this is necessary only for PulseTimer based on an 8-bits Timer. If you are
 * using a 16-bits based PulseTimer, then you don't need to use this macro.
 * 
 * @param TIMER_NUM the timer number (as defined in MCU datasheet)
 * @param PRESCALER the prescaler value used to instantiate the PulseTimer template
 * @param PIN_A the board::PWMPin connected to first PWM pin of the PulseTimer;
 * this is used for control only, to avoid bugs due to code typos.
 * 
 * @sa timer::PulseTimer
 */
#define REGISTER_PULSE_TIMER8_A_ISR(TIMER_NUM, PRESCALER, PIN_A)                          \
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))                                                \
	{                                                                                     \
		timer::isr_handler_pulse::pulse_timer_overflow<TIMER_NUM, PRESCALER, PIN_A, 0>(); \
	}                                                                                     \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                              \
	{                                                                                     \
		timer::isr_handler_pulse::pulse_timer_compare<TIMER_NUM, 0, PIN_A>();             \
	}                                                                                     \
	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPB_vect))

/**
 * Register all necessary ISR (Tnterrupt Service Routines) for a timer::PulseTimer
 * to work properly, when only its second PWM pins is connected.
 * 
 * Note: this is necessary only for PulseTimer based on an 8-bits Timer. If you are
 * using a 16-bits based PulseTimer, then you don't need to use this macro.
 * 
 * @param TIMER_NUM the timer number (as defined in MCU datasheet)
 * @param PRESCALER the prescaler value used to instantiate the PulseTimer template
 * @param PIN_B the board::PWMPin connected to second PWM pin of the PulseTimer;
 * this is used for control only, to avoid bugs due to code typos.
 * 
 * @sa timer::PulseTimer
 */
#define REGISTER_PULSE_TIMER8_B_ISR(TIMER_NUM, PRESCALER, PIN_B)                          \
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))                                                \
	{                                                                                     \
		timer::isr_handler_pulse::pulse_timer_overflow<TIMER_NUM, PRESCALER, PIN_B, 1>(); \
	}                                                                                     \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPB_vect))                                              \
	{                                                                                     \
		timer::isr_handler_pulse::pulse_timer_compare<TIMER_NUM, 1, PIN_B>();             \
	}                                                                                     \
	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect))

namespace timer
{
	/// @cond notdocumented
	template<board::Timer NTIMER_, typename Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer16 : public Timer<NTIMER_>
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using PARENT = Timer<NTIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(TRAIT::IS_16BITS, "TIMER must be a 16 bits timer");

	public:
		using CALCULATOR = Calculator<NTIMER>;
		using TPRESCALER = typename CALCULATOR::PRESCALER;
		static constexpr const TPRESCALER PRESCALER = PRESCALER_;

		explicit PulseTimer16(uint16_t pulse_frequency) : Timer<NTIMER>{TCCRA_MASK(), TCCRB_MASK()}
		{
			TRAIT::ICR = CALCULATOR::PWM_ICR_counter(PRESCALER, pulse_frequency);
		}

	private:
		static constexpr uint8_t TCCRA_MASK()
		{
			// If 16 bits, use ICR1 FastPWM
			return TRAIT::F_PWM_ICR_TCCRA;
		}
		static constexpr uint8_t TCCRB_MASK()
		{
			// If 16 bits, use ICR1 FastPWM and prescaler forced to best fit all pulse frequency
			return TRAIT::F_PWM_ICR_TCCRB | TRAIT::TCCRB_prescaler(PRESCALER);
		}
	};

	template<board::Timer NTIMER_, typename Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer8 : public Timer<NTIMER_>
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using PARENT = Timer<NTIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(!TRAIT::IS_16BITS, "TIMER must be an 8 bits timer");

	public:
		using CALCULATOR = Calculator<NTIMER>;
		using TPRESCALER = typename CALCULATOR::PRESCALER;
		static constexpr const TPRESCALER PRESCALER = PRESCALER_;

		explicit PulseTimer8(uint16_t pulse_frequency)
			: Timer<NTIMER>{TCCRA_MASK(), TCCRB_MASK(), TIMSK_int_mask()}, MAX{OVERFLOW_COUNTER(pulse_frequency)}
		{
			// If 8 bits timer, then we need ISR on Overflow and Compare A/B
			interrupt::register_handler(*this);
		}

	private:
		bool overflow()
		{
			++count_;
			if (count_ == MAX) count_ = 0;
			return (count_ == 0);
		}

		static constexpr uint8_t TCCRA_MASK()
		{
			// If 8 bits, use CTC/TOV ISR
			return 0;
		}
		static constexpr uint8_t TCCRB_MASK()
		{
			// If 8 bits, use CTC/TOV ISR with prescaler forced best fit max pulse width
			return TRAIT::TCCRB_prescaler(PRESCALER);
		}
		static constexpr uint8_t TIMSK_int_mask()
		{
			return TRAIT::TIMSK_int_mask(uint8_t(TimerInterrupt::OVERFLOW | TimerInterrupt::OUTPUT_COMPARE_A
												 | TimerInterrupt::OUTPUT_COMPARE_B));
		}
		static constexpr uint8_t OVERFLOW_COUNTER(uint16_t pulse_frequency)
		{
			return F_CPU / 256UL / BV16(uint8_t(PRESCALER)) / pulse_frequency;
		}

	private:
		const uint8_t MAX;
		uint8_t count_;

		friend struct isr_handler_pulse;
	};
	/// @endcond

	/**
	 * Special kind of `timer::Timer`, specialized in emitting pulses with accurate
	 * width, according to a slow frequency.
	 * This is typically useful for controlling servos, which need a pulse with a
	 * width range from ~1000us to ~2000us, send every 20ms, ie with a 50Hz frequency.
	 * This implementation ensures a good pulse width precision for both 16-bits
	 * and 8-bits timers.
	 * 
	 * Note: if @p NTIMER_ is an 8-bits Timer, then one of the following macros
	 * must be used to register the necessary ISR for the PulseTimer to work correctly:
	 * - REGISTER_PULSE_TIMER8_AB_ISR()
	 * - REGISTER_PULSE_TIMER8_A_ISR()
	 * - REGISTER_PULSE_TIMER8_B_ISR()
	 * 
	 * For concrete usage, you can check the following examples, provided with
	 * FastArduino source:
	 * - analog/PWM3
	 * - analog/PWM4
	 * - motors/Servo1
	 * - motors/Servo2
	 * 
	 * @tparam NTIMER_ the board::Timer to use for this PulseTimer
	 * @tparam PRESCALER_ the prescaler value to use for this PulseTimer; it shall
	 * be calculated with `timer::Calculator<NTIMER_>::PulseTimer_prescaler()`
	 * @tparam T the type (`uint8_t` or `uint16_t`) of the Timer determined by @p NTIMER_;
	 * you should not specify it yourself, as its default value alwayy matches @p NTIMER_.
	 * 
	 * @sa timer::Timer
	 * @sa timer::Calculator::PulseTimer_prescaler
	 * @sa REGISTER_PULSE_TIMER8_AB_ISR
	 * @sa REGISTER_PULSE_TIMER8_A_ISR
	 * @sa REGISTER_PULSE_TIMER8_B_ISR
	 */
	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_,
			 typename T = typename board_traits::Timer_trait<NTIMER_>::TYPE>
	class PulseTimer : public timer::Timer<NTIMER_>
	{
		static_assert(types_traits::is_uint8_or_uint16<T>(), "T must be either uint8_t or uint16_t");

	public:
		/**
		 * Create a PulseTimer with the provided @p pulse_frequency.
		 * @param pulse_frequency the frequency, in Hz, at which pulses will be
		 * generated; this frequency must match the @p PRESCALER_ template parameter. 
		 */
		explicit PulseTimer(UNUSED uint16_t pulse_frequency) : timer::Timer<NTIMER_>{0, 0} {}
	};

	/// @cond notdocumented
	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer<NTIMER_, PRESCALER_, uint8_t> : public timer::PulseTimer8<NTIMER_, PRESCALER_>
	{
	public:
		explicit PulseTimer(uint16_t pulse_frequency) : timer::PulseTimer8<NTIMER_, PRESCALER_>{pulse_frequency} {}
	};

	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer<NTIMER_, PRESCALER_, uint16_t> : public timer::PulseTimer16<NTIMER_, PRESCALER_>
	{
	public:
		explicit PulseTimer(uint16_t pulse_frequency) : timer::PulseTimer16<NTIMER_, PRESCALER_>{pulse_frequency} {}
	};
	/// @endcond

	/// @cond notdocumented
	// All PCI-related methods called by pre-defined ISR are defined here
	struct isr_handler_pulse
	{
		template<uint8_t TIMER_NUM_, board::PWMPin PIN_, uint8_t COM_NUM_>
		static constexpr board::Timer pulse_timer_check()
		{
			constexpr board::Timer NTIMER = isr_handler::check_timer<TIMER_NUM_>();
			using TRAIT = board_traits::Timer_trait<NTIMER>;
			static_assert(!TRAIT::IS_16BITS, "TIMER_NUM must be an 8 bits Timer");
			using PINT = board_traits::Timer_COM_trait<NTIMER, COM_NUM_>;
			static_assert(PIN_ == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");
			return NTIMER;
		}

		template<uint8_t TIMER_NUM_, typename timer::Calculator<(board::Timer) TIMER_NUM_>::PRESCALER PRESCALER_,
				 board::PWMPin PIN_, uint8_t COM_NUM_>
		static void pulse_timer_overflow()
		{
			static constexpr board::Timer NTIMER = pulse_timer_check<TIMER_NUM_, PIN_, COM_NUM_>();
			using PT = timer::PulseTimer8<NTIMER, PRESCALER_>;
			if (interrupt::HandlerHolder<PT>::handler()->overflow())
			{
				using PWMTRAIT = board_traits::PWMPin_trait<PIN_>;
				gpio::FastPinType<PWMTRAIT::ACTUAL_PIN>::set();
			}
		}

		template<uint8_t TIMER_NUM_, typename timer::Calculator<(board::Timer) TIMER_NUM_>::PRESCALER PRESCALER_,
				 board::PWMPin PINA_, uint8_t COMA_NUM_, board::PWMPin PINB_, uint8_t COMB_NUM_>
		static void pulse_timer_overflow()
		{
			static constexpr board::Timer NTIMER = pulse_timer_check<TIMER_NUM_, PINA_, COMA_NUM_>();
			pulse_timer_check<TIMER_NUM_, PINB_, COMB_NUM_>();
			using PT = timer::PulseTimer8<NTIMER, PRESCALER_>;
			if (interrupt::HandlerHolder<PT>::handler()->overflow())
			{
				using PINTA = board_traits::Timer_COM_trait<NTIMER, COMA_NUM_>;
				using PINTB = board_traits::Timer_COM_trait<NTIMER, COMB_NUM_>;
				if (PINTA::OCR != 0)
				{
					using PWMTRAIT = board_traits::PWMPin_trait<PINA_>;
					gpio::FastPinType<PWMTRAIT::ACTUAL_PIN>::set();
				}
				if (PINTB::OCR != 0)
				{
					using PWMTRAIT = board_traits::PWMPin_trait<PINB_>;
					gpio::FastPinType<PWMTRAIT::ACTUAL_PIN>::set();
				}
			}
		}

		template<uint8_t TIMER_NUM_, uint8_t COM_NUM_, board::PWMPin PIN_> static void pulse_timer_compare()
		{
			static constexpr board::Timer NTIMER = pulse_timer_check<TIMER_NUM_, PIN_, COM_NUM_>();
			using PINT = board_traits::Timer_COM_trait<NTIMER, COM_NUM_>;
			static_assert(PIN_ == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");
			using PWMTRAIT = board_traits::PWMPin_trait<PIN_>;
			gpio::FastPinType<PWMTRAIT::ACTUAL_PIN>::clear();
		}
	};
	/// @endcond
}

#endif /* PULSE_TIMER_HH */
/// @endcond
