#ifndef MULTIPLEXER_HH
#define MULTIPLEXER_HH

#include <fastarduino/devices/SIPO.hh>

template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, uint8_t BLINK_COUNT = 2>
class Matrix8x8Multiplexer
{
public:
	static constexpr const uint8_t ROWS = 8;
	static constexpr const uint8_t COLUMNS = 8;
	
	Matrix8x8Multiplexer()
		:_data(), _blinks(), _row(), _blink_count(), _blinking() {}
	
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
	
	inline bool is_blinking()
	{
		return _blinking;
	}
	
	inline void blinking(bool blink)
	{
		_blinking = blink;
	}
	
	void refresh()
	{
		uint8_t data = _data[_row];
		if (_blinking && _blink_count > BLINK_COUNT) data &= _blinks[_row];
		_sipo.output(as_uint16_t(data, _BV(_row) ^ 0xFF));
		if (++_row == ROWS)
		{
			_row = 0;
			if (++_blink_count == 2 * BLINK_COUNT) _blink_count = 0;
		}
	}
	
protected:
	SIPO<CLOCK, LATCH, DATA, uint16_t> _sipo;
	uint8_t _data[ROWS];
	uint8_t _blinks[ROWS];
	uint8_t _row;
	uint8_t _blink_count;
	bool _blinking;
};

#endif /* MULTIPLEXER_HH */

