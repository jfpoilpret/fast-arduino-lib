#include "watchdog.hh"
#include "utilities.hh"

Watchdog* Watchdog::_singleton = 0;

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
	Watchdog* watchdog = Watchdog::_singleton;
	if (watchdog)
	{
		watchdog->_millis += watchdog->_millis_per_tick;
		watchdog->_event_queue.push(Events::Event{Events::Type::WDT_TIMER});
	}
}
