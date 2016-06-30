#ifndef WATCHDOG_HH
#define	WATCHDOG_HH

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "Events.hh"
#include "Board.hh"

//TODO consider using singleton isntead here (try to optimize code size, as static members access may mean larger code)
class Watchdog
{
public:
	enum TimeOut
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
	} __attribute__((packed));
	
	static void set_event_queue(Queue<Events::Event>& event_queue)
	{
		ClearInterrupt clint;
		_event_queue = &event_queue;
	}
	
	static void begin(TimeOut timeout = TO_16ms);
	static void end()
	{
		ClearInterrupt clint;
		WDTCSR = _BV(WDCE) | _BV(WDE);
		WDTCSR = 0;
	}
	
	static uint32_t millis()
	{
		ClearInterrupt clint;
		return _millis;
	}
	static void delay(uint32_t ms);
	static void await();
	
private:
	// This class is not instantiable
	Watchdog() {}

	static uint16_t _millis_per_tick;
	static volatile uint32_t _millis;
	static Queue<Events::Event>* _event_queue;
	
	friend void WDT_vect(void);
};

#endif	/* WATCHDOG_HH */
