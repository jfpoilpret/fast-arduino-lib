#ifndef BUTTON_HH
#define BUTTON_HH

#include <fastarduino/FastIO.hh>

//TODO Possibly use same header for both classes Buttons and Button
//TODO Some more refactoring is possible (AbstractButton<type> to use also with Buttons)
//TODO Implement relaxing button that triggers state() true only once until button is depressed
//TODO optimize size by using bits in one byte
class AbstractButton
{
protected:
	AbstractButton():_latest_state(true), _pending_state{}, _changed{}, _count() {}

	bool _state(bool state, uint8_t debounce_count)
	{
		// Don't return state unless it remained the same during DEBOUNCE_COUNT calls
		if (_count)
		{
			// We are in a debouncing phase, check if we have reached end of debounce time
			if (++_count == debounce_count)
			{
				_changed = (state == _pending_state);
				if (_changed)
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
		return !_latest_state;
	}
	
	bool _unique_press(bool state, uint8_t debounce_count)
	{
		return _state(state, debounce_count) && changed();
	}
	
public:
	inline bool changed() INLINE
	{
		return _changed;
	}
	
private:
	bool _latest_state;
	bool _pending_state;
	bool _changed;
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
