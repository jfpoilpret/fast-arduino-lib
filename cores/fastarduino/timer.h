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

/// @cond api

/**
 * @file 
 * Timer API.
 */
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
/**
 * Register the necessary ISR (interrupt Service Routine) for a timer::Timer
 * with a callback method in CTC mode.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_TIMER_COMPARE_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), HANDLER, CALLBACK)

/**
 * Register the necessary ISR (interrupt Service Routine) for a timer::Timer
 * with a callback function in CTC mode.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_TIMER_COMPARE_ISR_FUNCTION(TIMER_NUM, CALLBACK)	\
REGISTER_ISR_FUNCTION_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), CALLBACK)

/**
 * Register an enpty ISR (interrupt Service Routine) for a timer::Timer.
 * This may be needed when using timer CTC mode but when you don't need any
 * callback.
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 */
#define REGISTER_TIMER_COMPARE_ISR_EMPTY(TIMER_NUM)	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect));

//TODO Document
#define REGISTER_TIMER_CAPTURE_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)		\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))										\
{																			\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);		\
	using TRAIT = board_traits::Timer_trait<TIMER>;							\
	TRAIT::TYPE capture = TRAIT::ICR;										\
	CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(capture);					\
}

#define REGISTER_TIMER_CAPTURE_ISR_FUNCTION(TIMER_NUM, CALLBACK)			\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))										\
{																			\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);		\
	using TRAIT = board_traits::Timer_trait<TIMER>;							\
	TRAIT::TYPE capture = TRAIT::ICR;										\
	CALLBACK (capture);														\
}

