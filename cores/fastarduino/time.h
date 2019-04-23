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
 * Simple time utilities.
 */
#ifndef TIME_HH
#define TIME_HH

#include <stdint.h>
#include "boards/io.h"
#include <util/delay_basic.h>
#include "utilities.h"

/**
 * Defines simple API to handle time and delays.
 */
namespace time
{
	/**
	 * Structure used to hold a time value with microsecond precision.
	 * Note that this value is not absolute but relative to some time base, not
	 * defined here, but depending on the API producing or consuming an `RTTTime`.
	 * 
	 * @param millis number of milliseconds elapsed since some predefined time base
	 * @param micros number of microseconds, added to @p millis, elapsed since some
	 * predefined time base reference, between `0` and `999`; setting a greater 
	 * value is not supported and will have an undefined behavior of the API
	 * consuming such an `RTTTime` instance.
	 */
	struct RTTTime
	{
		/**
		 * Construct a new `RTTTime` value.
		 * @param micros number of microseconds (can be > 1000us)
		 */
		RTTTime(uint32_t micros = 0UL) : millis(micros / 1000UL), micros(micros % 1000UL) {}

		/**
		 * Construct a new `RTTTime` value.
		 * @param millis number of milliseconds
		 * @param micros number of microseconds (0..999)
		 */
		RTTTime(uint32_t millis, uint16_t micros) : millis(millis), micros(micros) {}

		/**
		 * Construct a copy of @p that.
		 * @param that
		 */
		RTTTime(const RTTTime& that) : millis{that.millis}, micros{that.micros} {}

		/**
		 * Assign this `RTTTime` instance from @p that.
		 * @param that the source real time value
		 * @return this instance
		 */
		RTTTime& operator=(const RTTTime& that)
		{
			millis = that.millis;
			micros = that.micros;
			return *this;
		}

		/**
		 * Add @p microseconds to this RTTTime instance.
		 * @param microseconds the number of us to add to this RTTTime
		 * @return this instance
		 */
		RTTTime& operator+=(uint32_t microseconds)
		{
			uint32_t extra_millis = microseconds / 1000UL;
			uint16_t extra_micros = microseconds % 1000UL;
			millis += extra_millis;
			micros += extra_micros;
			if (micros >= 1000UL)
			{
				++millis;
				micros -= 1000UL;
			}
			return *this;
		}

		/**
		 * Remove @p microseconds from this RTTTime instance.
		 * @param microseconds the number of us to subtract from this RTTTime;
		 * results are undetermined if @p microseconds is larger than `total_micros()`.
		 * @return this instance
		 */
		RTTTime& operator-=(uint32_t microseconds)
		{
			//TODO code can probably be optimized
			return *this = RTTTime{total_micros() - microseconds};
		}

		/**
		 * Return current elapsed time in microseconds only.
		 */
		uint32_t total_micros() const
		{
			return millis * 1000UL + micros;
		}

		/** Number of elapsed milliseconds. */
		uint32_t millis;
		/** Number of elapsed microseconds (0..999). */
		uint16_t micros;
	};

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a > b
	 * @retval false if a <= b
	 */
	inline bool operator>(const RTTTime& a, const RTTTime& b)
	{
		if (a.millis > b.millis) return true;
		if (a.millis < b.millis) return false;
		return a.micros > b.micros;
	}

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a >= b
	 * @retval false if a < b
	 */
	inline bool operator>=(const RTTTime& a, const RTTTime& b)
	{
		if (a.millis > b.millis) return true;
		if (a.millis < b.millis) return false;
		return a.micros >= b.micros;
	}

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a < b
	 * @retval false if a >= b
	 */
	inline bool operator<(const RTTTime& a, const RTTTime& b)
	{
		if (a.millis < b.millis) return true;
		if (a.millis > b.millis) return false;
		return a.micros < b.micros;
	}

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a <= b
	 * @retval false if a > b
	 */
	inline bool operator<=(const RTTTime& a, const RTTTime& b)
	{
		if (a.millis < b.millis) return true;
		if (a.millis > b.millis) return false;
		return a.micros <= b.micros;
	}

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a == b
	 * @retval false if a != b
	 */
	inline bool operator==(const RTTTime& a, const RTTTime& b)
	{
		return a.millis == b.millis && a.micros == b.micros;
	}

