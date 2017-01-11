#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>
#include "Timer.hh"
#include "time.hh"
#include "Events.hh"

//TODO do we need to put everything here in a namespace?

class RTTCallback
{
public:
	virtual void on_rtt_change(uint32_t millis) = 0;
};

template<Board::Timer TIMER>
class RTT: private TimerCallback, private Timer<TIMER>
{
private:
	using TRAIT = typename RTT::TRAIT;
	using TIMER_TYPE = typename RTT::TIMER_TYPE;
	using TIMER_PRESCALER = typename RTT::TIMER_PRESCALER;
	
public:
	RTT() INLINE : Timer<TIMER>((TimerCallback&) *this), _callback{0} {}

	void set_callback(RTTCallback* callback)
	{
		_callback = callback;
	}
	
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
		synchronized _begin();
	}
	inline void _begin()
	{
		_millis = 0;
		Timer<TIMER>::_begin(MILLI_PRESCALER, MILLI_COUNTER);
	}
	inline void end()
	{
		Timer<TIMER>::end();
	}
	inline void _end()
	{
		Timer<TIMER>::_end();
	}
	
private:
	static constexpr const uint32_t ONE_MILLI = 1000UL;
	static constexpr const TIMER_PRESCALER MILLI_PRESCALER = RTT::prescaler(ONE_MILLI);
	static constexpr const TIMER_TYPE MILLI_COUNTER = RTT::counter(ONE_MILLI);
	
	volatile uint32_t _millis;
	RTTCallback* _callback;
	
	virtual void on_timer() override
	{
		++_millis;
		if (_callback) _callback->on_rtt_change(_millis);
	}
	
	inline uint16_t compute_micros() const
	{
		return uint16_t(1000UL * ((volatile TIMER_TYPE&) TRAIT::TCNT) / (1 + (volatile TIMER_TYPE&) TRAIT::OCRA));
	}
};

template<uint32_t PERIOD_MS = 1024>
class RTTEventCallback: public RTTCallback
{
	static_assert((PERIOD_MS & (PERIOD_MS - 1)) == 0, "PERIOD_MS must be a power of 2");
public:
	RTTEventCallback(Queue<Events::Event>& event_queue)
		:_event_queue(event_queue) {}
	
private:
	virtual void on_rtt_change(uint32_t millis) override
	{
		if ((millis & (PERIOD_MS - 1)) == 0)
			_event_queue._push(Events::Event{Events::Type::RTT_TIMER});
	}
	
	Queue<Events::Event>& _event_queue;
};

#endif /* RTT_HH */
