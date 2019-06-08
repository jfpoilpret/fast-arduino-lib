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
 * Watchdog API.
 */
#ifndef WATCHDOG_HH
#define WATCHDOG_HH

#include "boards/board.h"
#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include "interrupts.h"
#include "events.h"

/**
 * Register the necessary ISR (Interrupt Service Routine) for a watchdog::Watchdog
 * to work properly.
 * @param EVENT the type of Event<T> to be generated by the watchdog
 * @sa watchdog::Watchdog
 */
#define REGISTER_WATCHDOG_CLOCK_ISR(EVENT)              \
	ISR(WDT_vect)                                       \
	{                                                   \
		watchdog::isr_handler::watchdog_clock<EVENT>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a watchdog::WatchdogRTT
 * to work properly.
 * @sa watchdog::WatchdogRTT
 */
#define REGISTER_WATCHDOG_RTT_ISR()            \
	ISR(WDT_vect)                              \
	{                                          \
		watchdog::isr_handler::watchdog_rtt(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) with a callback method
 * that will be called every time a watchdog timeout occurs, according to how
 * watchdog::WatchdogSignal was started.
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_WATCHDOG_ISR_METHOD(HANDLER, CALLBACK)              \
	ISR(WDT_vect)                                                    \
	{                                                                \
		watchdog::isr_handler::watchdog_method<HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) with a callback function
 * that will be called every time a watchdog timeout occurs, according to how
 * watchdog::WatchdogSignal was started.
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_WATCHDOG_ISR_FUNCTION(CALLBACK)               \
	ISR(WDT_vect)                                              \
	{                                                          \
		watchdog::isr_handler::watchdog_function<CALLBACK>() : \
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for a watchdog::WatchdogSignal.
 * This may be needed when using watchdog just to awaken a sleeping MCU, but without
 * any necessary immediate callback.
 */
#define REGISTER_WATCHDOG_ISR_EMPTY() EMPTY_INTERRUPT(WDT_vect)

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_WATCHDOG_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_WATCHDOG_ISR_HANDLERS_FRIEND \
	friend struct watchdog::isr_handler;  \
	friend void ::WDT_vect();

/**
 * Defines the simple API for Watchdog timer management.
 */
namespace watchdog
{
	/**
	 * Defines the watchdog timeout period; watchdog interrupts will be 
	 * triggered at the selected period. The MCU will be awakened at this 
	 * period too.
	 * Note that watchdog timeout period is not very accurate, you would
	 * normally not use for real-time counting.
	 */
	enum class TimeOut : uint8_t
	{
		/** Watchdog timeout 16 ms. */
		TO_16ms = 0,
		/** Watchdog timeout 32 ms. */
		TO_32ms,
		/** Watchdog timeout 64 ms. */
		TO_64ms,
		/** Watchdog timeout 125 ms. */
		TO_125ms,
		/** Watchdog timeout 250 ms. */
		TO_250ms,
		/** Watchdog timeout 500 ms. */
		TO_500ms,
		/** Watchdog timeout 1 second. */
		TO_1s,
		/** Watchdog timeout 2 seconds. */
		TO_2s,
		/** Watchdog timeout 4 seconds. */
		TO_4s,
		/** Watchdog timeout 8 seconds. */
		TO_8s
	};

	/**
	 * Simple API to handle watchdog signals. In this mode, the AVR watchdog
	 * timer is used to wake up the MCU at a specific timer period, in order to
	 * leave a low-power sleep mode, to e.g. perform periodic tasks.
	 * With this API, you do not need to register an ISR callback if you do not
	 * need one; in this situation you need to register an empty ISR with
	 * `REGISTER_WATCHDOG_ISR_EMPTY()` macro.
	 * If you want to register a callback on every watchdog timer tick, then you
	 * can use either `REGISTER_WATCHDOG_ISR_FUNCTION` or 
	 * `REGISTER_WATCHDOG_ISR_METHOD` macros.
	 */
	class WatchdogSignal
	{
	public:
		/**
		 * Start the watchdog timer with the given @p timeout period.
		 * From now on, watchdog interrupts get generated at @p timeout period,
		 * and the MCU gets awakened at this period too, if sleeping.
		 * @param timeout the watchdog timeout period
		 */
		void begin(TimeOut timeout = TimeOut::TO_16ms)
		{
			uint8_t config = BV8(WDIE) | (uint8_t(timeout) & 0x07) | (uint8_t(timeout) & 0x08 ? BV8(WDP3) : 0);
			synchronized begin_with_config(config);
		}

		/**
		 * Stop this watchdog timer. No more watchdog interrupts get triggered.
		 */
		void end()
		{
			synchronized
			{
				WDTCSR_ = BV8(WDCE) | BV8(WDE);
				WDTCSR_ = 0;
			}
		}

	protected:
		/// @cond notdocumented
		inline void begin_with_config(uint8_t config) INLINE
		{
			__asm__ __volatile__("wdr");
			MCUSR_ |= 1 << WDRF;
			WDTCSR_ = BV8(WDCE) | BV8(WDE);
			WDTCSR_ = config;
		}
		/// @endcond

	private:
		using REG8 = board_traits::REG8;
		static constexpr const REG8 MCUSR_{MCUSR};
		static constexpr const REG8 WDTCSR_{WDTCSR};
	};

	/**
	 * Simple API to use watchdog timer as a real-time clock.
	 * For this to work correctly, you need to register the proper ISR through
	 * `REGISTER_WATCHDOG_RTT_ISR()` macro first.
	 */
	class WatchdogRTT : public WatchdogSignal
	{
	public:
		/**
		 * Construct a new watchdog-based clock that will count elapsed
		 * milliseconds since it was started with `begin()`.
		 * @sa REGISTER_WATCHDOG_RTT_ISR()
		 */
		WatchdogRTT() : millis_{0}, millis_per_tick_{0}
		{
			interrupt::register_handler(*this);
		}

		/// @cond notdocumented
		WatchdogRTT(const WatchdogRTT&) = delete;
		/// @endcond

		/**
		 * Start the watchdog clock with the given @p timeout period.
		 * From now on, watchdog interrupts get generated at @p timeout period,
		 * and the MCU gets awakened at this period too, if sleeping.
		 * @param timeout the watchdog timeout period
		 */
		void begin(TimeOut timeout = TimeOut::TO_16ms)
		{
			uint16_t ms_per_tick = 1 << (uint8_t(timeout) + 4);
			uint8_t config = BV8(WDIE) | (uint8_t(timeout) & 0x07) | (uint8_t(timeout) & 0x08 ? BV8(WDP3) : 0);

			synchronized
			{
				begin_with_config(config);
				millis_per_tick_ = ms_per_tick;
				millis_ = 0;
			}
		}

		/**
		 * Get the number of milliseconds that elapsed since `begin()` was called.
		 * Note: the precision is limited by the @p timeout value used with 
		 * `begin()`.
		 * @return number of milliseconds elpased since watchdog was started
		 */
		inline uint32_t millis() const
		{
			synchronized return millis_;
		}

		inline void reset()
		{
			synchronized millis_ = 0;
		}

		/**
		 * Delay program execution for the given amount of milliseconds.
		 * Contrarily to `time::delay_ms()`, this method does not perform a busy
		 * loop, but it calls `time::yield()` which will put the MCU in sleep
		 * mode but will be awakened by the watchdog interrupt and check if
		 * required delay has elapsed already.
		 * Note that the delay precision depends on the actual timeout value
		 * passed when `begin()` has been called.
		 * 
		 * @param ms the number of milliseconds to hold program execution
		 * @sa time::yield()
		 */
		void delay(uint32_t ms)
		{
			uint32_t limit = millis() + ms;
			while (millis() < limit)
			{
				time::yield();
			}
		}

	protected:
		/// @cond notdocumented
		// This constructor is used by subclass to avoid calling register_handler()
		explicit WatchdogRTT(bool dummy UNUSED) : millis_{0}, millis_per_tick_{0} {}

		void on_tick()
		{
			millis_ += millis_per_tick_;
		}
		/// @endcond

	private:
		volatile uint32_t millis_;
		uint16_t millis_per_tick_;

		friend struct isr_handler;
	};

	/**
	 * Simple API to use watchdog timer as a clock for events generation.
	 * For this to work correctly, you need to register the proper ISR through
	 * `REGISTER_WATCHDOG_CLOCK_ISR()` macro first.
	 * @tparam EVENT the `events::Event<T>` generated
	 */
	template<typename EVENT> class Watchdog : public WatchdogRTT
	{
		static_assert(events::Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");

	public:
		/**
		 * Construct a new watchdog-based clock that will, for each watchdog 
		 * timeout, add an event to the given @p event_queue for further 
		 * processing.
		 * This clock also counts elapsed milliseconds since it was started with
		 * `begin()`.
		 * @param event_queue the queue to which `Event`s will be pushed on each 
		 * watchdog tick
		 * @sa REGISTER_WATCHDOG_CLOCK_ISR()
		 */
		explicit Watchdog(containers::Queue<EVENT>& event_queue) : WatchdogRTT{true}, event_queue_{event_queue}
		{
			interrupt::register_handler(*this);
		}

		/// @cond notdocumented
		Watchdog(const Watchdog&) = delete;
		/// @endcond

	private:
		void on_tick()
		{
			WatchdogRTT::on_tick();
			event_queue_.push_(EVENT{events::Type::WDT_TIMER});
		}

		containers::Queue<EVENT>& event_queue_;

		friend struct isr_handler;
	};

	struct isr_handler
	{
		template<typename EVENT> static void watchdog_clock()
		{
			interrupt::HandlerHolder<watchdog::Watchdog<EVENT>>::handler()->on_tick();
		}

		static void watchdog_rtt()
		{
			interrupt::HandlerHolder<watchdog::WatchdogRTT>::handler()->on_tick();
		}

		template<typename HANDLER, void (HANDLER::*CALLBACK)()> static void watchdog_method()
		{
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();
		}

		template<void (*CALLBACK)()> static void watchdog_function()
		{
			interrupt::CallbackHandler<void (*)(), CALLBACK>::call();
		}
	};
}

#endif /* WATCHDOG_HH */
/// @endcond
