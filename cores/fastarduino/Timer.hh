#ifndef TIMER_HH
#define TIMER_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board_traits.hh"

//TODO do we need to put everything here in a namespace?

//TODO Improve size by using one unique vector and aliases for others
// Then the vector should check which timer interrupt is on and call the matching Timer class template instance
// This macro is internally used in further macros and should not be used in your programs
#define _USE_TIMER(TIMER_NUM)										\
ISR(TIMER ## TIMER_NUM ## _COMPA_vect)								\
{																	\
	Timer<Board::Timer::TIMER ## TIMER_NUM>::_callback->on_timer();	\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use RTT for a given Timer in your program, hence you need the proper ISR vector correctly defined
#define USE_TIMER0()	_USE_TIMER(0)
#define USE_TIMER1()	_USE_TIMER(1)
#define USE_TIMER2()	_USE_TIMER(2)
#define USE_TIMER3()	_USE_TIMER(3)
#define USE_TIMER4()	_USE_TIMER(4)
#define USE_TIMER5()	_USE_TIMER(5)

class TimerCallback
{
public:
	virtual void on_timer() = 0;
};

template<Board::Timer TIMER>
class Timer
{
private:
	using TRAIT = Board::Timer_trait<TIMER>;
	using TIMER_TYPE = typename TRAIT::TYPE;
	
public:
	Timer(TimerCallback& callback) INLINE
	{
		_callback = &callback;
	}

	inline void begin(Board::TimerPrescaler prescaler, TIMER_TYPE max)
	{
		synchronized _begin(prescaler, max);
	}
//	inline void begin(uint16_t period_ms)
//	{
//		synchronized _begin();
//	}
	//TODO calculate PRESCALER and OCRA based on period_ms
	// We'll need additional methods in Timer_trait<>
	inline void _begin(Board::TimerPrescaler prescaler, TIMER_TYPE max)
	{
		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
		(volatile uint8_t&) TRAIT::TCCRA = TRAIT::TCCRA_VALUE;
		// Don't force output compare (FOCA & FOCB), Clock Select according to prescaler
		(volatile uint8_t&) TRAIT::TCCRB = TRAIT::TCCRB_prescaler(prescaler);
		// Set timer counter compare match (when value reached, 1ms has elapsed)
		(volatile TIMER_TYPE&) TRAIT::OCRA = max;
		// Reset timer counter
		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
	}
//	inline void _begin(uint8_t period_ms)
//	{
//		// Use a timer with 1 ms interrupts
//		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
//		(volatile uint8_t&) TRAIT::TCCRA = TRAIT::TCCRA_VALUE;
//		// Don't force output compare (FOCA & FOCB), Clock Select clk/64 (CS = 3)
//		(volatile uint8_t&) TRAIT::TCCRB = TRAIT::TCCRB_VALUE;
//		// Set timer counter compare match (when value reached, 1ms has elapsed)
//		(volatile TIMER_TYPE&) TRAIT::OCRA = (F_CPU / TRAIT::PRESCALER / 1000 * period_ms) - 1;
//		// Reset timer counter
//		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
//		// Set timer interrupt mode (set interrupt on OCRnA compare match)
//		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
//	}
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
	static TimerCallback* _callback;
	
	friend void TIMER0_COMPA_vect();
#ifdef TIMER1_COMPA_vect
	friend void TIMER1_COMPA_vect();
#endif
#ifdef TIMER2_COMPA_vect
	friend void TIMER2_COMPA_vect();
#endif
#ifdef TIMER3_COMPA_vect
	friend void TIMER3_COMPA_vect();
#endif
#ifdef TIMER4_COMPA_vect
	friend void TIMER4_COMPA_vect();
#endif
#ifdef TIMER5_COMPA_vect
	friend void TIMER5_COMPA_vect();
#endif
};

template<Board::Timer TIMER>
TimerCallback* Timer<TIMER>::_callback = 0;

#endif /* TIMER_HH */
