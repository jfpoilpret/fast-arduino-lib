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
 * Timer API.
 */
#ifndef TIMER_HH
#define TIMER_HH

#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include <stddef.h>
#include "interrupts.h"
#include "utilities.h"

// Generic macros to register ISR on Timer
//=========================================
/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::Timer
 * with a callback method in CTC mode.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_TIMER_COMPARE_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)    \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                               \
	{                                                                      \
		timer::isr_handler::check_timer<TIMER_NUM>();                      \
		interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::Timer
 * with a callback function in CTC mode.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_TIMER_COMPARE_ISR_FUNCTION(TIMER_NUM, CALLBACK)  \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                      \
	{                                                             \
		timer::isr_handler::check_timer<TIMER_NUM>();             \
		interrupt::CallbackHandler<void (*)(), CALLBACK>::call(); \
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for a timer::Timer.
 * This may be needed when using timer CTC mode but when you don't need any
 * callback.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 */
#define REGISTER_TIMER_COMPARE_ISR_EMPTY(TIMER_NUM)                             \
	extern "C" void CAT3(TIMER, TIMER_NUM, _COMPA_vect)(void) NAKED_SIGNAL;     \
	void CAT3(TIMER, TIMER_NUM, _COMPA_vect)(void)                              \
	{                                                                           \
		timer::isr_handler::check_timer<TIMER_NUM>();                           \
		__asm__ __volatile__("reti" ::);                                        \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for the Counter Overflow
 * of a timer::Timer.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_TIMER_OVERFLOW_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)   \
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))                                 \
	{                                                                      \
		timer::isr_handler::check_timer<TIMER_NUM>();                      \
		interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for the Counter Overflow
 * of a timer::Timer.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_TIMER_OVERFLOW_ISR_FUNCTION(TIMER_NUM, CALLBACK) \
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))                        \
	{                                                             \
		timer::isr_handler::check_timer<TIMER_NUM>();             \
		interrupt::CallbackHandler<void (*)(), CALLBACK>::call(); \
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for the Counter Overflow
 * of a timer::Timer.
 * This would normally not be needed.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 */
#define REGISTER_TIMER_OVERFLOW_ISR_EMPTY(TIMER_NUM)                        \
	extern "C" void CAT3(TIMER, TIMER_NUM, _OVF_vect)(void) NAKED_SIGNAL;   \
	void CAT3(TIMER, TIMER_NUM, _OVF_vect)(void)                            \
	{                                                                       \
		timer::isr_handler::check_timer<TIMER_NUM>();                       \
		__asm__ __volatile__("reti" ::);                                    \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for the Input Capture
 * of a timer::Timer.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_TIMER_CAPTURE_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)           \
	ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))                                       \
	{                                                                             \
		timer::isr_handler::timer_capture_method<TIMER_NUM, HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for the Input Capture
 * of a timer::Timer.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_TIMER_CAPTURE_ISR_FUNCTION(TIMER_NUM, CALLBACK)           \
	ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))                                \
	{                                                                      \
		timer::isr_handler::timer_capture_function<TIMER_NUM, CALLBACK>(); \
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for the Input Capture
 * of a timer::Timer.
 * This would normally not be needed.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 */
#define REGISTER_TIMER_CAPTURE_ISR_EMPTY(TIMER_NUM)                         \
	extern "C" void CAT3(TIMER, TIMER_NUM, _CAPT_vect)(void) NAKED_SIGNAL;  \
	void CAT3(TIMER, TIMER_NUM, _CAPT_vect)(void)                           \
	{                                                                       \
		timer::isr_handler::check_timer_capture<TIMER_NUM>();               \
		__asm__ __volatile__("reti" ::);                                    \
	}

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_TIMER_COMPARE_ISR_METHOD`, `REGISTER_TIMER_OVERFLOW_ISR_METHOD`
 * or `REGISTER_TIMER_CAPTURE_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_TIMER_ISR_HANDLERS_FRIEND \
	friend struct timer::isr_handler;  \
	DECL_TIMER_COMP_FRIENDS            \
	DECL_TIMER_OVF_FRIENDS             \
	DECL_TIMER_CAPT_FRIENDS

/**
 * Defines all API to manipulate AVR Timers.
 * In order to properly use Timers, some concepts are important to understand:
 * - Frequency and Prescaler
 * - BOTTOM, TOP and MAX values
 * - Modes of operation
 * - Output modes
 * 
 * These concepts are described in details in AVR datasheets.
 */
namespace timer
{
	/**
	 * Defines the mode of operation of a timer.
	 */
	enum class TimerMode : uint8_t
	{
		/** 
		 * Timer "Normal" mode: counter is incremented up to maximum value (0xFF
		 * for 8-bits Timer, 0xFFFF for 16-bits Timer), and then resets to 0.
		 * Timer Overflow may generate an interrupt.
		 */
		NORMAL,
		/**
		 * Timer "Clear Timer on Compare match" mode: counter is incremented until
		 * it reaches "TOP" (OCRxA register value), then it is reset to 0.
		 * Reaching TOP may trigger an interrupt.
		 */
		CTC,
		/**
		 * Timer "Fast Phase Width Modulation" mode: counter is incremented until
		 * it reaches MAX value (0xFF for 8-bits Timer, 0x3FF for 16-bits Timer).
		 * In this mode the timer can be linked to PWM output pins to be 
		 * automatically set or cleared when:
		 * - counter reaches MAX
		 * - counter reaches TOP (OCRxA or OCRxB)
		 * Reaching TOP and MAX may trigger interrupts.
		 */
		FAST_PWM,
		/**
		 * Timer "Phase Correct Pulse Width Modulation" mode: counter is 
		 * incremented until MAX (0xFF for 8-bits Timer, 0x3FF for 16-bits Timer),
		 * then decremented until 0, and incremented again...
		 * In this mode the timer can be linked to PWM output pins to be 
		 * automatically set or cleared when:
		 * - counter reaches MAX
		 * - counter reaches TOP (OCRxA or OCRxB)
		 * Reaching TOP and MAX may trigger interrupts.
		 */
		PHASE_CORRECT_PWM
	};

	/**
	 * Defines the interrupts that can be handled by a timer.
	 * @sa Timer::set_interrupts()
	 */
	enum class TimerInterrupt : uint8_t
	{
		/** This interrupt occurs when the counter overflows it maximum value. */
		OVERFLOW = board_traits::TimerInterrupt::OVERFLOW,
		/** This interrupt occurs when the counter reached `OCRA`. */
		OUTPUT_COMPARE_A = board_traits::TimerInterrupt::OUTPUT_COMPARE_A,
		/** This interrupt occurs when the counter reached `OCRB`. */
		OUTPUT_COMPARE_B = board_traits::TimerInterrupt::OUTPUT_COMPARE_B,
		/** 
		 * This interrupt occurs when the counter reached `OCRC`.
		 * Note that this interrupt is not supported by all timers.
		 */
		OUTPUT_COMPARE_C = board_traits::TimerInterrupt::OUTPUT_COMPARE_C,
		/** 
		 * This interrupt occurs during input capture, i.e. when a signal (rising
		 * or falling edge, according to currently set `TimerInputCapture`) is
		 * detected.
		 * Note that this interrupt is not supported by all timers.
		 */
		INPUT_CAPTURE = board_traits::TimerInterrupt::INPUT_CAPTURE
	};

	/**
	 * Combine 2 timer interrupts for use with `Timer.set_interrupts()`.
	 * @sa Timer.set_interrupts()
	 */
	constexpr TimerInterrupt operator|(TimerInterrupt i1, TimerInterrupt i2)
	{
		return TimerInterrupt(uint8_t(i1) | uint8_t(i2));
	}

	/**
	 * Defines the "connection" between this timer and specific PWM output pins.
	 */
	enum class TimerOutputMode : uint8_t
	{
		/** No connection for this pin: pin is unaffected by timer operation. */
		DISCONNECTED,
		/** Pin is toggled on Compare Match. */
		TOGGLE,
		/** 
		 * Pin is cleared on Compare Match. For TimerMode::FAST_PWM and
		 * TimerMode::PHASE_CORRECT_PWM modes, pin will also be set when counter
		 * reaches BOTTOM (0). This is also known as "non-inverting" mode).
		 */
		NON_INVERTING,
		/** 
		 * Pin is set on Compare Match. For TimerMode::FAST_PWM and
		 * TimerMode::PHASE_CORRECT_PWM modes, pin will also be cleared when 
		 * counter reaches BOTTOM (0). This is also known as "inverting" mode).
		 */
		INVERTING
	};

	/**
	 * Defines the type of input capture we want for a timer.
	 */
	enum class TimerInputCapture : uint8_t
	{
		/** Input capture needed on rising edge of ICP pin. */
		RISING_EDGE,
		/** Input capture needed on falling edge of ICP pin. */
		FALLING_EDGE
	};

	/**
	 * Defines a set of calculation methods for the given @p NTIMER_
	 * The behavior of these methods is specific to each AVR Timer are there can
	 * be many important differences between 2 timers on AVR.
	 * 
	 * @tparam NTIMER_ the timer for which we need calculation methods
	 * @sa board::Timer
	 */
	template<board::Timer NTIMER_> struct Calculator
	{
	public:
		/**
		 * The timer for which calculation methods are provided.
		 */
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using TRAIT = board_traits::Timer_trait<NTIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;

	public:
		/** 
		 * The timer type: either `uint8_t` or `uint16_t`; this is defined and
		 * not changeable for each Timer. Timer counter and max values have 
		 * this type.
		 */
		using TYPE = typename TRAIT::TYPE;
		/**
		 * The type (`enum`) of the possible prescaler values for this timer;
		 * this is defined and not changeable for each Timer.
		 */
		using PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		/**
		 * The maximum value you can use for this timer in PWM modes: using this
		 * value will set PWM duty as 100%.
		 */
		static constexpr const TYPE PWM_MAX = TRAIT::MAX_PWM;

		/**
		 * Computes the ideal prescaler to use for this timer, in order to generate
		 * timer ticks of @p us_per_tick microseconds or less.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 * 
		 * @param us_per_tick the maximum number of us to be reached by one
		 * counter tick
		 * @return the best prescaler to use
		 */
		static constexpr PRESCALER tick_prescaler(uint32_t us_per_tick)
		{
			return best_tick_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us_per_tick);
		}

		/**
		 * Computes the number of microseconds reached for a given number of @p ticks
		 * with a given @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 * 
		 * @param prescaler the prescaler used for this calculation
		 * @param ticks the number of ticks for which we want to know the number of 
		 * microseconds elapsed
		 * @return the number of microseconds elapsed for @p ticks with @p prescaler
		 */
		static constexpr uint32_t ticks_to_us(PRESCALER prescaler, TYPE ticks)
		{
			return uint32_t(ticks) * bits::BV16(uint8_t(prescaler)) / INST_PER_US;
		}

		/**
		 * Computes the number of ticks needed for reaching the given @p us microseconds
		 * with a given @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 * 
		 * @param prescaler the prescaler used for this calculation
		 * @param us the number of microseconds we want to reach
		 * @return the number of ticks needed in order to reach @p us 
		 * microseconds elapsed
		 */
		static constexpr TYPE us_to_ticks(PRESCALER prescaler, uint32_t us)
		{
			return TYPE(us * INST_PER_US / bits::BV16(uint8_t(prescaler)));
		}

		/**
		 * Computes the ideal prescaler value to use for this timer, in
		 * TimerMode::CTC mode, in order to be able to count up to @p us
		 * microseconds.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param us the number of microseconds we want to be able to count up to
		 * @return the best prescaler to use
		 * @sa CTC_counter()
		 * @sa CTC_frequency()
		 */
		static constexpr PRESCALER CTC_prescaler(uint32_t us)
		{
			return best_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us);
		}

		/**
		 * Computes the frequency at which this timer would perform, in 
		 * TimerMode::CTC mode, if it was using @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param prescaler the prescaler value for which to compute timer
		 * frequency
		 * @return timer frequency when using @p prescaler
		 * @sa CTC_prescaler()
		 */
		static constexpr uint32_t CTC_frequency(PRESCALER prescaler)
		{
			return F_CPU / bits::BV16(uint8_t(prescaler));
		}

		/**
		 * Computes the value of counter to use for this timer, in TimerMode::CTC
		 * mode, with @p prescaler, in order to reach @p us microseconds.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided constant arguments.
		 * 
		 * @param prescaler the prescaler used for this timer
		 * @param us the number of microseconds to reach with this timer in CTC mode
		 * @return the MAX value to be used in order to reach @p us
		 * @sa CTC_prescaler()
		 * @sa Timer::begin(PRESCALER, TYPE)
		 */
		static constexpr TYPE CTC_counter(PRESCALER prescaler, uint32_t us)
		{
			return (TYPE) prescaler_quotient(prescaler, us) - 1;
		}

		/**
		 * Verifies that the given prescaler @p prescaler is suitable for this timer in
		 * TimerMode::CTC in order to be able to reach @p us microseconds.
		 * This is normally not needed but can be helpful in a `static_assert` in
		 * your code.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided constant arguments.
		 * You could use this method in `static_assert()` in order to check that
		 * the requested period @p us is OK for this timer.
		 *
		 * @param prescaler the prescaler used for this timer
		 * @param us the number of microseconds to reach with this timer in CTC mode
		 * @retval `true` if @p prescaler and @p us are adequate for @p TIMER
		 * @retval `false` if @p prescaler and @p us are **NOT** adequate for @p TIMER
		 * @sa CTC_prescaler()
		 * @sa CTC_counter()
		 */
		static constexpr bool is_adequate_for_CTC(PRESCALER prescaler, uint32_t us)
		{
			return prescaler_is_adequate(prescaler_quotient(prescaler, us));
		}

		/**
		 * Computes the ideal prescaler value to use for this timer, in
		 * TimerMode::FAST_PWM mode, in order to be able to reach at least
		 * @p pwm_frequency.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param pwm_frequency the PWM frequency we want to reach
		 * @return the best prescaler to use
		 * @sa FastPWM_frequency()
		 */
		static constexpr PRESCALER FastPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (PWM_MAX + 1UL));
		}

		/**
		 * Computes the frequency at which this timer would perform, in 
		 * TimerMode::FAST_PWM mode, if it was using @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param prescaler the prescaler value for which to compute timer
		 * frequency
		 * @return timer frequency when using @p prescaler
		 * @sa FastPWM_prescaler()
		 */
		static constexpr uint16_t FastPWM_frequency(PRESCALER prescaler)
		{
			return F_CPU / bits::BV16(uint8_t(prescaler)) / (PWM_MAX + 1UL);
		}

		/**
		 * Computes the ideal prescaler value to use for this timer, in
		 * TimerMode::PHASE_CORRECT_PWM mode, in order to be able to reach at
		 * least @p pwm_frequency.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param pwm_frequency the PWM frequency we want to reach
		 * @return the best prescaler to use
		 * @sa PhaseCorrectPWM_frequency()
		 */
		static constexpr PRESCALER PhaseCorrectPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (2UL * PWM_MAX));
		}

		/**
		 * Computes the frequency at which this timer would perform, in 
		 * TimerMode::PHASE_CORRECT_PWM mode, if it was using @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param prescaler the prescaler value for which to compute timer
		 * frequency
		 * @return timer frequency when using @p prescaler
		 * @sa PhaseCorrectPWM_prescaler()
		 */
		static constexpr uint16_t PhaseCorrectPWM_frequency(PRESCALER prescaler)
		{
			return F_CPU / bits::BV16(uint8_t(prescaler)) / (2UL * PWM_MAX);
		}

		/**
		 * Computes the ideal prescaler value to use for this timer, when used
		 * in timer::PulseTimer, in order to be able to reach at least
		 * @p pulse_frequency with a maximum pulse width of @p max_pulse_width_us.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param max_pulse_width_us the maximum pulse width, in microseconds,
		 * to produce
		 * @param pulse_frequency the pulse frequency we want to reach
		 * @return the best prescaler to use
		 * @sa PulseTimer_value()
		 */
		static constexpr PRESCALER PulseTimer_prescaler(uint16_t max_pulse_width_us, uint16_t pulse_frequency)
		{
			return TRAIT::IS_16BITS ? PWM_ICR_prescaler(pulse_frequency) : CTC_prescaler(max_pulse_width_us);
		}

		/**
		 * Computes the ideal value to use for this timer, when used
		 * in timer::PulseTimer, in order to produce a pulse of the required width
		 * @p period_us with the given @p prescaler.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * Note that this method is directly called as needed by timer::PulseTimer,
		 * hence you generally won't need to call it yourself.
		 *
		 * @param prescaler the prescaler value for which to compute value
		 * @param period_us the pulse width, in microseconds, to produce
		 * @return the value to use to produce the required pulse
		 * @sa PulseTimer_prescaler()
		 */
		static constexpr TYPE PulseTimer_value(PRESCALER prescaler, uint16_t period_us)
		{
			return CTC_counter(prescaler, period_us);
		}

		/**
		 * Computes the ideal prescaler value to use for this timer, when used
		 * in timer::PulseTimer, in order to be able to reach at least
		 * @p pulse_frequency with a maximum pulse width of @p max_pulse_width_us.
		 * This method applies only to 16-bits timers, for which PulseTimer uses
		 * FastPWM mode with register `ICR` as TOP value.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * Note that this method is directly called as needed by timer::PulseTimer,
		 * hence you generally won't need to call it yourself.
		 *
		 * @param pwm_frequency the pulse frequency we want to reach
		 * @return the best prescaler to use
		 * @sa PWM_ICR_frequency()
		 * @sa PWM_ICR_counter()
		 */
		static constexpr PRESCALER PWM_ICR_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (TRAIT::MAX_PWM + 1UL));
		}

		/**
		 * Computes the frequency at which this timer would perform, when used
		 * in timer::PulseTimer, if it was using @p prescaler and @p counter.
		 * This method applies only to 16-bits timers, for which PulseTimer uses
		 * FastPWM mode with register `ICR` as TOP value.
		 *
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param prescaler the prescaler value for which to compute timer
		 * frequency
		 * @param counter the counter value for which to compute timer
		 * frequency
		 * @return timer frequency
		 * @sa PWM_ICR_prescaler()
		 * @sa PWM_ICR_counter()
		 */
		static constexpr uint16_t PWM_ICR_frequency(PRESCALER prescaler, uint16_t counter)
		{
			return F_CPU / bits::BV16(uint8_t(prescaler)) / counter;
		}

		/**
		 * Computes the ideal counter TOP value to use for this timer, when used
		 * in timer::PulseTimer, in order to be able to reach at least
		 * @p pulse_frequency with the given @p prescaler.
		 * This method applies only to 16-bits timers, for which PulseTimer uses
		 * FastPWM mode with register `ICR` as TOP value.
		 *
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided a constant argument.
		 *
		 * @param prescaler the prescaler value for which to compute timer
		 * frequency
		 * @param pwm_frequency the pulse frequency we want to reach
		 * @return ideal value to set for `ICR` register
		 * @sa PWM_ICR_prescaler()
		 * @sa PWM_ICR_frequency()
		 */
		static constexpr uint16_t PWM_ICR_counter(PRESCALER prescaler, uint16_t pwm_frequency)
		{
			return F_CPU / bits::BV16(uint8_t(prescaler)) / pwm_frequency;
		}

	private:
		static constexpr uint32_t prescaler_quotient(PRESCALER prescaler, uint32_t us)
		{
			return (INST_PER_US * us) / bits::BV16(uint8_t(prescaler));
		}

		static constexpr uint32_t prescaler_remainder(PRESCALER prescaler, uint32_t us)
		{
			return (INST_PER_US * us) % bits::BV16(uint8_t(prescaler));
		}

		static constexpr bool prescaler_is_adequate(uint32_t quotient)
		{
			return (quotient > 1) && (quotient < TRAIT::MAX_COUNTER);
		}

		template<size_t N> static constexpr PRESCALER best_prescaler(const PRESCALER (&prescalers)[N], uint32_t us)
		{
			// Find the best prescaler:
			// - quotient is in ]1; MAX_COUNTER[
			// - smallest remainder
			// - largest quotient
			uint32_t smallest_remainder = UINT32_MAX;
			uint32_t largest_quotient = 0;
			PRESCALER current = prescalers[N - 1];
			for (size_t i = 0; i < N; ++i)
			{
				PRESCALER prescaler = prescalers[i];
				uint32_t quotient = prescaler_quotient(prescaler, us);
				if (prescaler_is_adequate(quotient))
				{
					uint32_t remainder = prescaler_remainder(prescaler, us);
					if (remainder > smallest_remainder) continue;
					if ((remainder == smallest_remainder) && (quotient <= largest_quotient)) continue;
					current = prescaler;
					smallest_remainder = remainder;
					largest_quotient = quotient;
				}
			}
			return current;
		}

		static constexpr bool prescaler_is_adequate_for_frequency(PRESCALER prescaler, uint32_t freq)
		{
			return (F_CPU / (uint32_t) bits::BV16(uint8_t(prescaler))) > freq;
		}

		template<size_t N>
		static constexpr PRESCALER best_frequency_prescaler(const PRESCALER (&prescalers)[N], uint32_t freq)
		{
			// Best prescaler is biggest possible divider, hence try all prescalers from largest to smallest
			for (size_t i = 0; i < N; ++i)
			{
				PRESCALER prescaler = prescalers[N - 1 - i];
				if (prescaler_is_adequate_for_frequency(prescaler, freq)) return prescaler;
			}
			// If no prescaler is adequate then return largest divider
			return prescalers[N - 1];
		}

		static constexpr bool prescaler_is_adequate_for_tick(PRESCALER prescaler, uint32_t us)
		{
			return (prescaler_quotient(prescaler, us) >= 1);
		}

		template<size_t N> static constexpr PRESCALER best_tick_prescaler(const PRESCALER (&prescalers)[N], uint32_t us)
		{
			// Best prescaler is biggest possible divider, hence try all prescalers from largest to smallest
			for (size_t i = 0; i < N; ++i)
			{
				PRESCALER prescaler = prescalers[N - 1 - i];
				if (prescaler_is_adequate_for_tick(prescaler, us)) return prescaler;
			}
			// If no prescaler is adequate then return largest divider
			return prescalers[N - 1];
		}
	};

	/**
	 * General API to handle an AVR timer.
	 * Note that many timer usages will require ISR registration with one of
	 * the macros defined in this header file.
	 * 
	 * @tparam NTIMER_ the AVR timer to use for this Timer
	 * @sa board::Timer
	 */
	template<board::Timer NTIMER_> class Timer
	{
	public:
		/** The Board timer used by this Timer. */
		static constexpr const board::Timer NTIMER = NTIMER_;

	protected:
		/// @cond notdocumented
		using TRAIT = board_traits::Timer_trait<NTIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;
		/// @endcond

	public:
		/// @cond notdocumented
		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
		/// @endcond
		
		/**
		 * The type of this timer's counter (either `uint8_t` or `uint16_t`).
		 */
		using TYPE = typename TRAIT::TYPE;

		/**
		 * The Calculator type for this Timer.
		 */
		using CALCULATOR = Calculator<NTIMER>;

		/**
		 * The enum type listing all available precaler values for this timer.
		 */
		using PRESCALER = typename PRESCALERS_TRAIT::TYPE;

		/**
		 * The maximum value that can be set to this timer's counter.
		 */
		static constexpr const TYPE TIMER_MAX = TRAIT::MAX_COUNTER - 1;

		/**
		 * The maximum value that can be set to this timer's counter in PWM mode.
		 */
		static constexpr const TYPE PWM_MAX = TRAIT::MAX_PWM;

		/**
		 * The pin that is used for Input Capture feature, if supported by this 
		 * timer, or `board::DigitalPin::NONE` if not.
		 */
		static constexpr const board::DigitalPin ICP_PIN = TRAIT::ICP_PIN;

		/**
		 * Construct a new Timer handler and initialize its mode.
		 * Note this constructor does *not* start the timer.
		 * @param timer_mode the mode to initalize this timer with
		 * @param prescaler the prescale enum value to use for this timer
		 * @param interrupts default interrupts that will be used when timer is 
		 * started; note that some interrupts are not supported by all timers, if
		 * used here, they will silently be ignored.
		 * @sa begin()
		 * @sa set_timer_mode()
		 * @sa set_interrupts()
		 * @sa set_prescaler()
		 */
		Timer(TimerMode timer_mode, PRESCALER prescaler, TimerInterrupt interrupts = TimerInterrupt(0))
			: tccra_{timer_mode_TCCRA(timer_mode)},
			  tccrb_{uint8_t(timer_mode_TCCRB(timer_mode) | TRAIT::TCCRB_prescaler(prescaler))},
			  timsk_{TRAIT::TIMSK_int_mask(uint8_t(interrupts))}
		{}

		/**
		 * Set the list of interrupts that must be triggered by this timer.
		 * If you do not want any interrupt triggered, then call this method
		 * with no argument.
		 * To select several interrupts, just "or" them together:
		 * @code
		 * timer.set_interrupts(TimerInterrupt::OVERFLOW | TimerInterrupt::INPUT_CAPTURE);
		 * @endcode
		 * 
		 * @param interrupts interrupts that will be used when timer is 
		 * started; note that some interrupts are not supported by all timers, if
		 * used here, they will silently be ignored.
		 */
		void set_interrupts(TimerInterrupt interrupts = TimerInterrupt(0))
		{
			timsk_ = TRAIT::TIMSK_int_mask(uint8_t(interrupts));
			// Check if timer is currently running
			if (TRAIT::TCCRB) utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, timsk_);
		}

		/**
		 * Add new interrupts to be enabled to the current list of interrupts
		 * that must be triggered by this timer.
		 * To select several interrupts, just "or" them together:
		 * @code
		 * timer.enable_interrupts(TimerInterrupt::OVERFLOW | TimerInterrupt::INPUT_CAPTURE);
		 * @endcode
		 * 
		 * @param interrupts additional interrupts that will be used when timer is 
		 * started; note that some interrupts are not supported by all timers, if
		 * used here, they will silently be ignored.
		 */
		void enable_interrupts(TimerInterrupt interrupts)
		{
			timsk_ |= TRAIT::TIMSK_int_mask(uint8_t(interrupts));
			// Check if timer is currently running
			if (TRAIT::TCCRB) utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, timsk_);
		}

		/**
		 * Remove interrupts from the current list of enabled interrupts triggered
		 * by this timer.
		 * To select several interrupts to be disabled, just "or" them together:
		 * @code
		 * timer.disable_interrupts(TimerInterrupt::OVERFLOW | TimerInterrupt::INPUT_CAPTURE);
		 * @endcode
		 * 
		 * @param interrupts interrupts that will not be used when timer is 
		 * started
		 */
		void disable_interrupts(TimerInterrupt interrupts)
		{
			timsk_ &= bits::COMPL(TRAIT::TIMSK_int_mask(uint8_t(interrupts)));
			// Check if timer is currently running
			if (TRAIT::TCCRB) utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, timsk_);
		}

		/**
		 * Test if interrupts are currently enabled for this timers.
		 * To select several interrupts to be tested, just "or" them together:
		 * @code
		 * if (timer.are_interrupts_enabled(TimerInterrupt::OVERFLOW | TimerInterrupt::INPUT_CAPTURE)) ...
		 * @endcode
		 * 
		 * @param interrupts interrupts to be checked
		 */
		bool are_interrupts_enabled(TimerInterrupt interrupts) const
		{
			uint8_t mask = TRAIT::TIMSK_int_mask(uint8_t(interrupts));
			return (TRAIT::TIMSK_ & mask) == mask;
		}

		/**
		 * Set the input capture mode for this timer.
		 * Input Capture will work only if `set_interrupts()` is called with
		 * `TimerInterrupt::INPUT_CAPTURE`.
		 * Note that some timers do not support input capture; in this situation,
		 * using this method will generate a compiler error.
		 * @param input_capture new input capture mode to use
		 */
		void set_input_capture(TimerInputCapture input_capture)
		{
			static_assert(TRAIT::ICES_TCCRB != 0, "TIMER must support Input Capture");
			utils::set_mask(tccrb_, TRAIT::ICES_TCCRB, input_capture_TCCRB(input_capture));
			// Check if timer is currently running
			if (TRAIT::TCCRB) TRAIT::TCCRB = tccrb_;
		}

		/** 
		 * Return the current `TimerInputCapture` used by this timer.
		 */
		TimerInputCapture input_capture() const
		{
			static_assert(TRAIT::ICES_TCCRB != 0, "TIMER must support Input Capture");
			return TRAIT::TCCRB & TRAIT::ICES_TCCRB ? TimerInputCapture::RISING_EDGE : TimerInputCapture::FALLING_EDGE;
		}

		/**
		 * Set or clear the noise canceller for input capture on this timer.
		 * Note that some timers do not support input capture; in this situation,
		 * using this method will generate a compiler error.
		 * @param cancel_noise if `true`, set noise canceller for this timer's 
		 * input capture; if `false`, clear noise canceller.
		 */
		void set_capture_noise_canceller(bool cancel_noise)
		{
			static_assert(TRAIT::ICNC_TCCRB != 0, "TIMER must support Input Capture");
			if (cancel_noise)
				tccrb_ |= TRAIT::ICNC_TCCRB;
			else
				tccrb_ &= ~TRAIT::ICNC_TCCRB;
			// Check if timer is currently running
			if (TRAIT::TCCRB) TRAIT::TCCRB = tccrb_;
		}

		/**
		 * Tell whether noise canceller for this timer's input capture is active
		 * or not.
		 */
		bool has_capture_noise_canceller() const
		{
			static_assert(TRAIT::ICNC_TCCRB != 0, "TIMER must support Input Capture");
			return TRAIT::TCCRB & TRAIT::ICNC_TCCRB;
		}

		/**
		 * Change timer mode.
		 * @param timer_mode the mode to reset this timer to
		 */
		void set_timer_mode(TimerMode timer_mode)
		{
			utils::set_mask(tccra_, TRAIT::MODE_MASK_TCCRA, timer_mode_TCCRA(timer_mode));
			utils::set_mask(tccrb_, TRAIT::MODE_MASK_TCCRB, timer_mode_TCCRB(timer_mode));
			// Check if timer is currently running
			if (TRAIT::TCCRB)
			{
				if (!TRAIT::TCCRA.is_no_reg()) TRAIT::TCCRA = tccra_;
				TRAIT::TCCRB = tccrb_;
			}
		}

		/**
		 * Get current time mode.
		 * This method exists only for the sake of completeness; if not really
		 * needed, do not use it, as it will generate extra code with no real
		 * added value.
		 */
		TimerMode get_timer_mode() const
		{
			return timer_mode(tccra_, tccrb_);
		}

		/**
		 * Change prescaler for this timer.
		 * @param prescaler the prescale enum value to use for this timer
		 */
		void set_prescaler(PRESCALER prescaler)
		{
			utils::set_mask(tccrb_, TRAIT::CS_MASK_TCCRB, TRAIT::TCCRB_prescaler(prescaler));
			// Check if timer is currently running
			if (TRAIT::TCCRB) TRAIT::TCCRB = tccrb_;
		}

		/**
		 * Start this timer in the currently selected mode, with the provided
		 * @p prescaler value and @p max value. 
		 * This method enables timer interrupts (Compare Match),hence ISR
		 * registration is required.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `begin_()` instead.
		 * @param max the maximum value to use for this timer's counter
		 * @sa end()
		 * @sa begin_(TYPE)
		 */
		void begin(TYPE max = 0)
		{
			synchronized begin_(max);
		}

		/**
		 * Start this timer in the currently selected mode, with the provided
		 * @p prescaler value and @p max value. 
		 * This method enables timer interrupts (Compare Match),hence ISR
		 * registration is required.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `begin()` instead.
		 * @param max the maximum value to use for this timer's counter
		 * @sa end_()
		 * @sa begin(TYPE)
		 */
		void begin_(TYPE max = 0)
		{
			if (!TRAIT::TCCRA.is_no_reg()) TRAIT::TCCRA = tccra_;
			TRAIT::TCCRB = tccrb_;
			// Set timer counter compare match
			TRAIT::OCRA = max;
			if (!TRAIT::CTC_MAX.is_no_reg()) TRAIT::CTC_MAX = max;
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, timsk_);
		}

		/**
		 * Reset current counter to @p ticks .
		 * This method is synchronized if needed (i.e. if this timer is 16 bits).
		 * If you do not need synchronization, then you should better use
		 * `reset_()` instead.
		 * @param ticks number of ticks to reset counter to
		 * 
		 * @sa reset_()
		 */
		void reset(TYPE ticks = 0)
		{
			if (sizeof(TYPE) > 1)
				synchronized reset_(ticks);
			else
				reset_(ticks);
		}

		/**
		 * Reset current counter to @p ticks .
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `reset()` instead.
		 * @param ticks number of ticks to reset counter to
		 * 
		 * @sa reset()
		 */
		void reset_(TYPE ticks = 0)
		{
			TRAIT::TCNT = ticks;
		}

		/**
		 * Return the current counter value for this timer.
		 * This method is synchronized if needed (i.e. if this timer is 16 bits).
		 * If you do not need synchronization, then you should better use
		 * `ticks_()` instead.
		 * @sa ticks_()
		 */
		TYPE ticks()
		{
			if (sizeof(TYPE) > 1)
				synchronized return ticks_();
			else
				return ticks_();
		}

		/**
		 * Return the current counter value for this timer.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `ticks()` instead.
		 * @sa ticks()
		 */
		TYPE ticks_()
		{
			return TRAIT::TCNT;
		}

		/**
		 * Temporarily suspend this timer: the timer does not generate any
		 * interrupt any longer.
		 * Note that this method is synchronized, i.e. it disables AVR interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `suspend_interrupts_()` instead.
		 * @sa resume_interrupts()
		 * @sa suspend_interrupts_()
		 */
		void suspend_interrupts()
		{
			synchronized suspend_interrupts_();
		}

		/**
		 * Temporarily suspend this timer: the timer does not generate any
		 * interrupt any longer.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while AVR interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `suspend_interrupts()` instead.
		 * @sa resume_interrupts_()
		 * @sa suspend_interrupts()
		 */
		void suspend_interrupts_()
		{
			// Clear timer interrupt mode
			utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, uint8_t(0));
		}

		/**
		 * Resume this timer if it was previously suspended: the timer's counter
		 * is reset and the timer starts generating interrupts again.
		 * Note that this method is synchronized, i.e. it disables AVR interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `resume_interrupts_()` instead.
		 * @sa suspend_interrupts()
		 * @sa resume_interrupts_()
		 */
		void resume_interrupts()
		{
			synchronized resume_interrupts_();
		}

		/**
		 * Resume this timer if it was previously suspended: the timer's counter
		 * is reset and the timer starts generating interrupts again.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while AVR interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `resume_interrupts()` instead.
		 * @sa suspend_interrupts_()
		 * @sa resume_interrupts()
		 */
		void resume_interrupts_()
		{
			// Reset timer counter
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, timsk_);
		}

		/**
		 * Check if this timer interrupts are currently suspended, i.e. it does 
		 * not generate any interrupts.
		 * @retval true the timer is currently suspended
		 * @retval false the timer is currently active
		 */
		bool is_interrupt_suspended()
		{
			return (TRAIT::TIMSK_ & TRAIT::TIMSK_MASK) == 0;
		}

		/**
		 * Suspend this timer, ie stop this timer counting (`ticks()` will not change then).
		 * Note that this method is synchronized, i.e. it disables AVR interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `suspend_timer_()` instead.
		 * 
		 * @sa resume_timer()
		 * @sa suspend_timer_()
		 */
		void suspend_timer()
		{
			synchronized suspend_timer_();
		}

		/**
		 * Suspend this timer, ie stop this timer counting (`ticks()` will not change then).
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while AVR interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `suspend_timer()` instead.
		 * 
		 * @sa resume_timer_()
		 * @sa suspend_timer()
		 */
		void suspend_timer_()
		{
			// Check if timer is currently running, and force clock select to 0 (timer stopped)
			if (TRAIT::TCCRB) TRAIT::TCCRB = tccrb_ & uint8_t(~TRAIT::CS_MASK_TCCRB);
		}

		/**
		 * Resume this timer, ie restart counting (`ticks()` will start changing again).
		 * Note that this method is synchronized, i.e. it disables AVR interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `resume_timer_()` instead.
		 * 
		 * @sa suspend_timer()
		 * @sa resume_timer_()
		 */
		void resume_timer()
		{
			synchronized resume_timer_();
		}

		/**
		 * Resume this timer, ie restart counting (`ticks()` will start changing again).
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while AVR interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `resume_timer()` instead.
		 * 
		 * @sa suspend_timer_()
		 * @sa resume_timer()
		 */
		void resume_timer_()
		{
			// Check if timer is currently running
			TRAIT::TCCRB = tccrb_;
		}

		/**
		 * Completely stop this timer: timer interrupts are disabled and counter 
		 * is stopped.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `end_()` instead.
		 * @sa begin()
		 * @sa end_()
		 */
		void end()
		{
			synchronized end_();
		}

		/**
		 * Completely stop this timer: timer interrupts are disabled and counter 
		 * is stopped.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `end()` instead.
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			// Stop timer
			TRAIT::TCCRB = 0;
			// Clear timer interrupt mode (set interrupt on OCRnA compare match)
			utils::set_mask((volatile uint8_t&) TRAIT::TIMSK_, TRAIT::TIMSK_MASK, uint8_t(0));
		}

		/**
		 * Change the output mode for this timer; this enables connection between 
		 * this timer to one of its associated digital output pins.
		 * @tparam COM the index of the output pin for this timer; your program
		 * will not compile if you use an incorrect value.
		 * @param mode the new output mode for this timer and this pin
		 */
		template<uint8_t COM> void set_output_mode(TimerOutputMode mode)
		{
			static_assert(COM < TRAIT::COM_COUNT, "COM must exist for TIMER");
			using COM_TRAIT = board_traits::Timer_COM_trait<NTIMER, COM>;
			utils::set_mask(tccra_, COM_TRAIT::COM_MASK, convert_COM<COM>(mode));
		}

		/**
		 * Change the maximum value for this timer, in relation to one of its
		 * associated digital output pins.
		 * Note that this method is always synchronized, i.e. it disables
		 * interrupts during its call and restores interrupts on return.
		 * 
		 * @tparam COM the index of the output pin for this timer; your program
		 * will not compile if you use an incorrect value.
		 * @param max the new maximum value for this timer and this pin
		 */
		template<uint8_t COM> void set_max(TYPE max)
		{
			static_assert(COM < TRAIT::COM_COUNT, "COM must exist for TIMER");
			using COM_TRAIT = board_traits::Timer_COM_trait<NTIMER, COM>;
			synchronized
			{
				if (max)
					utils::set_mask((volatile uint8_t&) TRAIT::TCCRA, COM_TRAIT::COM_MASK, tccra_);
				else
					utils::set_mask((volatile uint8_t&) TRAIT::TCCRA, COM_TRAIT::COM_MASK,
									convert_COM<COM>(TimerOutputMode::DISCONNECTED));
				COM_TRAIT::OCR = max;
			}
		}

	protected:
		/// @cond notdocumented
		Timer(uint8_t tccra, uint8_t tccrb, uint8_t timsk = 0) : tccra_{tccra}, tccrb_{tccrb}, timsk_{timsk} {}

		template<uint8_t COM> static constexpr uint8_t convert_COM(TimerOutputMode output_mode)
		{
			using COM_TRAIT = board_traits::Timer_COM_trait<NTIMER, COM>;
			if (output_mode == TimerOutputMode::TOGGLE) return COM_TRAIT::COM_TOGGLE;
			if (output_mode == TimerOutputMode::INVERTING) return COM_TRAIT::COM_SET;
			if (output_mode == TimerOutputMode::NON_INVERTING) return COM_TRAIT::COM_CLEAR;
			return COM_TRAIT::COM_NORMAL;
		}

		static constexpr bool TIMSK_int_mask_IS_SUPPORTED(TimerInterrupt interrupt)
		{
			return (TRAIT::TIMSK_int_mask(UINT8_MAX) & TRAIT::TIMSK_int_mask(uint8_t(interrupt)))
				   == TRAIT::TIMSK_int_mask(uint8_t(interrupt));
		}

		static constexpr uint8_t timer_mode_TCCRA(TimerMode timer_mode)
		{
			if (timer_mode == TimerMode::CTC) return TRAIT::CTC_TCCRA;
			if (timer_mode == TimerMode::FAST_PWM) return TRAIT::F_PWM_TCCRA;
			if (timer_mode == TimerMode::PHASE_CORRECT_PWM) return TRAIT::PC_PWM_TCCRA;
			return 0;
		}
		static constexpr uint8_t timer_mode_TCCRB(TimerMode timer_mode)
		{
			if (timer_mode == TimerMode::CTC) return TRAIT::CTC_TCCRB;
			if (timer_mode == TimerMode::FAST_PWM) return TRAIT::F_PWM_TCCRB;
			if (timer_mode == TimerMode::PHASE_CORRECT_PWM) return TRAIT::PC_PWM_TCCRB;
			return 0;
		}

		static constexpr bool is_timer_mode(TimerMode mode, uint8_t TCCRA, uint8_t TCCRB)
		{
			return utils::is_mask_equal(TCCRA, TRAIT::MODE_MASK_TCCRA, timer_mode_TCCRA(mode))
				   && utils::is_mask_equal(TCCRB, TRAIT::MODE_MASK_TCCRB, timer_mode_TCCRB(mode));
		}
		static constexpr TimerMode timer_mode(uint8_t TCCRA, uint8_t TCCRB)
		{
			if (is_timer_mode(TimerMode::CTC, TCCRA, TCCRB)) return TimerMode::CTC;
			if (is_timer_mode(TimerMode::FAST_PWM, TCCRA, TCCRB)) return TimerMode::FAST_PWM;
			if (is_timer_mode(TimerMode::PHASE_CORRECT_PWM, TCCRA, TCCRB)) return TimerMode::PHASE_CORRECT_PWM;
			return TimerMode::NORMAL;
		}

		static constexpr uint8_t input_capture_TCCRB(TimerInputCapture input_capture)
		{
			return (input_capture == TimerInputCapture::RISING_EDGE ? TRAIT::ICES_TCCRB : 0);
		}

		uint8_t tccra_;
		uint8_t tccrb_;
		uint8_t timsk_;
		/// @endcond
	};

	/// @cond notdocumented

	// All PCI-related methods called by pre-defined ISR are defined here
	//====================================================================

	struct isr_handler
	{
		template<board::Timer NTIMER_> static constexpr board::Timer check_timer()
		{
			using TRAIT = board_traits::Timer_trait<NTIMER_>;
			static_assert(TRAIT::PRESCALERS != board_traits::TimerPrescalers::PRESCALERS_NONE,
						  "TIMER_NUM must be an actual Timer in target MCU");
			return NTIMER_;
		}

		template<uint8_t TIMER_NUM_> static constexpr board::Timer check_timer()
		{
			constexpr board::Timer NTIMER = (board::Timer) TIMER_NUM_;
			return check_timer<NTIMER>();
		}

		template<uint8_t TIMER_NUM_> static constexpr board::Timer check_timer_capture()
		{
			constexpr board::Timer NTIMER = check_timer<TIMER_NUM_>();
			static_assert(board_traits::Timer_trait<NTIMER>::ICES_TCCRB != 0,
						  "TIMER_NUM must be Timer supporting capture");
			return NTIMER;
		}

		template<uint8_t TIMER_NUM_, typename T, typename HANDLER_, void (HANDLER_::*CALLBACK_)(T)>
		static void timer_capture_method_helper()
		{
			static constexpr board::Timer NTIMER = check_timer_capture<TIMER_NUM_>();
			using TRAIT = board_traits::Timer_trait<NTIMER>;
			static_assert(sizeof(typename TRAIT::TYPE) == sizeof(T),
						  "CALLBACK argument is not the proper type (should be same as Timer bits size)");
			T capture = TRAIT::ICR;
			interrupt::CallbackHandler<void (HANDLER_::*)(T), CALLBACK_>::call(capture);
		}

		template<uint8_t TIMER_NUM_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(uint8_t)>
		static void timer_capture_method()
		{
			timer_capture_method_helper<TIMER_NUM_, uint8_t, HANDLER_, CALLBACK_>();
		}

		template<uint8_t TIMER_NUM_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(uint16_t)>
		static void timer_capture_method()
		{
			timer_capture_method_helper<TIMER_NUM_, uint16_t, HANDLER_, CALLBACK_>();
		}

		template<uint8_t TIMER_NUM_, typename T, void (*CALLBACK_)(T)> static void timer_capture_function_helper()
		{
			static constexpr board::Timer NTIMER = check_timer_capture<TIMER_NUM_>();
			using TRAIT = board_traits::Timer_trait<NTIMER>;
			static_assert(sizeof(typename TRAIT::TYPE) == sizeof(T),
						  "CALLBACK argument is not the proper type (should be same as Timer bits size)");
			T capture = TRAIT::ICR;
			CALLBACK_(capture);
		}

		template<uint8_t TIMER_NUM_, void (*CALLBACK_)(uint8_t)> static void timer_capture_function()
		{
			timer_capture_function_helper<TIMER_NUM_, uint8_t, CALLBACK_>();
		}

		template<uint8_t TIMER_NUM_, void (*CALLBACK_)(uint16_t)> static void timer_capture_function()
		{
			timer_capture_function_helper<TIMER_NUM_, uint16_t, CALLBACK_>();
		}
	};
	/// @endcond
}

#endif /* TIMER_HH */
/// @endcond
