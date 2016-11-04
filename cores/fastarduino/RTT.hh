#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"
#include "Events.hh"

//TODO Improvements
// - use 2 flavours: one without event and one with event											1h+
//   -> how to deal with call to the right tick() method from ISR then?
// - limit friendship to the right ISR (how to do that?)											30'
// - improve _USE_RTT_TIMER to take Board::Timer value and remove each USE_RTT_TIMER_XX (how?)		30'
// - implement mechanism to set/restore Time::delay function (also for watchdog)					1h
// - add utility (here or in Time.hh, or somewhere else?) to compute time delta (in us)				30'

// This macro is internally used in further macros and should not be used in your programs
#define _USE_RTT_TIMER(TIMER_NUM)								\
ISR(TIMER ## TIMER_NUM ## _COMPA_vect)							\
{																\
	RTT<Board::Timer::TIMER ## TIMER_NUM>::_singleton->tick();	\
}

//#define _FRIEND_RTT_ISR(TIMER) friend void 

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use RTT for a given Timer in your program, hence you need the proper ISR vector correctly defined
#define USE_RTT_TIMER0()	_USE_RTT_TIMER(0)
#define USE_RTT_TIMER1()	_USE_RTT_TIMER(1)
#define USE_RTT_TIMER2()	_USE_RTT_TIMER(2)
#define USE_RTT_TIMER3()	_USE_RTT_TIMER(3)
#define USE_RTT_TIMER4()	_USE_RTT_TIMER(4)
#define USE_RTT_TIMER5()	_USE_RTT_TIMER(5)

struct RTTTime
{
	RTTTime(uint32_t millis, uint16_t micros):millis(millis), micros(micros) {}
	const uint32_t millis;
	const uint16_t micros;
};

template<Board::Timer TIMER>
class RTT
{
private:
	using TRAIT = Board::Timer_trait<TIMER>;
	using TYPE = typename TRAIT::TYPE;
	
public:
	RTT() INLINE:_millis{}
	{
		_singleton = this;
	}

	inline uint32_t millis() const
	{
		synchronized return _millis;
	}
	inline void millis(uint32_t ms)
	{
		synchronized
		{
			_millis = ms;
			_micros = 0;
			// Reset timer counter
			(volatile TYPE&) TRAIT::TCNT = 0;
		}
	}
	inline uint16_t micros() const
	{
		synchronized return compute_micros();
	}
	RTTTime time() const
	{
		synchronized return RTTTime(_millis, compute_micros());
	}
	void delay(uint32_t ms) const
	{
		uint32_t end = millis() + ms + 1;
		while (millis() < end)
			::Time::yield();
	}

	inline void begin()
	{
		synchronized
		{
			// Use a timer with 1 ms interrupts
			// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
			(volatile uint8_t&) TRAIT::TCCRA = TRAIT::TCCRA_VALUE;
			// Don't force output compare (FOCA & FOCB), Clock Select clk/64 (CS = 3)
			(volatile uint8_t&) TRAIT::TCCRB = TRAIT::TCCRB_VALUE;
			// Set timer counter compare match (when value reached, 1ms has elapsed)
			(volatile TYPE&) TRAIT::OCRA = (F_CPU / TRAIT::PRESCALER / 1000) - 1;
			// Reset timer counter
			(volatile TYPE&) TRAIT::TCNT = 0;
			// Set timer interrupt mode (set interrupt on OCRnA compare match)
			(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
		}
	}
	inline void end()
	{
		synchronized
		{
			// Stop timer
			(volatile uint8_t&) TRAIT::TCCRB = 0;
			// Clear timer interrupt mode (set interrupt on OCRnA compare match)
			(volatile uint8_t&) TRAIT::TIMSK = 0;
		}
	}
	
private:
	inline uint16_t compute_micros() const
	{
		return uint16_t(1000UL * ((volatile TYPE&) TRAIT::TCNT) / (1 + (volatile TYPE&) TRAIT::OCRA));
	}
	inline void tick()
	{
		++_millis;
	}
	
	volatile uint32_t _millis;
	uint16_t _micros;
	
	static RTT* _singleton;
	
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
RTT<TIMER>* RTT<TIMER>::_singleton = 0;

//class EventRTT: public RTT
//{
//protected:
//	EventRTT(Queue<Events::Event>& event_queue, uint32_t period = 1000):_event_queue(event_queue) {}
//	
//private:
//	Queue<Events::Event>& _event_queue;
//};

#endif /* RTT_HH */
