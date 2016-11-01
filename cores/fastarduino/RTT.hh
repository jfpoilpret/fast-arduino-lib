#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"
#include "Events.hh"

// This macro is internally used in further macros and should not be used in your programs
#define _USE_RTT_TIMER(TIMER_NUM)		\
ISR(TIMER ## TIMER_NUM ## _COMPA_vect)	\
{										\
	RTT* rtt = RTT::_singleton;			\
	if (rtt) rtt->tick();				\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use RTT for a given Timer in your program, hence you need the proper ISR vector correctly defined
#define USE_RTT_TIMER0()	_USE_RTT_TIMER(0)
#define USE_RTT_TIMER1()	_USE_RTT_TIMER(1)
#define USE_RTT_TIMER2()	_USE_RTT_TIMER(2)
#define USE_RTT_TIMER3()	_USE_RTT_TIMER(3)
#define USE_RTT_TIMER4()	_USE_RTT_TIMER(4)
#define USE_RTT_TIMER5()	_USE_RTT_TIMER(5)

class RTT
{
public:
	inline uint32_t millis() const
	{
		synchronized return _millis;
	}
	inline void millis(uint32_t ms)
	{
		synchronized _millis = ms;
	}
	void delay(uint32_t ms) const;

protected:
	RTT() INLINE:_millis{}
	{
		_singleton = this;
	}
	void _begin8(volatile uint8_t& TCCRA, volatile uint8_t& TCCRB, 
		volatile uint8_t& OCRA, volatile uint8_t& TCNT, volatile uint8_t& TIMSK);
	void _begin16(volatile uint8_t& TCCRA, volatile uint8_t& TCCRB, 
		volatile uint16_t& OCRA, volatile uint16_t& TCNT, volatile uint8_t& TIMSK);
	void _end(volatile uint8_t& TCCRA, volatile uint8_t& TIMSK);
	
private:
	inline void tick()
	{
		++_millis;
	}
	
	volatile uint32_t _millis;
	
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

//TODO Support Event queueing at defined period
// ==> need to change the way vector call tick()...)

//class EventRTT: public RTT
//{
//protected:
//	EventRTT(Queue<Events::Event>& event_queue, uint32_t period = 1000):_event_queue(event_queue) {}
//	
//private:
//	Queue<Events::Event>& _event_queue;
//};

template<Board::Timer8 TIMER>
class RTT8: public RTT
{
public:
	inline void begin()
	{
		_begin8(_TCCRA, _TCCRB, _OCRA, _TCNT, _TIMSK);
	}
	inline void end()
	{
		_end(_TCCRB, _TIMSK);
	}
	
private:
	static constexpr const REGISTER _TCCRA = Board::TCCRA(TIMER);
	static constexpr const REGISTER _TCCRB = Board::TCCRB(TIMER);
	static constexpr const REGISTER _TCNT = Board::TCNT(TIMER);
	static constexpr const REGISTER _OCRA = Board::OCRA(TIMER);
	static constexpr const REGISTER _TIMSK = Board::TIMSK(TIMER);
};

template<Board::Timer16 TIMER>
class RTT16: public RTT
{
public:
	inline void begin()
	{
		_begin16(_TCCRA, _TCCRB, _OCRA, _TCNT, _TIMSK);
	}
	inline void end()
	{
		_end(_TCCRB, _TIMSK);
	}
	
private:
	static constexpr const REGISTER _TCCRA = Board::TCCRA(TIMER);
	static constexpr const REGISTER _TCCRB = Board::TCCRB(TIMER);
	static constexpr const REGISTER _TCNT = Board::TCNT(TIMER);
	static constexpr const REGISTER _OCRA = Board::OCRA(TIMER);
	static constexpr const REGISTER _TIMSK = Board::TIMSK(TIMER);
};

#endif /* RTT_HH */
