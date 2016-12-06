#ifndef MULTIPLEXER_HH
#define MULTIPLEXER_HH

#include <fastarduino/devices/SIPO.hh>

//TODO do we need BLINK_COUNT as class template argument, or just as method template argument?
//TODO Make this class more generic (make rows/columns parameters)
template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, uint8_t BLINK_COUNT = 2>
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

	// Refreshes the next row of LED and blink it
	// In this mode, blink is done for LEDs that are ON (in _data) and set to blink (in _blinks)
	void refresh_and_blink()
	{
		uint8_t data = _data[_row];
		if (_blink_count > BLINK_COUNT) data &= ~_blinks[_row];
		_sipo.output(as_uint16_t(_BV(_row), data ^ 0xFF));
		if (++_row == ROWS)
		{
			_row = 0;
			if (++_blink_count == 2 * BLINK_COUNT) _blink_count = 0;
		}
	}
	
	// Refreshes the next row of LED and blink LEDs that must blink
	// In this mode, blink is done for LEDs that are set to blink (in _blinks), whatever if they're ON or not (in _data)
	void blink()
	{
		uint8_t data = _data[_row];
		if (_blink_count > BLINK_COUNT)
			data &= ~_blinks[_row];
		else
			data |= _blinks[_row];
		_sipo.output(as_uint16_t(_BV(_row), data ^ 0xFF));
		if (++_row == ROWS)
		{
			_row = 0;
			if (++_blink_count == 2 * BLINK_COUNT) _blink_count = 0;
		}
	}
	
	// Refreshes the next row of LED without any blinking
	void refresh()
	{
		_sipo.output(as_uint16_t(_BV(_row), _data[_row] ^ 0xFF));
		if (++_row == ROWS)
			_row = 0;
	}
	
protected:
	SIPO<CLOCK, LATCH, DATA> _sipo;
	uint8_t _data[ROWS];
	uint8_t _blinks[ROWS];
	uint8_t _row;
	uint8_t _blink_count;
};

#endif /* MULTIPLEXER_HH */

