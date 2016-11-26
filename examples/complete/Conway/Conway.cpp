/*
 * Conway's Game of Life implementation with less than 1KB code.
 * Prototype is developed first with Arduino then ported to ATtiny84A.
 * 
 * Description:
 * - cells are displayed on an 8x8 LED matrix
 * - initial setup is set through 2 pots (X and Y) and one button to select/unselect a cell
 * - starting/suspending the game is done by a second push button
 * - a 3rd pot allows speed tuning
 * 
 * Circuit:
 * - MCU is connected to 2 chained 74HC595 SIPO
 * - First SIPO is connected to matrix columns through 8 330Ohm resistors
 * - Second SIPO is connected to matrix rows
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D2 is an output connected to both SIPO clock pins
 *   - D3 is an output connected to both SIPO latch pins
 *   - D4 is an output connected to first SIPO serial data input
 *   - TODO LATER other pins for ADC pots and 2 push buttons
 * - on ATtinyX4 based boards:
 *   - TODO
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/devices/SIPO.hh>

#if defined(ARDUINO_UNO)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D4;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D2;
#else
#error "Current target is not yet supported!"
#endif

//DO WE NEED THAT?
union univ_uint16_t
{
	univ_uint16_t(uint16_t input = 0): as_uint16_t(input) {}
	univ_uint16_t(uint8_t high, uint8_t low): as_uint16_t((high << 8) | low) {}
	
	uint16_t as_uint16_t;
	uint8_t as_uint8_t[2];
};

//TODO do we need BLINK_COUNT as template argument really?
template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, uint8_t BLINK_COUNT = 2>
class Matrix8x8Multiplexer
{
public:
	static constexpr const uint8_t ROWS = 8;
	static constexpr const uint8_t COLUMNS = 8;
	
	Matrix8x8Multiplexer(uint8_t data[ROWS], uint8_t blinks[ROWS])
		:_data(data), _blinks(blinks), _row(0), _blink_count(0) {}
	
	void refresh()
	{
		uint8_t data = _data[_row];
		if (_blink_count > BLINK_COUNT) data &= _blinks[_row];
		_sipo.output(univ_uint16_t(data, _BV(_row) ^ 0xFF).as_uint16_t);
		if (++_row == ROWS)
		{
			_row = 0;
			if (++_blink_count == 2 * BLINK_COUNT)
				_blink_count = 0;
		}
	}
	
protected:
	SIPO<CLOCK, LATCH, DATA, uint16_t> _sipo;
	uint8_t* _data;
	uint8_t* _blinks;
	uint8_t _row;
	uint8_t _blink_count;
};

static uint8_t data[] =
{
	0B00111100,
	0B01100110,
	0B10000001,
	0B10100101,
	0B10000001,
	0B10011001,
	0B01100110,
	0B00111100
};

// if 0, that means this pixel shall blink, if 1 it shall never blink
// only lit pixels can blink, dark pixels never blink
static uint8_t blinks[] =
{
	0xFF,
	0,
	0xFF,
	0,
	0xFF,
	0,
	0xFF,
	0
};

int main()
{
	// Enable interrupts at startup time
	sei();
	// Initialize Multiplexer
	Matrix8x8Multiplexer<CLOCK, LATCH, DATA, 20> mux{data, blinks};
	
	// Loop to light every LED for one second
	while (true)
	{
		mux.refresh();
		_delay_ms(2.0);
	}
	return 0;
}
