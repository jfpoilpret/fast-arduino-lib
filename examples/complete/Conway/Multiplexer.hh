#ifndef MULTIPLEXER_HH
#define MULTIPLEXER_HH

#include <fastarduino/devices/SIPO.hh>

enum class BlinkMode: uint8_t
{
	// In this mode, no blink occurs at all
	NO_BLINK = 0,
	// In this mode, blink is done for LEDs that are ON (in _data), whatever _blinks content
	BLINK_ALL_DATA,
	// In this mode, blink is done for LEDs that are set to blink (in _blinks), whatever if they're ON or not (in _data)
	BLINK_ALL_BLINKS,
	// In this mode, blink is done for LEDs that are ON (in _data) and set to blink (in _blinks)
	BLINK_BLINKABLE_DATA
};

//TODO do we need BLINK_COUNT as class template argument, or just as method template argument?
//TODO Make this class more generic (make rows/columns parameters)
template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, uint16_t BLINK_COUNT = 16>
class Matrix8x8Multiplexer
{
private:
	using SIPO_TYPE = SIPO<CLOCK, LATCH, DATA>;
	
public:
	static constexpr const uint8_t ROWS = 8;
	static constexpr const uint8_t COLUMNS = 8;
	
	static constexpr const Board::Port PORT = SIPO_TYPE::PORT;
	static constexpr const uint8_t DDR_MASK = SIPO_TYPE::DDR_MASK;
	static constexpr const uint8_t PORT_MASK = SIPO_TYPE::PORT_MASK;

	Matrix8x8Multiplexer():_data(), _blinks(), _row(), _blink_count() {}
	
	inline void init()
	{
		_sipo.init();
	}
	inline uint8_t* data()
	{
		return _data;
	}
	
	// if 0, that means this pixel shall blink, if 1 it shall never blink
	// only lit pixels can blink, dark pixels never blink
	inline uint8_t* blinks()
	{
		return _blinks;
	}

	void refresh(BlinkMode blink_mode)
	{
		uint8_t data = _data[_row];
		if (blink_mode != BlinkMode::NO_BLINK)
		{
			if (_blink_count > BLINK_COUNT / ROWS)
			{
				if (blink_mode == BlinkMode::BLINK_BLINKABLE_DATA || blink_mode == BlinkMode::BLINK_ALL_BLINKS)
					data &= ~_blinks[_row];
				else
					data = 0;
			}
			else if (blink_mode == BlinkMode::BLINK_ALL_BLINKS)
				data |= _blinks[_row];
		}
		_sipo.output(as_uint16_t(_BV(_row) ^ 0xFF, data));
		if (++_row == ROWS)
		{
			_row = 0;
			if (blink_mode != BlinkMode::NO_BLINK && ++_blink_count == 2 * BLINK_COUNT / ROWS)
				_blink_count = 0;
		}
	}
	
	// Refreshes the next row of LED without any blinking
	// This is equivalent to refresh(BlinkMode::NO_BLINK) but smaller and faster
	void refresh()
	{
		_sipo.output(as_uint16_t(_BV(_row) ^ 0xFF, _data[_row]));
		if (++_row == ROWS)
			_row = 0;
	}
	
	void clear()
	{
		_sipo.output(0);
	}
	
protected:
	SIPO<CLOCK, LATCH, DATA> _sipo;
	uint8_t _data[ROWS];
	uint8_t _blinks[ROWS];
	uint8_t _row;
	uint16_t _blink_count;
};

#endif /* MULTIPLEXER_HH */

