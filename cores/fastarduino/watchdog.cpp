#include "watchdog.hh"
#include "utilities.hh"

uint16_t Watchdog::_millis_per_tick = 0;
volatile uint32_t Watchdog::_millis = 0;
Queue<Events::Event>* Watchdog::_event_queue = 0;

void Watchdog::begin(TimeOut timeout)
{
	uint16_t ms_per_tick = 1 << (timeout + 4);
	uint8_t config = _BV(WDIE) | (timeout & 0x07) | (timeout & 0x08 ? _BV(WDP3) : 0);
	
	ClearInterrupt clint;
	wdt_reset();
	//TODO Check generated assembly!!!
	MCUSR |= 1 << WDRF;
	WDTCSR = _BV(WDCE) | _BV(WDE);
	WDTCSR = config;
	_millis_per_tick = ms_per_tick;
	_millis = 0;
}

void Watchdog::delay(uint32_t ms)
{
	//TODO
	uint32_t limit = millis() + ms;
	while (millis() < limit)
	{
		//TODO yield...
		;
	}
}

ISR(WDT_vect)
{
	// Introduce timing somehow in the event (or outside)
	Watchdog::_millis += Watchdog::_millis_per_tick;
	// Just send a timeout event
	if (Watchdog::_event_queue)
		Watchdog::_event_queue->push(Events::Event{Events::Type::WDT_TIMER});
}