//TODO is an empty interrupt handler useful really?
#define REGISTER_TIMER_CAPTURE_ISR_EMPTY(TIMER_NUM)	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _CAPT_vect));

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
	enum class TimerMode:uint8_t
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

	//TODO document
	enum class TimerInterrupt:uint8_t
	{
		OVERFLOW = board_traits::TimerInterrupt::OVERFLOW,
		OUTPUT_COMPARE_A = board_traits::TimerInterrupt::OUTPUT_COMPARE_A,
		OUTPUT_COMPARE_B = board_traits::TimerInterrupt::OUTPUT_COMPARE_B,
		OUTPUT_COMPARE_C = board_traits::TimerInterrupt::OUTPUT_COMPARE_C,
		INPUT_CAPTURE = board_traits::TimerInterrupt::INPUT_CAPTURE
	};

	constexpr TimerInterrupt operator|(TimerInterrupt i1, TimerInterrupt i2)
	{
		return TimerInterrupt(uint8_t(i1) | uint8_t(i2));
	}

	/**
	 * Defines the "connection" between this timer and specific PWM output pins.
	 */
	enum class TimerOutputMode:uint8_t
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

	//TODO Later add Analog Compare in addition to ICP edges
	/**
	 * Defines the type of input capture we want for a timer.
	 */
	enum class TimerInputCapture:uint8_t
	{
		/** Input capture needed on rising edge of ICP pin. */
		RISING_EDGE,
		/** Input capture needed on falling edge of ICP pin. */
		FALLING_EDGE
	};
	
	/**
	 * Defines a set of calculation methods for the given @p TIMER
	 * The behavior of these methods is specific to each AVR Timer are there can
	 * be many important differences between 2 timers on AVR.
	 * 
	 * @tparam TIMER the timer for which we need calculation methods
	 * @sa board::Timer
	 */
	template<board::Timer TIMER>
	struct Calculator
	{
	private:
		using TRAIT = board_traits::Timer_trait<TIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;

	public:
		/** 
		 * The timer type: either `uint8_t` or `uint16_t`; this is defined and
		 * not changeable for each Timer. Timer counter and max values have 
		 * this type.
		 */
		using TIMER_TYPE = typename TRAIT::TYPE;
		/**
		 * The type (`enum`) of the possible prescaler values for this timer;
		 * this is defined and not changeable for each Timer.
		 */
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		/**
		 * The maximum value you can use for this timer in PWM modes: using this
		 * value will set PWM duty as 100%.
		 */
		static constexpr const TIMER_TYPE PWM_MAX = TRAIT::MAX_PWM;
		
		//TODO doc. calculation method for Normal mode: return best prescaler for 
		// duration per tick (simple)
		static constexpr TIMER_PRESCALER tick_prescaler(uint32_t us_per_tick)
		{
			return best_tick_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us_per_tick);
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
		static constexpr TIMER_PRESCALER CTC_prescaler(uint32_t us)
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
		static constexpr uint32_t CTC_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler));
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
		 * @sa Timer::begin(TIMER_PRESCALER, TIMER_TYPE)
		 */
		static constexpr TIMER_TYPE CTC_counter(TIMER_PRESCALER prescaler, uint32_t us)
		{
			return (TIMER_TYPE) prescaler_quotient(prescaler, us) - 1;
		}

		/**
		 * Verifies that the given prescaler @p p is suitable for this timer in
		 * TimerMode::CTC in ordeer to be able to reach @p us microseconds.
		 * This is normally not needed but can be helpful in a `static_assert` in
		 * your code.
		 * Note this is a `constexpr` method, i.e. it allows compile-time 
		 * computation when provided constant arguments.
		 * You could use this method in `static_assert()` in order to check that
		 * the requested period @p us is OK for this timer.
		 *
		 * @param p the prescaler used for this timer
		 * @param us the number of microseconds to reach with this timer in CTC mode
		 * @retval `true` if @p p and @p us are adequate for @p TIMER
		 * @retval `false` if @p p and @p us are **NOT** adequate for @p TIMER
		 * @sa CTC_prescaler()
		 * @sa CTC_counter()
		 */
		static constexpr bool is_adequate_for_CTC(TIMER_PRESCALER p, uint32_t us)
		{
			return prescaler_is_adequate(prescaler_quotient(p, us));
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
		static constexpr TIMER_PRESCALER FastPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (PWM_MAX + 1UL));
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
		static constexpr uint16_t FastPWM_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / (PWM_MAX + 1UL);
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
		static constexpr TIMER_PRESCALER PhaseCorrectPWM_prescaler(uint16_t pwm_frequency)
		{
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (2UL * PWM_MAX));
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
		static constexpr uint16_t PhaseCorrectPWM_frequency(TIMER_PRESCALER prescaler)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / (2UL * PWM_MAX);
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
		static constexpr TIMER_PRESCALER PulseTimer_prescaler(uint16_t max_pulse_width_us, uint16_t pulse_frequency)
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
		static constexpr TIMER_TYPE PulseTimer_value(TIMER_PRESCALER prescaler, uint16_t period_us)
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
		static constexpr TIMER_PRESCALER PWM_ICR_prescaler(uint16_t pwm_frequency)
		{
			static_assert(TRAIT::ICP_PIN != board::DigitalPin::NONE, "TIMER must have ICR");
			return best_frequency_prescaler(
				PRESCALERS_TRAIT::ALL_PRESCALERS, pwm_frequency * (TRAIT::MAX_PWM + 1UL));
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
		static constexpr uint16_t PWM_ICR_frequency(TIMER_PRESCALER prescaler, uint16_t counter)
		{
			return F_CPU / _BV(uint8_t(prescaler)) / counter;
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
		
		static constexpr bool prescaler_is_adequate_for_tick(TIMER_PRESCALER p, uint32_t us)
		{
			return (prescaler_quotient(p, us) >= 1);
		}
		
		static constexpr TIMER_PRESCALER best_tick_prescaler_in_2(TIMER_PRESCALER p1, TIMER_PRESCALER p2, uint32_t us)
		{
			return (prescaler_is_adequate_for_tick(p2, us)? p2 : p1);
		}

		static constexpr TIMER_PRESCALER best_tick_prescaler(const TIMER_PRESCALER* begin, const TIMER_PRESCALER* end, uint32_t us)
		{
			return (begin + 1 == end ? *begin : 
					best_tick_prescaler_in_2(*begin, best_tick_prescaler(begin + 1 , end, us), us));
		}

		template<size_t N>
		static constexpr TIMER_PRESCALER best_tick_prescaler(const TIMER_PRESCALER(&prescalers)[N], uint32_t us)
		{
			return best_frequency_prescaler(prescalers, prescalers + N, us);
		}
		
	};

	//TODO also need to update pulse_timer templates accordingly!
	//TODO rework, possibly subclass for input capture?
	/**
	 * General API to handle an AVR timer.
	 * Note that many timer usages will require ISR registration with one of
	 * the macros defined in this header file.
	 * 
	 * @tparam TIMER the AVR timer forto use for this Timer
	 * @sa board::Timer
	 */
	template<board::Timer TIMER>
	class Timer
	{
	protected:
		/// @cond notdocumented
		using TRAIT = board_traits::Timer_trait<TIMER>;
		using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;
		/// @endcond
		
	public:
		/**
		 * The type of this timer's counter (either `uint8_t` or `uint16_t`).
		 */
		using TIMER_TYPE = typename TRAIT::TYPE;

		/**
		 * The enum type listing all available precaler values for this timer.
		 */
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;

		/**
		 * The maximum value that can be set to this timer's counter.
		 */
		static constexpr const TIMER_TYPE TIMER_MAX = TRAIT::MAX_COUNTER - 1;

		/**
		 * The maximum value that can be set to this timer's counter in PWM mode.
		 */
		static constexpr const TIMER_TYPE PWM_MAX = TRAIT::MAX_PWM;

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
		 * started
		 * @sa begin()
		 * @sa set_timer_mode()
		 * @sa set_interrupts()
		 * @sa set_prescaler()
		 */
		Timer(TimerMode timer_mode, TIMER_PRESCALER prescaler, TimerInterrupt interrupts = TimerInterrupt(0))
		:	_tccra{timer_mode_TCCRA(timer_mode)}, 
			_tccrb{timer_mode_TCCRB(timer_mode) | TRAIT::TCCRB_prescaler(prescaler)},
			_timsk{uint8_t(interrupts)}
		{
			//TODO assert interrupts
			static_assert(TIMSK_MASK_IS_SUPPORTED(interrupts), "TIMER does not support requested interrupts");
		}

		//TODO doc
		inline void set_interrupts(const TimerInterrupt interrupts) INLINE
		{
			//TODO static_assert
			static_assert(TIMSK_MASK_IS_SUPPORTED(interrupts), "TIMER does not support requested interrupts");
			_timsk = uint8_t(interrupts);
			// Check if timer is currently running
			if (TRAIT::TCCRB)
				TRAIT::TIMSK = uint8_t(interrupts);
		}

		//TODO doc
		inline void set_input_capture(TimerInputCapture input_capture)
		{
			utils::set_mask(_tccrb, TRAIT::ICES_TCCRB, input_capture_TCCRB(input_capture));
			// Check if timer is currently running
			if (TRAIT::TCCRB)
				TRAIT::TCCRB = _tccrb;
		}

		/**
		 * Change timer mode.
		 * @param timer_mode the mode to reset this timer to
		 */
		inline void set_timer_mode(TimerMode timer_mode)
		{
			utils::set_mask(_tccra, TRAIT::MODE_MASK_TCCRA, timer_mode_TCCRA(timer_mode));
			utils::set_mask(_tccrb, TRAIT::MODE_MASK_TCCRB, timer_mode_TCCRB(timer_mode));
			// Check if timer is currently running
			if (TRAIT::TCCRB)
			{
				TRAIT::TCCRA = _tccra;
				TRAIT::TCCRB = _tccrb;
			}
		}

		//TODO doc
		inline void set_prescaler(TIMER_PRESCALER prescaler)
		{
			utils::set_mask(_tccrb, TRAIT::CS_MASK_TCCRB, TRAIT::TCCRB_prescaler(prescaler));
			// Check if timer is currently running
			if (TRAIT::TCCRB)
				TRAIT::TCCRB = _tccrb;
		}

		/**
		 * Start this timer in the currently selected mode, with the provided
		 * @p prescaler value and @p max value. 
		 * This method enables timer interrupts (Compare Match),hence ISR
		 * registration is required.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_begin()` instead.
		 * @param max the maximum value to use for this timer's counter
		 * @sa end()
		 * @sa _begin(TIMER_TYPE)
		 */
		inline void begin(TIMER_TYPE max = 0)
		{
			synchronized _begin(max);
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
		 * @sa _end()
		 * @sa begin(TIMER_TYPE)
		 */
		inline void _begin(TIMER_TYPE max = 0)
		{
			TRAIT::TCCRA = _tccra;
			TRAIT::TCCRB = _tccrb;
			// Set timer counter compare match
			TRAIT::OCRA = max;
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = _timsk;
		}

		/**
		 * Reset current counter to 0.
		 * This method is synchronized if needed (i.e. if this timer is 16 bits).
		 * If you do not need synchronization, then you should better use
		 * `_reset()` instead.
		 * @sa _reset()
		 */
		inline void reset()
		{
			if (sizeof(TIMER_TYPE) > 1)
				synchronized _reset();
			else
				_reset();
		}
		
		/**
		 * Reset current counter to 0.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `reset()` instead.
		 * @sa reset()
		 */
		inline void _reset()
		{
			TRAIT::TCNT = 0;
		}
		
		/**
		 * Temporarily suspend this timer: the timer does not generate any
		 * interrupt any longer.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_suspend()` instead.
		 * @sa resume()
		 * @sa _suspend()
		 */
		inline void suspend()
		{
			synchronized _suspend();
		}

		/**
		 * Temporarily suspend this timer: the timer does not generate any
		 * interrupt any longer.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `suspend()` instead.
		 * @sa _resume()
		 * @sa suspend()
		 */
		inline void _suspend()
		{
			// Clear timer interrupt mode
			TRAIT::TIMSK = 0;
		}

		/**
		 * Resume this timer if it was previously suspended: the timer's counter
		 * is reset and the timer starts generating interrupts again.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_resume()` instead.
		 * @sa suspend()
		 * @sa _resume()
		 */
		inline void resume()
		{
			synchronized _resume();
		}

		/**
		 * Resume this timer if it was previously suspended: the timer's counter
		 * is reset and the timer starts generating interrupts again.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `resume()` instead.
		 * @sa _suspend()
		 * @sa resume()
		 */
		inline void _resume()
		{
			// Reset timer counter
			TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = _timsk;
		}

		/**
		 * Check if this timer is currently suspended, i.e. it does not generate
		 * any interrupts.
		 * @retval true the timer is currently suspended
		 * @retval false the timer is currently active
		 */
		inline bool is_suspended()
		{
			return TRAIT::TIMSK == 0;
		}

		/**
		 * Completely stop this timer: timer interrupts are disabled and counter 
		 * is stopped.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_end()` instead.
		 * @sa begin()
		 * @sa _end()
		 */
		inline void end()
		{
			synchronized _end();
		}

		/**
		 * Completely stop this timer: timer interrupts are disabled and counter 
		 * is stopped.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `end()` instead.
		 * @sa _begin()
		 * @sa end()
		 */
		inline void _end()
		{
			// Stop timer
			TRAIT::TCCRB = 0;
			// Clear timer interrupt mode (set interrupt on OCRnA compare match)
			TRAIT::TIMSK = 0;
		}

		/**
		 * Change the output mode for this timer; this enables connection between 
		 * this timer to one of its associated digital output pins.
		 * @tparam COM the index of the output pin for this timer; your program
		 * will not compile if you use an incorrect value.
		 * @param mode the new output mode for this timer and this pin
		 */
		template<uint8_t COM>
		inline void set_output_mode(TimerOutputMode mode)
		{
			static_assert(COM < TRAIT::COM_COUNT, "COM must exist for TIMER");
			using COM_TRAIT = board_traits::Timer_COM_trait<TIMER, COM>;
			utils::set_mask(_tccra, COM_TRAIT::COM_MASK, convert_COM<COM>(mode));
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
		/// @cond notdocumented
		Timer(uint8_t tccra, uint8_t tccrb, uint8_t timsk = 0)
			:_tccra{tccra}, _tccrb{tccrb}, _timsk{timsk} {}

		template<uint8_t COM>
		static constexpr uint8_t convert_COM(TimerOutputMode output_mode)
		{
			using COM_TRAIT = board_traits::Timer_COM_trait<TIMER, COM>;
			return (output_mode == TimerOutputMode::TOGGLE ? COM_TRAIT::COM_TOGGLE :
					output_mode == TimerOutputMode::INVERTING ? COM_TRAIT::COM_SET :
					output_mode == TimerOutputMode::NON_INVERTING ? COM_TRAIT::COM_CLEAR :
					COM_TRAIT::COM_NORMAL);
		}

		static constexpr bool TIMSK_MASK_IS_SUPPORTED(TimerInterrupt interrupt)
		{
			return (TRAIT::TIMSK_MASK(0xFF) & TRAIT::TIMSK_MASK(uint8_t(interrupt))) 
					== TRAIT::TIMSK_MASK(uint8_t(interrupt));
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
		static constexpr uint8_t input_capture_TCCRB(TimerInputCapture input_capture)
		{
			return (input_capture == TimerInputCapture::RISING_EDGE ? TRAIT::ICES_TCCRB : 0);
		}
		
		uint8_t _tccra;
		uint8_t _tccrb;
		uint8_t _timsk;
		/// @endcond
	};
}

#endif /* TIMER_HH */
/// @endcond
