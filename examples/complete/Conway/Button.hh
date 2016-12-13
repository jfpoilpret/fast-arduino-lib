#ifndef BUTTON_HH
#define BUTTON_HH

#include <fastarduino/FastIO.hh>

//TODO Possibly use same header for both classes Buttons and Button
//TODO Some more refactoring is possible (AbstractButton<type> to use also with Buttons)
//TODO optimize size by using bits in one byte
class AbstractButton
{
protected:
	AbstractButton():_status{}, _count() {}

	bool _state(bool state, uint8_t debounce_count)
	{
		// Don't return state unless it remained the same during DEBOUNCE_COUNT calls
		_status.changed = false;
		if (_count)
		{
			// We are in a debouncing phase, check if we have reached end of debounce time
			if (++_count == debounce_count)
			{
				if (state == _status.pending)
				{
					_status.changed = true;
					_status.latest = state;
				}
				_count = 0;
			}
		}
		else if (state != _status.latest)
		{
			// State has changed for the first time, start debouncing period now
			_status.pending = state;
			_count = 1;
		}
		// Note that we want state to hold 1 when button is pushed, hence we invert all bits linked to button pins
		return !_status.latest;
	}
	
	bool _unique_press(bool state, uint8_t debounce_count)
	{
		return _state(state, debounce_count) && changed();
	}
	
public:
	inline bool changed() INLINE
	{
		return _status.changed;
	}
	
private:
	struct State
	{
		State():latest{true}, pending{}, changed{} {}
		
		bool latest		:1;
		bool pending	:1;
		bool changed	:1;
	} _status;
	uint8_t _count;
};

template<Board::DigitalPin DPIN, uint8_t DEBOUNCE_COUNT>
class Button: public AbstractButton
{
public:
	static constexpr const Board::Port PORT = FastPinType<DPIN>::PORT;
	static constexpr const uint8_t DDR_MASK = 0;
	static constexpr const uint8_t PORT_MASK = FastPinType<DPIN>::MASK;
	
	Button() {}

	inline void init() INLINE
	{
		_pin.set_mode(PinMode::INPUT_PULLUP);
	}

	inline bool state() INLINE
	{
		return _state(_pin.value(), DEBOUNCE_COUNT);
	}
	
	inline bool unique_press() INLINE
	{
		return _unique_press(_pin.value(), DEBOUNCE_COUNT);
	}

private:
	typename FastPinType<DPIN>::TYPE _pin;
};

#endif /* BUTTON_HH */
