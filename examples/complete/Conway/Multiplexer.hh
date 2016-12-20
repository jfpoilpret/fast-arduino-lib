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

// Special "traits" used by Multiplexer to determine, at compile time, its behavior based on the number of 
// ROWS and COLUMNS
template<typename ROW_TYPE_, uint8_t ROWS_>
struct MATRIX_TRAIT
{
	using ROW_TYPE = ROW_TYPE_;
	using COLUMN_TYPE = uint8_t;
	using TYPE = uint16_t;
	static TYPE as_type(COLUMN_TYPE column, ROW_TYPE row);
	static constexpr const bool SUPPORTED = false;
};

template<typename ROW_TYPE_, typename COLUMN_TYPE_, typename TYPE_>
struct MATRIX_TRAIT_IMPL
{
	using ROW_TYPE = ROW_TYPE_;
	using COLUMN_TYPE = COLUMN_TYPE_;
	using TYPE = TYPE_;
	static TYPE as_type(COLUMN_TYPE column, ROW_TYPE row)
	{
		return TYPE((column << (8 * sizeof(ROW_TYPE))) | row);
	}
	static constexpr const bool SUPPORTED = true;
};

template<> struct MATRIX_TRAIT<uint8_t, 8>: MATRIX_TRAIT_IMPL<uint8_t, uint8_t, uint16_t> {};
template<> struct MATRIX_TRAIT<uint16_t, 16>: MATRIX_TRAIT_IMPL<uint16_t, uint16_t, uint32_t> {};
//TODO other template for more special cases: 16+8, 8+16, 24+8...

//TODO do we need BLINK_COUNT as class template argument, or just as method template argument?

//TODO improve use ROWS/COLUMNS as parameters and deduce types from TRAIT
template<	Board::DigitalPin CLOCK, 
			Board::DigitalPin LATCH, 
			Board::DigitalPin DATA, 
			uint16_t BLINK_COUNT = 16,
			uint8_t ROWS_ = 8,
			typename ROW_TYPE_ = uint8_t>
class MatrixMultiplexer
{
private:
	using SIPO_TYPE = SIPO<CLOCK, LATCH, DATA>;
	using TRAIT = MATRIX_TRAIT<ROW_TYPE_, ROWS_>;
	
	static_assert(TRAIT::SUPPORTED, "Provided ROW_TYPE_ and ROWS_ template parameters are not supported");
	
public:
	using ROW_TYPE = ROW_TYPE_;
	static constexpr const uint8_t ROWS = ROWS_;
	static constexpr const uint8_t COLUMNS = sizeof(ROW_TYPE) * 8;
	
	static constexpr const Board::Port PORT = SIPO_TYPE::PORT;
	static constexpr const uint8_t DDR_MASK = SIPO_TYPE::DDR_MASK;
	static constexpr const uint8_t PORT_MASK = SIPO_TYPE::PORT_MASK;

	MatrixMultiplexer():_data(), _blinks(), _row(), _blink_count() {}
	
	inline void init()
	{
		_sipo.init();
	}
	inline ROW_TYPE* data()
	{
		return _data;
	}
	
	// if 0, that means this pixel shall blink, if 1 it shall never blink
	// only lit pixels can blink, dark pixels never blink
	inline ROW_TYPE* blinks()
	{
		return _blinks;
	}

	void refresh(BlinkMode blink_mode)
	{
		ROW_TYPE data = _data[_row];
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
		//FIXME Adapt to work on any ROW/COLUMN size
		_sipo.output(TRAIT::as_type(~_BV(_row), data));
//		_sipo.output(as_uint16_t(_BV(_row) ^ 0xFF, data));
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
		//FIXME Adapt to work on any ROW/COLUMN size
		_sipo.output(TRAIT::as_type(~_BV(_row), _data[_row]));
//		_sipo.output(as_uint16_t(_BV(_row) ^ 0xFF, _data[_row]));
		if (++_row == ROWS)
			_row = 0;
	}
	
	void clear()
	{
		//FIXME Adapt to work on any ROW/COLUMN size
		_sipo.output(TRAIT::as_type(0, 0));
//		_sipo.output(uint16_t(0));
	}
	
protected:
	SIPO<CLOCK, LATCH, DATA> _sipo;
	ROW_TYPE _data[ROWS];
	ROW_TYPE _blinks[ROWS];
	uint8_t _row;
	uint16_t _blink_count;
};

#endif /* MULTIPLEXER_HH */

