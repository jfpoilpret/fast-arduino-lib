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

#include <fastarduino/time.hh>
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

static constexpr const uint16_t REFRESH_PERIOD_MS = 2;
static constexpr const uint16_t PROGRESS_PERIOD_MS = 2000;
static constexpr const uint16_t PROGRESS_COUNTER = PROGRESS_PERIOD_MS / REFRESH_PERIOD_MS;
static constexpr const uint8_t BLINKING_COUNTER = 20;

static constexpr const uint8_t ROWS = 8;
static constexpr const uint8_t COLUMNS = 8;

//TODO Make it a template based on game size (num rows, num columns)
class GameOfLife
{
public:
	GameOfLife(uint8_t game[ROWS]):_current_generation(game) {}
	
	void progress_game()
	{
		uint8_t next_generation[ROWS];
		for (uint8_t row = 0; row < ROWS; ++row)
			for (uint8_t col = 0; col < COLUMNS; ++col)
			{
				uint8_t count_neighbours = neighbours(row, col);
				switch (count_neighbours)
				{
					case 3:
					// cell is alive
					next_generation[row] |= _BV(col);
					break;

					case 4:
					// cell state is kept
					if (_current_generation[row] & _BV(col))
						next_generation[row] |= _BV(col);
					else
						next_generation[row] &= ~_BV(col);
					break;

					default:
					// cell state is dead
					next_generation[row] &= ~_BV(col);
					break;
				}
			}
		// Copy next generation to current one
		for (uint8_t row = 0; row < ROWS; ++row)
			_current_generation[row] = next_generation[row];
	}

private:
	static uint8_t neighbours_in_row(uint8_t game_row, uint8_t col)
	{
		//TODO possibly optimize by:
		// - copy row to GPIOR0
		// - rotate GPIOR (col+1) times
		// check individual bits 0, 1 and 2
		uint8_t count = 0;
		if (game_row & _BV(col)) ++count;
		if (game_row & _BV(col ? col - 1 : COLUMNS - 1)) ++count;
		if (game_row & _BV(col == COLUMNS - 1 ? 0 : col + 1)) ++count;
		return count;
	}
	
	uint8_t neighbours(uint8_t row, uint8_t col)
	{
		uint8_t count = neighbours_in_row(row ? _current_generation[row - 1] : _current_generation[ROWS - 1], col);
		count += neighbours_in_row(row == ROWS - 1 ? _current_generation[0] : _current_generation[row + 1], col);
		count += neighbours_in_row(_current_generation[row], col);
		return count;
	}

	uint8_t* _current_generation;
};



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
		_sipo.output(univ_uint16_t(data, _BV(_row) ^ 0xFF).as_uint16_t);
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

using MULTIPLEXER = Matrix8x8Multiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER>;

//static uint8_t data[] =
//{
//	0B00111100,
//	0B01100110,
//	0B10000001,
//	0B10100101,
//	0B10000001,
//	0B10011001,
//	0B01100110,
//	0B00111100
//};

// Game of Life paterns example
// Oscillator #1 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00010000,
//	0B00010000,
//	0B00010000,
//	0B00000000,
//	0B00000000,
//	0B00000000
//};
// Oscillator #2 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00000000,
//	0B00011100,
//	0B00111000,
//	0B00000000,
//	0B00000000,
//	0B00000000
//};
// Oscillator #3 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00110000,
//	0B00110000,
//	0B00001100,
//	0B00001100,
//	0B00000000,
//	0B00000000
//};
// Still #1 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00000000,
//	0B00011000,
//	0B00100100,
//	0B00011000,
//	0B00000000,
//	0B00000000
//};
// Glider #1 OK
static uint8_t data[] =
{
	0B01000000,
	0B00100000,
	0B11100000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000
};

// OPEN POINTS/TODO
// - Use TIMER vectors or not? For refresh, for game progress or for both?
// - Use INT/PCI Vectors for start/stop and other buttons? Normally not needed
// - Implement initial board setup (with 3 buttons at first: previous, next, select/unselect)
// - Improve (use templates) to allow larger matrix size (eg 16x8, 16x16)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Initialize Multiplexer
	MULTIPLEXER mux;
	for (uint8_t i = 0; i < MULTIPLEXER::ROWS; ++i)
		mux.data()[i] = data[i];
	
	// Initialize game board
	GameOfLife game{mux.data()};

	// Loop to light every LED for one second
	uint16_t progress_counter = 0;
	while (true)
	{
		mux.refresh();
		Time::delay_us(2000);
		if (++progress_counter == PROGRESS_COUNTER)
		{
			game.progress_game();
			progress_counter = 0;
		}
	}
	return 0;
}