	/**
	 * Compare 2 RTTTime instances.
	 * @param a the first RTTTime instance in comparison
	 * @param b the second RTTTime instance in comparison
	 * @retval true if a != b
	 * @retval false if a == b
	 */
	inline bool operator!=(const RTTTime& a, const RTTTime& b)
	{
		return a.millis != b.millis || a.micros != b.micros;
	}

	/**
	 * Add 2 RTTTime instances.
	 * @param a the first RTTTime instance in addition
	 * @param b the second RTTTime instance in addition
	 * @return a + b
	 */
	inline RTTTime operator+(const RTTTime& a, const RTTTime& b)
	{
		uint32_t millis = a.millis + b.millis;
		uint16_t micros = a.micros + b.micros;
		if (micros >= 1000UL)
		{
			++millis;
			micros -= 1000UL;
		}
		return RTTTime{millis, micros};
	}

	/**
	 * Subtract 2 RTTTime instances.
	 * @param a the first RTTTime instance in subtraction
	 * @param b the RTTTime instance to be subtracted from @p a
	 * @return a - b if a > b, 0 if a <= b
	 */
	inline RTTTime operator-(const RTTTime& a, const RTTTime& b)
	{
		if (a <= b) return RTTTime{0, 0};
		uint32_t millis = a.millis - b.millis;
		if (a.micros > b.micros) return RTTTime{millis, a.micros - b.micros};
		return RTTTime{millis + 1, 1000 + a.micros - b.micros};
	}

	/**
	 * Utility method used by many FastArduino API in order to "yield" some 
	 * processor time; concretely it just calls `power::Power::sleep()` which
	 * will put the MCU into a default sleep mode.
	 * @sa power::Power::sleep()
	 */
	void yield();

	/**
	 * Compute the time delta from @p time1 and @p time2.
	 * The method expects @p time1 to occur *before* @p time2; otherwise, behavior 
	 * is undefined.
	 * @param time1 the starting time
	 * @param time2 the ending time
	 * @return  the difference between @p time1 and @p time2
	 */
	RTTTime delta(const RTTTime& time1, const RTTTime& time2);

	/**
	 * Compute the time elapsed, in milliseconds, since @p start_ms.
	 * To calculate the elapsed time, this method uses the function pointed to
	 * by `time::millis()`.
	 * @param start_ms
	 * @return number of milliseconds elapsed between @p start_ms and now (as 
	 * returned by `time::millis()`)
	 * @sa time::millis
	 */
	uint32_t since(uint32_t start_ms);

	/**
	 * Function pointer type used for `time::delay` global variable.
	 */
	using DELAY_PTR = void (*)(uint32_t ms);

	/**
	 * Delay program execution for the given amount of milliseconds.
	 * `delay` is not actually a function but a function pointer; this allows 
	 * FastArduino to set the best implementation based on currently used
	 * features.
	 * By default, `delay` just performs a busy loop for the right amount of MCU 
	 * cycles.
	 * Note that if you want to be sure reduce code size to the minimum and you
	 * are OK with always using busy loops for delays, then you should use
	 * `time::delay_ms()` instead.
	 * @param ms number of milliseconds to wait for
	 * @sa time::delay_ms()
	 */
	extern DELAY_PTR delay;

	/**
	 * Function pointer type used for `time::millis` global variable.
	 */
	using MILLIS_PTR = uint32_t (*)(void);

	/**
	 * Count number of milliseconds elapsed since some time base reference 
	 * (generally since MCU startup).
	 * `millis` is not actually a function but a function pointer; this allows 
	 * FastArduino to set the best implementation based on currently used
	 * features.
	 * By default, `millis` is not pointing to any concrete implementation, hence
	 * you should not call it if it has not been initialized yet.
	 * You can initialize it with `timer::set_clock()` provided with the instance of
	 * a class implementing a minimal "clock" API.
	 * The following classes implement this minimal clock API:
	 * - `timer::RTT`
	 * - `watchdog::Watchdog`
	 * 
	 * @return number of milliseconds elapsed since some given time base base 
	 * reference
	 * @sa time::set_clock()
	 */
	extern MILLIS_PTR millis;

	/**
	 * Delay program execution for the given amount of microseconds.
	 * Delay is achieved by a busy loop for the right amount of MCU cycles.
	 * 
	 * @param us the number of microseconds to hold program execution
	 */
	inline void delay_us(uint16_t us) INLINE;
	inline void delay_us(uint16_t us)
	{
		_delay_loop_2(us * (F_CPU / 1000000UL) / 4UL);
	}

