//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

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
