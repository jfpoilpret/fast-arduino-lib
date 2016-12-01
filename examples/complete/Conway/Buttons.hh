#ifndef BUTTONS_HH
#define BUTTONS_HH

#include <fastarduino/FastIO.hh>

template<Board::Port PORT, uint8_t MASK, uint8_t DEBOUNCE_COUNT>
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

