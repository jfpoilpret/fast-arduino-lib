#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "time.hh"
#include "Board.hh"
#include "Events.hh"

//TODO do we need to put everything here in a namespace?

// This macro is internally used in further macros and should not be used in your programs
#define _USE_RTT_TIMER(TIMER_NUM)		\
ISR(TIMER ## TIMER_NUM ## _COMPA_vect)	\
{										\
	AbstractRTT::_singleton->tick();	\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use RTT for a given Timer in your program, hence you need the proper ISR vector correctly defined
#define USE_RTT_TIMER0()	_USE_RTT_TIMER(0)
#define USE_RTT_TIMER1()	_USE_RTT_TIMER(1)
#define USE_RTT_TIMER2()	_USE_RTT_TIMER(2)
#define USE_RTT_TIMER3()	_USE_RTT_TIMER(3)
#define USE_RTT_TIMER4()	_USE_RTT_TIMER(4)
#define USE_RTT_TIMER5()	_USE_RTT_TIMER(5)

class RTTCallback
{
public:
	virtual void on_rtt_change(uint32_t millis) = 0;
};

class AbstractRTT
{
public:
	void set_callback(RTTCallback* callback)
	{
		_callback = callback;
	}
	
private:
	AbstractRTT() INLINE:_millis{}, _callback{0}
	{
		_singleton = this;
	}

	volatile uint32_t _millis;

	inline void tick()
	{
		++_millis;
		if (_callback) _callback->on_rtt_change(_millis);
	}
	
	RTTCallback* _callback;
	static AbstractRTT* _singleton;
	
	template<Board::Timer TIMER> friend class RTT;
	
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
class RTT: public AbstractRTT
{
private:
	using TRAIT = Board::Timer_trait<TIMER>;
	using TIMER_TYPE = typename TRAIT::TYPE;
	
public:
	RTT() INLINE {}

	inline uint32_t millis() const
	{
		synchronized return _millis;
	}
	void delay(uint32_t ms) const
	{
		uint32_t end = millis() + ms + 1;
		while (millis() < end)
			::Time::yield();
	}
	inline uint16_t micros() const
	{
		synchronized return compute_micros();
	}
	Time::RTTTime time() const
	{
		synchronized return Time::RTTTime(_millis, compute_micros());
	}

	inline void millis(uint32_t ms)
	{
		synchronized
		{
			_millis = ms;
			// Reset timer counter
			(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		}
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
			(volatile TIMER_TYPE&) TRAIT::OCRA = (F_CPU / TRAIT::PRESCALER / 1000) - 1;
			// Reset timer counter
			(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
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
		return uint16_t(1000UL * ((volatile TIMER_TYPE&) TRAIT::TCNT) / (1 + (volatile TIMER_TYPE&) TRAIT::OCRA));
	}
};

class RTTEventCallback: public RTTCallback
{
public:
	RTTEventCallback(Queue<Events::Event>& event_queue, uint32_t period_ms = 1000)
		:_event_queue(event_queue), _period_ms(period_ms) {}
	
private:
	virtual void on_rtt_change(uint32_t millis) override
	{
		// This unusual way to check if millis is multiple of _period_ms allows avoiding division (extra code)
		while (millis >= _period_ms)
			millis -= _period_ms;
		if (!millis)
			_event_queue._push(Events::Event{Events::Type::RTT_TIMER});
	}
	
	Queue<Events::Event>& _event_queue;
	const uint32_t _period_ms;
};

#endif /* RTT_HH */
