//   Copyright 2016-2022 Jean-Francois Poilpret
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

#ifndef BUTTONS_HH
#define BUTTONS_HH

#include <fastarduino/gpio.h>

template<board::Port PORT, uint8_t MASK, uint8_t DEBOUNCE_COUNT>
class Buttons
{
public:
	Buttons():_latest_state(MASK), _pending_state(), _count() {}

	inline void init()
	{
		_port.set_DDR(0);
		_port.set_PORT(MASK);
	}

	uint8_t state()
	{
		uint8_t state = _port.get_PIN() & MASK;
		// Don't return state unless it remained the same during DEBOUNCE_COUNT calls
		if (_count)
		{
			// We are in a debouncing phase, check if we have reached end of debounce time
			if (++_count == DEBOUNCE_COUNT)
			{
				if (state == _pending_state)
					_latest_state = state;
				_count = 0;
			}
		}
		else if (state != _latest_state)
		{
			// State has changed for the first time, start debouncing period now
			_pending_state = state;
			_count = 1;
		}
		// Note that we want state to hold 1 when button is pushed, hence we invert all bits linked to button pins
		return _latest_state ^ MASK;
	}
	
private:
	FastPort<PORT> _port;
	uint8_t _latest_state;
	uint8_t _pending_state;
	uint8_t _count;
};

#endif /* BUTTONS_HH */

