#ifndef TIMER_HH
#define TIMER_HH

#include <avr/interrupt.h>
#include <stddef.h>

#include "utilities.hh"
#include "Board_traits.hh"

//TODO do we need to put everything here in a namespace?

// These macros are internally used in further macro and should not be used in your programs
#define ISR_TIMER_(T0, ...)										\
ISR(TIMER ## T0 ## _COMPA_vect, ISR_NAKED)						\
{																\
	asm volatile(												\
		"push r16\n\t"											\
		"ldi r16, %[TIMER]\n\t"									\
		"out %[GPIOR], r16\n\t"									\
		::[GPIOR] "I" (_SFR_IO_ADDR(GPIOR0)), [TIMER] "I" (T0)	\
	);															\
	ISRCallback::callback();									\
	asm volatile(												\
		"pop r16\n\t"											\
		"reti\n\t"												\
	);															\
}																\

#define PREPEND_TIMER_(TN, ...) Board::Timer::TIMER ## TN

#define USE_TIMERS(T0, ...)																				\
using ISRCallback =																						\
	timer_impl::ISRHandler<FOR_EACH_SEP(PREPEND_TIMER_, , EMPTY, COMMA, EMPTY, T0, ##__VA_ARGS__)>;		\
																										\
FOR_EACH(ISR_TIMER_, , T0, ##__VA_ARGS__)

// Forward declaration necessary to be declared as friend
// Complete declaration can be found at the end of this file
namespace timer_impl
{
	template<Board::Timer T0, Board::Timer... TS> struct CallbackHandler;
}

class TimerCallback
{
public:
	virtual void on_timer() = 0;
};

template<Board::Timer TIMER>
class Timer
{
protected:
	using TRAIT = Board::Timer_trait<TIMER>;
	using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;
	
public:
	using TIMER_TYPE = typename TRAIT::TYPE;
	using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;

	Timer(TimerCallback& callback) INLINE
	{
		_callback = &callback;
	}

	static constexpr bool is_adequate(TIMER_PRESCALER p, uint32_t us)
	{
		return prescaler_is_adequate(prescaler_quotient(p, us));
	}
	static constexpr TIMER_PRESCALER prescaler(uint32_t us)
	{
		return best_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us);
	}
	static constexpr TIMER_TYPE counter(TIMER_PRESCALER prescaler, uint32_t us)
	{
		return (TIMER_TYPE) prescaler_quotient(prescaler, us) - 1;
	}
	static constexpr TIMER_TYPE counter(uint32_t us)
	{
		return (TIMER_TYPE) prescaler_quotient(prescaler(us), us) - 1;
	}

	inline void begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
	{
		synchronized _begin(prescaler, max);
	}

	// We'll need additional methods in Timer_trait<>
	inline void _begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
	{
		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
		(volatile uint8_t&) TRAIT::TCCRA = TRAIT::CTC_TCCRA;
		// Don't force output compare (FOCA & FOCB), Clock Select according to prescaler
		(volatile uint8_t&) TRAIT::TCCRB = TRAIT::CTC_TCCRB | TRAIT::TCCRB_prescaler(prescaler);
		// Set timer counter compare match (when value reached, 1ms has elapsed)
		(volatile TIMER_TYPE&) TRAIT::OCRA = max;
		// Reset timer counter
		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
	}
	inline void suspend()
	{
		synchronized _suspend();
	}
	inline void _suspend()
	{
		// Clear timer interrupt mode
		(volatile uint8_t&) TRAIT::TIMSK = 0;
	}
	inline void resume()
	{
		synchronized _resume();
	}
	inline void _resume()
	{
		// Reset timer counter
		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
	}
	inline bool is_suspended()
	{
		return (volatile uint8_t&) TRAIT::TIMSK == 0;
	}
	inline void end()
	{
		synchronized _end();
	}
	inline void _end()
	{
		// Stop timer
		(volatile uint8_t&) TRAIT::TCCRB = 0;
		// Clear timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = 0;
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
	
	static void call_back_if_needed()
	{
		if (GPIOR0 == uint8_t(TIMER))
			_callback->on_timer();
	}

	static TimerCallback* _callback;
	
	template<Board::Timer, Board::Timer...> friend struct timer_impl::CallbackHandler;
};

template<Board::Timer TIMER>
TimerCallback* Timer<TIMER>::_callback = 0;

namespace timer_impl
{
	template<Board::Timer... TS>
	struct ISRHandler
	{
		static void callback() __attribute__((naked))
		{
			asm volatile(
				"push r1\n\t"
				"push r0\n\t"
				"in r0, __SREG__\n\t"
				"push r0\n\t"
				"eor r1, r1\n\t"
				"push r18\n\t"
				"push r19\n\t"
				"push r20\n\t"
				"push r21\n\t"
				"push r22\n\t"
				"push r23\n\t"
				"push r24\n\t"
				"push r25\n\t"
				"push r26\n\t"
				"push r27\n\t"
				"push r30\n\t"
				"push r31\n\t"
			);
			CallbackHandler<TS...>::callback();
			asm volatile(
				"pop r31\n\t"
				"pop r30\n\t"
				"pop r27\n\t"
				"pop r26\n\t"
				"pop r25\n\t"
				"pop r24\n\t"
				"pop r23\n\t"
				"pop r22\n\t"
				"pop r21\n\t"
				"pop r20\n\t"
				"pop r19\n\t"
				"pop r18\n\t"
				"pop r0\n\t"
				"out __SREG__, r0\n\t"
				"pop r0\n\t"
				"pop r1\n\t"
				"ret\n\t"
			);
		}
	};
	
	template<Board::Timer T0, Board::Timer... TS>
	struct CallbackHandler
	{
		static void callback()
		{
			CallbackHandler<T0>::callback();
			CallbackHandler<TS...>::callback();
		}
	};

	template<Board::Timer T>
	struct CallbackHandler<T>
	{
		static void callback()
		{
			Timer<T>::call_back_if_needed();
		}
	};
}

#endif /* TIMER_HH */
