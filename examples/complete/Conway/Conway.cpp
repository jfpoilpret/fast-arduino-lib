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

#include "Multiplexer.hh"

#include "TestData.hh"

#if defined(ARDUINO_UNO)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D4;

static constexpr const Board::DigitalPin PREVIOUS = Board::DigitalPin::D5;
static constexpr const Board::DigitalPin NEXT = Board::DigitalPin::D6;
static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D7;

#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D2;
#else
#error "Current target is not yet supported!"
#endif

// Single port used by this circuit
static constexpr const Board::Port PORT = FastPinType<CLOCK>::PORT;

// Check at compile time that all pins are on the same port
static_assert(FastPinType<LATCH>::PORT == PORT, "LATCH must be on same port as CLOCK");
static_assert(FastPinType<DATA>::PORT == PORT, "DATA must be on same port as CLOCK");
static_assert(FastPinType<PREVIOUS>::PORT == PORT, "PREVIOUS must be on same port as CLOCK");
static_assert(FastPinType<NEXT>::PORT == PORT, "NEXT must be on same port as CLOCK");
static_assert(FastPinType<SELECT>::PORT == PORT, "SELECT must be on same port as CLOCK");

// Timing constants
// Multiplexing is done one row every 2ms, ie 8 rows in 16ms
static constexpr const uint16_t REFRESH_PERIOD_MS = 2;
// Blinking LEDs are toggled every 20 times the display is fully refreshed (ie 20 x 8 x 2ms = 320ms)
static constexpr const uint8_t BLINKING_COUNTER = 20;

static constexpr const uint16_t PROGRESS_PERIOD_MS = 2000;
static constexpr const uint16_t PROGRESS_COUNTER = PROGRESS_PERIOD_MS / REFRESH_PERIOD_MS;

// Useful constants and types
using MULTIPLEXER = Matrix8x8Multiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER>;
static constexpr const uint8_t ROWS = MULTIPLEXER::ROWS;
static constexpr const uint8_t COLUMNS = MULTIPLEXER::COLUMNS;

// Calculate direction of pins (3 output, 4 input with pullups)
static constexpr const uint8_t ALL_DDR = MULTIPLEXER::DDR_MASK;
static constexpr const uint8_t ALL_PORT =	MULTIPLEXER::PORT_MASK |
											FastPinType<PREVIOUS>::MASK |
											FastPinType<NEXT>::MASK |
											FastPinType<SELECT>::MASK;

//TODO Make it a template based on game size (num rows, num columns)
//TODO Make a class to hold one generation and access its members?
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
		uint8_t count = (game_row & _BV(col)) ? 1 : 0;
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

// OPEN POINTS/TODO
// - Implement initial board setup (with 3 buttons at first: previous, next, select/unselect)
// - Improve (use templates) to allow larger matrix size (eg 16x8, 16x16)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Initialize all pins (only one port)
	FastPort<PORT>{ALL_DDR, ALL_PORT};
	
	// Initialize Multiplexer (blinking at first)
	MULTIPLEXER mux{true};
	
	// Step #1: Initialize board with 1st generation
	//===============================================
	//TODO 
	for (uint8_t i = 0; i < MULTIPLEXER::ROWS; ++i)
		mux.data()[i] = data[i];
	
	// Step #2: Start game
	//=====================
	// Initialize game board
	GameOfLife game{mux.data()};
	mux.blinking(false);
	
	// Loop to light every LED for one second
	uint16_t progress_counter = 0;
	while (true)
	{
		mux.refresh();
		Time::delay_ms(REFRESH_PERIOD_MS);
		if (++progress_counter == PROGRESS_COUNTER)
		{
			game.progress_game();
			progress_counter = 0;
			//TODO Check if game is finished (ie no more live cell)
		}
	}
	return 0;
}
