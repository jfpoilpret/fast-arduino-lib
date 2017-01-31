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
#include "Board.hh"
#include "power.hh"

Time::DELAY_PTR Time::delay = Time::default_delay;
Time::MILLIS_PTR Time::millis = 0;

void Time::yield()
{
	Power::sleep();
}

Time::RTTTime Time::delta(const RTTTime& time1, const RTTTime& time2)
{
	uint32_t millis = (time1.millis <= time2.millis ? time2.millis - time1.millis : 0);
	uint16_t micros = 0;
	if (time1.micros <= time2.micros)
		micros = time2.micros - time1.micros;
	else if (millis)
	{
		--millis;
		micros = 1000 + time2.micros - time1.micros;
	}
	return RTTTime{millis, micros};
}

uint32_t Time::since(uint32_t start_ms)
{
	uint32_t now = Time::millis();
	return (start_ms <= now ? now - start_ms : 0);
}

void Time::default_delay(uint32_t ms)
{
	while (ms--) Time::delay_us(1000);
}