	/**
	 * Delay program execution for the given amount of milliseconds.
	 * Delay is achieved by a busy loop for the right amount of MCU cycles.
	 * 
	 * @param ms the number of milliseconds to hold program execution
	 */
	inline void delay_ms(uint16_t ms) INLINE;
	inline void delay_ms(uint16_t ms)
	{
		while (ms--) delay_us(1000);
	}

	/**
	 * Delay program execution for the given amount of milliseconds.
	 * This is the default implementation for `delay` function pointer global
	 * variable.
	 * 
	 * @param ms the number of milliseconds to hold program execution
	 * @sa time::delay
	 */
	void default_delay(uint32_t ms);

	/// @cond notdocumented
	template<typename CLOCK> class ClockDelegate
	{
		using TYPE = ClockDelegate<CLOCK>;

	public:
		static void set_clock(const CLOCK& clock)
		{
			clock_ = &clock;
			time::delay = TYPE::delay;
			time::millis = TYPE::millis;
		}

		static void delay(uint32_t ms)
		{
			clock_->delay(ms);
		}

		static uint32_t millis()
		{
			return clock_->millis();
		}

	private:
		static const CLOCK* clock_;
	};

	template<typename CLOCK> const CLOCK* ClockDelegate<CLOCK>::clock_ = 0;
	// @endcond

	/**
	 * Utility method to transform `millis()` and `delay()` methods of a @p clock
	 * instance of any @p CLOCK class into static method equivalents, and assign
	 * the resulting static methods to `time::millis` and `time::delay` global 
	 * function pointers.
	 * This will work with any class as long as it defines the following methods:
	 * @code
	 * void delay(uint32_t ms);
	 * uint32_t millis();
	 * @endcode
	 * 
	 * @tparam CLOCK the class that defines the needed methods `millis()` and
	 * `delay()`
	 * @param clock the instance which methods will be called
	 */
	template<typename CLOCK> void set_clock(const CLOCK& clock)
	{
		time::ClockDelegate<CLOCK>::set_clock(clock);
	}

	/**
	 * Set a new `time::delay` function for the duration of the current scope;
	 * when the scope is left, the previous `time::delay` function is restored.
	 * Usage example:
	 * @code
	 * void my_delay(uint32_t ms)
	 * {
	 *     ...
	 * }
	 * void f()
	 * {
	 *     // Set new time::delay function
	 *     auto_delay d{my_delay};
	 *     // call API that rely on time::delay
	 *     ...
	 *     // Restore previous time::delay function
	 * }
	 * @endcode
	 */
	class auto_delay
	{
	public:
		/**
		 * Set new `time::delay` to @p new_delay after storing current function 
		 * for later restore.
		 * @param new_delay new function pointer to replace `time::delay`
		 */
		auto_delay(DELAY_PTR new_delay) INLINE : old_delay_{delay}
		{
			delay = new_delay;
		}
		/// @cond notdocumented
		~auto_delay() INLINE
		{
			delay = old_delay_;
		}
		/// @endcond
	private:
		const DELAY_PTR old_delay_;
	};

	/**
	 * Set a new `time::millis` function for the duration of the current scope;
	 * when the scope is left, the previous `time::millis` function is restored.
	 * Usage example:
	 * @code
	 * uint32_t my_millis()
	 * {
	 *     ...
	 * }
	 * void f()
	 * {
	 *     // Set new time::millis function
	 *     auto_millis d{my_millis};
	 *     // call API that rely on time::millis
	 *     ...
	 *     // Restore previous time::millis function
	 * }
	 * @endcode
	 */
	class auto_millis
	{
	public:
		/**
		 * Set new `time::millis` to @p new_millis after storing current function 
		 * for later restore.
		 * @param new_millis new function pointer to replace `time::millis`
		 */
		auto_millis(MILLIS_PTR new_millis) INLINE : old_millis_{millis}
		{
			millis = new_millis;
		}
		/// @cond notdocumented
		~auto_millis() INLINE
		{
			millis = old_millis_;
		}
		/// @endcond
	private:
		const MILLIS_PTR old_millis_;
	};
};

#endif /* TIME_HH */
/// @endcond
