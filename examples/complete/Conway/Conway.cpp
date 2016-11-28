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
#include <fastarduino/Timer.hh>

#include "Multiplexer.hh"

#include "TestData.hh"

#if defined(ARDUINO_UNO)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D4;
static constexpr const Board::Timer TIMER_DISPLAY = Board::Timer::TIMER0;
static constexpr const Board::TimerPrescaler PRESCALER_DISPLAY = Board::TimerPrescaler::DIV_256;
static constexpr const Board::Timer TIMER_PROGRESS = Board::Timer::TIMER1;
static constexpr const Board::TimerPrescaler PRESCALER_PROGRESS = Board::TimerPrescaler::DIV_1024;
USE_TIMER0();
USE_TIMER1();
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D2;
static constexpr const Board::Timer TIMER_DISPLAY = Board::Timer::TIMER0;
static constexpr const Board::TimerPrescaler PRESCALER_DISPLAY = Board::TimerPrescaler::DIV_256;
static constexpr const Board::Timer TIMER_PROGRESS = Board::Timer::TIMER1;
static constexpr const Board::TimerPrescaler PRESCALER_PROGRESS = Board::TimerPrescaler::DIV_1024;
USE_TIMER0();
USE_TIMER1();
#else
#error "Current target is not yet supported!"
#endif

static constexpr const uint16_t REFRESH_PERIOD_MS = 2;
static constexpr const uint16_t PROGRESS_PERIOD_MS = 2000;
static constexpr const uint16_t PROGRESS_COUNTER = PROGRESS_PERIOD_MS / REFRESH_PERIOD_MS;
static constexpr const uint8_t BLINKING_COUNTER = 20;

using MULTIPLEXER = Matrix8x8Multiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER>;

static constexpr const uint8_t ROWS = MULTIPLEXER::ROWS;
static constexpr const uint8_t COLUMNS = MULTIPLEXER::COLUMNS;

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

class DisplayRefresher: public TimerCallback
{
public:
	DisplayRefresher(MULTIPLEXER& mux):_mux(mux) {}
	virtual void on_timer() override
	{
		_mux.refresh();
	}
	
private:
	MULTIPLEXER& _mux; 
};

class GameProgresser: public TimerCallback
{
public:
	GameProgresser(GameOfLife& game): _game(game) {}
	virtual void on_timer() override
	{
		_game.progress_game();
	}
	
private:
	GameOfLife& _game;
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
	
	DisplayRefresher display_refresher{mux};
	Timer<TIMER_DISPLAY> display_timer{display_refresher};
	display_timer.begin(PRESCALER_DISPLAY, F_CPU / 1000 / _BV(uint8_t(PRESCALER_DISPLAY)) * REFRESH_PERIOD_MS - 1);
	
	GameProgresser game_progresser{game};
	Timer<TIMER_PROGRESS> progress_timer{game_progresser};
	progress_timer.begin(PRESCALER_PROGRESS, F_CPU / 1000 / _BV(uint8_t(PRESCALER_PROGRESS)) * PROGRESS_PERIOD_MS - 1);

	// Loop to light every LED for one second
//	uint16_t progress_counter = 0;
	while (true)
	{
//		mux.refresh();
//		Time::delay_ms(PROGRESS_PERIOD_MS);
//		if (++progress_counter == PROGRESS_COUNTER)
//		{
//			game.progress_game();
//			progress_counter = 0;
//		}
	}
	return 0;
}
