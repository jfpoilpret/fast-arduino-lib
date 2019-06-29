//   Copyright 2016-2019 Jean-Francois Poilpret
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

#ifndef MULTIPLEXER_HH
#define MULTIPLEXER_HH

#include <fastarduino/devices/sipo.h>

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
template<uint8_t ROWS_, uint8_t COLUMNS_>
struct MATRIX_TRAIT
{
	using COLUMN_TYPE = uint8_t;
	using ROW_TYPE = uint8_t;
	using TYPE = uint16_t;
	static TYPE as_type(COLUMN_TYPE column, ROW_TYPE row);
	static constexpr const bool SUPPORTED = false;
};

template<typename ROW_TYPE_, typename COLUMN_TYPE_, typename TYPE_>
struct MATRIX_TRAIT_IMPL
{
	using COLUMN_TYPE = COLUMN_TYPE_;
	using ROW_TYPE = ROW_TYPE_;
	using TYPE = TYPE_;
	static TYPE as_type(COLUMN_TYPE column, ROW_TYPE row)
	{
		return ((TYPE(column)) << (8 * sizeof(ROW_TYPE))) | TYPE(row);
	}
	static constexpr const bool SUPPORTED = true;
};

template<> struct MATRIX_TRAIT<8, 8>: MATRIX_TRAIT_IMPL<uint8_t, uint8_t, uint16_t> {};
template<> struct MATRIX_TRAIT<16, 16>: MATRIX_TRAIT_IMPL<uint16_t, uint16_t, uint32_t> {};

template<	board::DigitalPin CLOCK, 
			board::DigitalPin LATCH, 
			board::DigitalPin DATA, 
			uint16_t BLINK_COUNT = 16,
			uint8_t ROWS_ = 8,
			uint8_t COLUMNS_ = 8>
class MatrixMultiplexer
{
private:
	using SIPO_TYPE = devices::SIPO<CLOCK, LATCH, DATA>;
	using TRAIT = MATRIX_TRAIT<COLUMNS_, ROWS_>;
	
	static_assert(TRAIT::SUPPORTED, "Provided COLUMNS_ and ROWS_ template parameters are not supported");
	
public:
	using ROW_TYPE = typename TRAIT::ROW_TYPE;
	static constexpr const uint8_t ROWS = ROWS_;
	static constexpr const uint8_t COLUMNS = COLUMNS_;
	
	static constexpr const board::Port PORT = gpio::FastPinType<CLOCK>::PORT;
	static constexpr const uint8_t DDR_MASK = 
		gpio::FastPinType<CLOCK>::MASK | gpio::FastPinType<LATCH>::MASK | gpio::FastPinType<DATA>::MASK;
	static constexpr const uint8_t PORT_MASK = gpio::FastPinType<LATCH>::MASK;

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
		_sipo.output(TRAIT::as_type(bits::CBV8(_row), data));
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
		_sipo.output(TRAIT::as_type(bits::CBV8(_row), _data[_row]));
		if (++_row == ROWS)
			_row = 0;
	}
	
	void clear()
	{
		_sipo.output(TRAIT::as_type(0, 0));
	}
	
protected:
	devices::SIPO<CLOCK, LATCH, DATA> _sipo;
	ROW_TYPE _data[ROWS];
	ROW_TYPE _blinks[ROWS];
	uint8_t _row;
	uint16_t _blink_count;
};

#endif /* MULTIPLEXER_HH */

