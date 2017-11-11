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
 * Real-time Timer API.
 */
#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>
#include "interrupts.h"
#include "timer.h"
#include "time.h"
#include "events.h"

/**
 * Register the necessary ISR (interrupt Service Routine) for a timer::RTT to work
 * properly.
 * This will not register any user callback though; if you need to register a
 * method or function to be called back every time one millsiecond has elapsed,
 * you need to use `REGISTER_RTT_ISR_METHOD` or `REGISTER_RTT_ISR_FUNCTION`
 * instead.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * 
 * @sa REGISTER_RTT_ISR_METHOD
 * @sa REGISTER_RTT_ISR_FUNCTION
 */
#define REGISTER_RTT_ISR(TIMER_NUM)                                                                  \
	REGISTER_TIMER_COMPARE_ISR_METHOD(TIMER_NUM, CAT(timer::RTT < board::Timer::TIMER, TIMER_NUM) >, \
									  CAT(&timer::RTT < board::Timer::TIMER, TIMER_NUM) > ::on_timer)

/**
 * Register the necessary ISR (interrupt Service Routine) for a timer::RTT to work
 * properly, along with a callback method that will be notified every millisecond.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function) that
 * takes an `uint32_t` argument that will receive the total number of milliseconds
 * elapsed since the RTT has started.
 * 
 * @sa REGISTER_RTT_ISR_FUNCTION
 * @sa REGISTER_RTT_ISR
 */
#define REGISTER_RTT_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)                                   \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                    \
	{                                                                                           \
		using RTT_HANDLER = CAT(timer::RTT < board::Timer::TIMER, TIMER_NUM) > ;                \
		using RTT_HOLDER = HANDLER_HOLDER_(RTT_HANDLER);                                        \
		using RTT_HANDLE = CALLBACK_HANDLER_HOLDER_(RTT_HANDLER, &RTT_HANDLER::on_timer, void); \
		RTT_HANDLE::handle();                                                                   \
		CALL_HANDLER_(HANDLER, CALLBACK, uint32_t)(RTT_HOLDER::handler()->millis());            \
	}

/**
 * Register the necessary ISR (interrupt Service Routine) for a timer::RTT to work
 * properly, along with a callback function that will be notified every millisecond.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered; this function must accept an `uint32_t` argument that will receive 
 * the total number of milliseconds elapsed since the RTT has started.
 * 
 * @sa REGISTER_RTT_ISR_METHOD
 * @sa REGISTER_RTT_ISR
 */
#define REGISTER_RTT_ISR_FUNCTION(TIMER_NUM, CALLBACK)                                    \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                              \
	{                                                                                     \
		using RTT_HANDLER = CAT(timer::RTT < board::Timer::TIMER, TIMER_NUM) > ;          \
		using RTT_HOLDER = HANDLER_HOLDER_(RTT_HANDLER);                                  \
		using RTT_HANDLE = CALLBACK_HANDLER_HOLDER_(RTT_HANDLER, &RTT_HANDLER::on_timer); \
		RTT_HANDLE::handle();                                                             \
		CALLBACK(RTT_HOLDER::handler()->millis());                                        \
	}

