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

#include <fastarduino/FastIO.hh>

#if defined(ARDUINO_UNO)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D4;
#elif defined (BREADBOARD_ATTINYX4)
#else
#error "Current target is not yet supported!"
#endif

static constexpr const uint8_t ROWS = 8;

//TODO Class for SIPO handling
template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, typename T = uint8_t>
class SIPO
{
public:
	SIPO():_clock{PinMode::OUTPUT, false}, _latch{PinMode::OUTPUT, true}, _data{PinMode::OUTPUT, false} {}
	void output(T data)
	{
		uint8_t num_bits = sizeof(T) * 8;
		T mask = 1 << (sizeof(T) * 8 - 1);
		_latch.clear();
		do
		{
			if (data & mask) _data.set(); else _data.clear();
			_clock.set();
			mask >>= 1;
			_clock.clear();
		}
		while (--num_bits);
		_latch.set();
	}
	
private:
	typename FastPinType<CLOCK>::TYPE _clock;
	typename FastPinType<LATCH>::TYPE _latch;
	typename FastPinType<DATA>::TYPE _data;
};

union univ_uint16_t
{
	univ_uint16_t(uint16_t input = 0): as_uint16_t(input) {}
	univ_uint16_t(uint8_t high, uint8_t low): as_uint16_t((high << 8) | low) {}
	
	uint16_t as_uint16_t;
	uint8_t as_uint8_t[2];
};

int main()
{
	// Enable interrupts at startup time
	sei();
	// Initialize SIPO handler
	SIPO<CLOCK, LATCH, DATA, uint16_t> sipo;
	
	// Loop to light every LED for one second
	while (true)
	{
		for (uint8_t row = 0; row < 8; ++row)
		{
			for (uint8_t col = 0; col < 8; ++col)
			{
//				sipo.output(univ_uint16_t(_BV(col) ^ 0xFF, _BV(row)).as_uint16_t);
				sipo.output(univ_uint16_t(_BV(row), _BV(col) ^ 0xFF).as_uint16_t);
				_delay_ms(1000.0);
			}
		}
	}
	return 0;
}
