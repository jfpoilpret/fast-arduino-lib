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
 *   - D0 is an input connected to the START/STOP button (connected itself to GND)
 *   - D7 is an input connected to the SELECT button (connected itself to GND)
 *   - A0 is an analog input connected to the ROW potentiometer
 *   - A1 is an analog input connected to the COLUMN potentiometer
 * - on ATtinyX4 based boards:
 *   - PAx is an output connected to both SIPO clock pins
 *   - PAx is an output connected to both SIPO latch pins
 *   - PA0 is an output connected to first SIPO serial data input
 *   - PA5 is an input connected to the START/STOP button (connected itself to GND)
 *   - PA4 is an input connected to the SELECT button (connected itself to GND)
 *   - A6 is an analog input connected to the ROW potentiometer
 *   - A7 is an analog input connected to the COLUMN potentiometer
 */

//TODO 2. Reuse left pot (row selection) to determine speed of game
//TODO 3. Optimize if needed (several leads: remove vectors, use GPIOR for neighbours, use bit-parallel calculation...)
//TODO 4. Update ATtiny84 pins for actual project boards (no more prototype)

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/time.hh>
#include <fastarduino/AnalogInput.hh>

#include "Multiplexer.hh"
#include "Button.hh"
#include "Game.hh"

#if defined(ARDUINO_UNO)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D4;

static constexpr const Board::AnalogPin ROW = Board::AnalogPin::A0;
static constexpr const Board::AnalogPin COLUMN = Board::AnalogPin::A1;

static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D7;
static constexpr const Board::DigitalPin START_STOP = Board::DigitalPin::D0;

#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D2;

static constexpr const Board::AnalogPin ROW = Board::AnalogPin::A3;
static constexpr const Board::AnalogPin COLUMN = Board::AnalogPin::A4;

static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D5;
static constexpr const Board::DigitalPin START_STOP = Board::DigitalPin::D6;
//TODO Find real PAx for latch and clock
//static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::DX;
//static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::DX;
//static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D0;
//
//static constexpr const Board::AnalogPin ROW = Board::AnalogPin::A7;
//static constexpr const Board::AnalogPin COLUMN = Board::AnalogPin::A6;
//
//static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D4;
//static constexpr const Board::DigitalPin START_STOP = Board::DigitalPin::D5;
#else
#error "Current target is not yet supported!"
#endif

// Single port used by this circuit
static constexpr const Board::Port PORT = FastPinType<CLOCK>::PORT;

// Check at compile time that all pins are on the same port
static_assert(FastPinType<LATCH>::PORT == PORT, "LATCH must be on same port as CLOCK");
static_assert(FastPinType<DATA>::PORT == PORT, "DATA must be on same port as CLOCK");
static_assert(FastPinType<SELECT>::PORT == PORT, "SELECT must be on same port as CLOCK");
static_assert(FastPinType<START_STOP>::PORT == PORT, "START_STOP must be on same port as CLOCK");

// Timing constants
// Multiplexing is done one row every 2ms, ie 8 rows in 16ms
static constexpr const uint16_t REFRESH_PERIOD_US = 1000;
// Blinking LEDs are toggled every 20 times the display is fully refreshed (ie 20 x 8 x 2ms = 320ms)
static constexpr const uint16_t BLINKING_HALF_TIME_MS = 250;
static constexpr const uint16_t BLINKING_COUNTER = BLINKING_HALF_TIME_MS * 1000UL / REFRESH_PERIOD_US;
// Buttons debouncing is done on a duration of 20ms
static constexpr const uint16_t DEBOUNCE_TIME_MS = 20;
static constexpr const uint8_t DEBOUNCE_COUNTER = DEBOUNCE_TIME_MS * 1000UL / REFRESH_PERIOD_US;

static constexpr const uint16_t PROGRESS_PERIOD_MS = 2000;
static constexpr const uint16_t PROGRESS_COUNTER = PROGRESS_PERIOD_MS * 1000UL / REFRESH_PERIOD_US;

// Useful constants and types
using MULTIPLEXER = Matrix8x8Multiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER>;
static constexpr const uint8_t ROWS = MULTIPLEXER::ROWS;
static constexpr const uint8_t COLUMNS = MULTIPLEXER::COLUMNS;
using GAME = GameOfLife<ROWS, COLUMNS>;

// Calculate direction of pins (3 output, 2 input with pullups)
static constexpr const uint8_t ALL_DDR = MULTIPLEXER::DDR_MASK;
static constexpr const uint8_t BUTTONS_MASK = FastPinType<SELECT>::MASK | FastPinType<START_STOP>::MASK;
static constexpr const uint8_t ALL_PORT = MULTIPLEXER::PORT_MASK | BUTTONS_MASK;

static constexpr const uint8_t SMILEY[] =
{
	0B01100110,
	0B10011001,
	0B10000001,
	0B10000001,
	0B01000010,
	0B00100100,
	0B00011000,
	0B00000000
};

// OPEN POINTS/TODO
// - Improve (use templates) to allow larger matrix size (eg 16x8, 16x16)
// - Cleanify code with 2 functions, 1 setup, 1 game?
int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Initialize all pins (only one port)
	FastPort<PORT>{ALL_DDR, ALL_PORT};
	
	// Initialize Multiplexer
	MULTIPLEXER mux;
	
	Button<START_STOP, DEBOUNCE_COUNTER> stop;
	// Step #1: Initialize board with 1st generation
	//===============================================
	{
		Button<SELECT, DEBOUNCE_COUNTER> select;
		AnalogInput<ROW, Board::AnalogReference::AVCC, uint8_t> row_input;
		AnalogInput<COLUMN, Board::AnalogReference::AVCC, uint8_t> column_input;
		uint8_t row = 0;
		uint8_t col = 0;
		mux.blinks()[0] = _BV(0);
		while (true)
		{
			// Update selected cell
			mux.blinks()[row] = 0;
			row = row_input.sample() >> 5;
			col = column_input.sample() >> 5;
			mux.blinks()[row] = _BV(col);
			// Check button states
			if (stop.unique_press())
				break;
			if (select.unique_press())
				mux.data()[row] ^= _BV(col);
			mux.refresh(BlinkMode::BLINK_ALL_BLINKS);
			Time::delay_us(REFRESH_PERIOD_US);
		}
	}
	
	// Step #2: Start game
	//=====================
	{
		// Initialize game board
		GAME game{mux.data()};

		// Loop to refresh LED matrix and progress game to next generation
		uint16_t progress_counter = 0;
		bool pause = false;
		while (true)
		{
			mux.refresh(BlinkMode::NO_BLINK);
			Time::delay_us(REFRESH_PERIOD_US);
			if (stop.unique_press())
				pause = !pause;
			if (!pause && ++progress_counter == PROGRESS_COUNTER)
			{
				game.progress_game();
				progress_counter = 0;
				// Check if game is finished (ie no more live cell, or still life)
				if (game.is_empty())
				{
					// Load a smiley into the game
					for (uint8_t i = 0; i < sizeof(SMILEY)/sizeof(SMILEY[0]); ++i)
						mux.data()[i] = SMILEY[i];
					break;
				}
				if (game.is_still())
					break;
			}
		}
	}
	
	// Step #3: End game
	//===================
	// Here we just need to refresh content and blink it until reset
	Time::delay_ms(1000);
	while (true)
	{
		Time::delay_us(REFRESH_PERIOD_US);
		mux.refresh(BlinkMode::BLINK_ALL_DATA);
	}
	return 0;
}
