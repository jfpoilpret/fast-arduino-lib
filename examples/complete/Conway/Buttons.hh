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
		//TODO this fi else is simplifyable (1st and 3rd condition can be merged in one)
		if (state == _latest_state)
		{
			_pending_state = state;
			_count = 0;
		}
		else if (state == _pending_state)
		{
			// Has new state been stable long enough?
			if (++_count == DEBOUNCE_COUNT)
				_latest_state = state;
		}
		else
		{
			// First is a really new state, start counting
			_pending_state = state;
			_count = 0;
		}
		return _latest_state;
	}
	
private:
	FastPort<PORT> _port;
	uint8_t _latest_state;
	uint8_t _pending_state;
	uint8_t _count;
};

#endif /* BUTTONS_HH */

