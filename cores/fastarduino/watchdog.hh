#ifndef WATCHDOG_HH
#define	WATCHDOG_HH

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "Events.hh"
#include "Board.hh"

class Watchdog
{
public:
	enum class TimeOut: uint8_t
	{
		TO_16ms = 0,
		TO_32ms,
		TO_64ms,
		TO_125ms,
		TO_250ms,
		TO_500ms,
		TO_1s,
		TO_2s,
		TO_4s,
		TO_8s
	};
	
	Watchdog(Queue<Events::Event>& event_queue)
		:_millis{0}, _millis_per_tick{0}, _event_queue(event_queue)
	{
		Watchdog::_singleton = this;
	}
	Watchdog(const Watchdog&) = delete;
	
	void begin(TimeOut timeout = TimeOut::TO_16ms);
	void end()
	{
		synchronized
		{
			WDTCSR = _BV(WDCE) | _BV(WDE);
			WDTCSR = 0;
		}
	}
	
	uint32_t millis() const
	{
		synchronized return _millis;
	}
	void delay(uint32_t ms);
	
private:
	volatile uint32_t _millis;
	uint16_t _millis_per_tick;
	Queue<Events::Event>& _event_queue;
	
	static Watchdog *_singleton;
	friend void WDT_vect(void);
};

#endif	/* WATCHDOG_HH */
