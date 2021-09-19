//   Copyright 2016-2021 Jean-Francois Poilpret
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

#include "boards/board.h"
#include "time.h"
#include "power.h"

time::DELAY_PTR time::delay = time::default_delay;
time::MILLIS_PTR time::millis = nullptr;

void time::yield()
{
	power::Power::sleep();
}

time::RTTTime time::delta(const RTTTime& time1, const RTTTime& time2)
{
	uint32_t millis = ((time1.millis() <= time2.millis()) ? (time2.millis() - time1.millis()) : 0);
	uint16_t micros = 0;
	if (time1.micros() <= time2.micros())
		micros = time2.micros() - time1.micros();
	else if (millis)
	{
		--millis;
		micros = ONE_MILLI_16 + time2.micros() - time1.micros();
	}
	return RTTTime{millis, micros};
}

uint32_t time::since(uint32_t start_ms)
{
	uint32_t now = time::millis();
	return ((start_ms <= now) ? (now - start_ms) : 0);
}

void time::default_delay(uint32_t ms)
{
	while (ms--) time::delay_us(ONE_MILLI_16);
}
