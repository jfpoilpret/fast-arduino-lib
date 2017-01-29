#include "time.hh"
#include "watchdog.hh"
#include "utilities.hh"

void WatchdogSignal::begin(TimeOut timeout)
{
	uint8_t config = _BV(WDIE) | (uint8_t(timeout) & 0x07) | (uint8_t(timeout) & 0x08 ? _BV(WDP3) : 0);
	synchronized _begin(config);
}

void Watchdog::begin(TimeOut timeout)
{
	uint16_t ms_per_tick = 1 << (uint8_t(timeout) + 4);
	uint8_t config = _BV(WDIE) | (uint8_t(timeout) & 0x07) | (uint8_t(timeout) & 0x08 ? _BV(WDP3) : 0);
	
	synchronized
	{
		_begin(config);
		_millis_per_tick = ms_per_tick;
		_millis = 0;
	}
}

void Watchdog::delay(uint32_t ms)
{
	uint32_t limit = millis() + ms;
	while (millis() < limit)
	{
		Time::yield();
	}
}