namespace timer
{
	//TODO rename template args per new conventions
	/**
	 * API to handle a real-time timer.
	 * A real-time timer keeps track of time with micro-second precision.
	 * In order to perform properly, an appropriate ISR must be registered for it.
	 * 
	 * A real-time timer can be used to:
	 * - capture the duration of some event with good accuracy
	 * - implement timeouts in programs waiting for an event to occur
	 * - delay program execution for some us or ms
	 * - generate periodic events
	 * 
	 * @tparam TIMER the AVR timer used by this RTT
	 * @sa board::Timer
	 * @sa REGISTER_RTT_ISR()
	 */
	template<board::Timer TIMER> class RTT : private Timer<TIMER>
	{
	private:
		using TRAIT = typename Timer<TIMER>::TRAIT;
		using TIMER_TYPE = typename Timer<TIMER>::TIMER_TYPE;
		using TIMER_PRESCALER = typename Timer<TIMER>::TIMER_PRESCALER;

	public:
		/**
		 * Construct a new real-time timer handler and initializes its current
		 * time to 0ms.
		 * Note that this constructor does **not** start the timer.
		 * @sa begin()
		 * @sa millis()
		 */
		RTT() : Timer<TIMER>{TimerMode::CTC, MILLI_PRESCALER}, _millis{}
		{
		}

		/**
		 * Register this RTT with the matching ISR that should have been
		 * registered with REGISTER_RTT_ISR().
		 * Calling this method is mandatory for this timer to work.
		 * @sa REGISTER_RTT_ISR()
		 */
		void register_rtt_handler()
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Elapsed time, in milliseconds, since this timer has started.
		 * If `millis(uint32_t)` is called, this sets a new time reference point
		 * to count elapsed time from.
		 * If you want to get more precision about the elpased time, you can get
		 * the number of microsecond elapsed, **in addition to `millis()`**, by
		 * calling `micros()`.
		 * @return elapsed time in ms
		 * @sa millis(uint32_t)
		 * @sa micros()
		 */
		inline uint32_t millis() const
		{
			synchronized return _millis;
		}

		/**
		 * Delay program execution for the given amount of milliseconds.
		 * Contrarily to `time::delay_ms()`, this method does not perform a busy
		 * loop, but it calls `time::yield()` which will put the MCU in sleep
		 * mode but will be awakened every ms (by a timer interrupt) and check if
		 * required delay has elapsed already.
		 * 
		 * @param ms the number of milliseconds to hold program execution
		 * @sa time::yield()
		 */
		void delay(uint32_t ms) const
		{
			uint32_t end = millis() + ms + 1;
			while (millis() < end) time::yield();
		}

		/**
		 * Compute the microseconds part (from `0` to `999`) of the time that has
		 * elapsed, since this timer has started. The milliseconds part is
		 * provided by `millis()`.
		 * In general, you will not call this method unless you are sure the
		 * elapsed time is strictly less than 1ms.
		 * If you need the elapsed time with microsecond precision, then you
		 * should call `time()` which returns an `time::RTTTime` structure that
		 * contains both milliseconds and microseconds.
		 * @return the us part of elapsed time
		 * @sa millis()
		 * @sa time()
		 */
		inline uint16_t micros() const
		{
			synchronized return compute_micros();
		}

		/**
		 * Elapsed time, in milliseconds and microseconds, since this timer has
		 * started.
		 * If you do not need microsecond precision, you should instead use 
		 * `millis()`.
		 * @return elapsed time in ms and us
		 * @sa millis()
		 * @sa micros()
		 */
		time::RTTTime time() const
		{
			synchronized return time::RTTTime(_millis, compute_micros());
		}

		/**
		 * Reset the current milliseconds count of this RTT to the given value.
		 * Evey elapsed millisecond will then be added to this new value.
		 * @param ms the new millisecond start
		 */
		inline void millis(uint32_t ms)
		{
			synchronized
			{
				_millis = ms;
				// Reset timer counter
				TRAIT::TCNT = 0;
			}
		}

		/**
		 * Start this real-time timer, hence elapsed time starts getting counted
		 * from then.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_begin()` instead.
		 * @sa end()
		 * @sa _begin()
		 */
		inline void begin()
		{
			synchronized _begin();
		}

		/**
		 * Start this real-time timer, hence elapsed time starts getting counted
		 * from then.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `begin()` instead.
		 * @sa _end()
		 * @sa begin()
		 */
		inline void _begin()
		{
			_millis = 0;
			Timer<TIMER>::_begin(MILLI_COUNTER);
		}

		/**
		 * Stop this real-time timer, hence time gets not counted anymore.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `_end()` instead.
		 * @sa begin()
		 * @sa _end()
		 */
		inline void end()
		{
			Timer<TIMER>::end();
		}

		/**
		 * Stop this real-time timer, hence time gets not counted anymore.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `end()` instead.
		 * @sa _begin()
		 * @sa end()
		 */
		inline void _end()
		{
			Timer<TIMER>::_end();
		}

		/// @cond notdocumented
		void on_timer()
		{
			++_millis;
		}
		/// @endcond

		//TODO DOC
		inline Timer<TIMER>& timer()
		{
			return *this;
		}

	protected:
		/// @cond notdocumented
		volatile uint32_t _millis;
		/// @endcond

	private:
		using CALC = Calculator<TIMER>;
		static constexpr const uint32_t ONE_MILLI = 1000UL;
		static constexpr const TIMER_PRESCALER MILLI_PRESCALER = CALC::CTC_prescaler(ONE_MILLI);
		static constexpr const TIMER_TYPE MILLI_COUNTER = CALC::CTC_counter(MILLI_PRESCALER, ONE_MILLI);

		inline uint16_t compute_micros() const
		{
			return uint16_t(ONE_MILLI * ((volatile TIMER_TYPE&) TRAIT::TCNT) /
							(1 + (volatile TIMER_TYPE&) TRAIT::OCRA));
		}
	};

	template<uint32_t PERIOD_MS = 1024> class RTTEventCallback
	{
		static_assert((PERIOD_MS & (PERIOD_MS - 1)) == 0, "PERIOD_MS must be a power of 2");

	public:
		RTTEventCallback(containers::Queue<events::Event>& event_queue) : _event_queue(event_queue)
		{
		}

		void on_rtt_change(uint32_t millis)
		{
			if ((millis & (PERIOD_MS - 1)) == 0) _event_queue._push(events::Event{events::Type::RTT_TIMER});
		}

		containers::Queue<events::Event>& _event_queue;
	};
}

#endif /* RTT_HH */
/// @endcond
