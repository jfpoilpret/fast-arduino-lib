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
 *   - PA2 is an output connected to both SIPO clock pins
 *   - PA1 is an output connected to both SIPO latch pins
 *   - PA0 is an output connected to first SIPO serial data input
 *   - PA5 is an input connected to the START/STOP button (connected itself to GND)
 *   - PA4 is an input connected to the SELECT button (connected itself to GND)
 *   - A7 is an analog input connected to the ROW potentiometer
 *   - A6 is an analog input connected to the COLUMN potentiometer
 */

//TODO 1. Reuse left pot (row selection) to determine speed of game
//TODO 2. Optimize further if needed (several leads: remove vectors, use GPIOR for neighbours)

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

static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D5;
static constexpr const Board::DigitalPin START_STOP = Board::DigitalPin::D6;

#define HAS_TRACE 1
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin CLOCK = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LATCH = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin DATA = Board::DigitalPin::D0;

static constexpr const Board::AnalogPin ROW = Board::AnalogPin::A6;
static constexpr const Board::AnalogPin COLUMN = Board::AnalogPin::A7;

static constexpr const Board::DigitalPin SELECT = Board::DigitalPin::D4;
static constexpr const Board::DigitalPin START_STOP = Board::DigitalPin::D5;
#else
#error "Current target is not yet supported!"
#endif

// Trace is used only for Arduino UNO if needed
#if HAS_TRACE
#include <fastarduino/uart.hh>
USE_UATX0();

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static UATX<Board::USART::USART0> uatx{output_buffer};
FormattedOutput<OutputBuffer> trace = uatx.fout();
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
// Delay between 2 generations during phase 2
static constexpr const uint16_t PROGRESS_PERIOD_MS = 2000;
static constexpr const uint16_t PROGRESS_COUNTER = PROGRESS_PERIOD_MS * 1000UL / REFRESH_PERIOD_US;
// Delay between phase 2 (game) and phase 3 (end of game)
static constexpr const uint16_t DELAY_BEFORE_END_GAME_MS = 1000;

// Useful constants and types
using MULTIPLEXER = Matrix8x8Multiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER>;
static constexpr const uint8_t ROWS = MULTIPLEXER::ROWS;
static constexpr const uint8_t COLUMNS = MULTIPLEXER::COLUMNS;
using GAME = GameOfLife<ROWS, COLUMNS>;

// Calculate direction of pins (3 output, 2 input with pullups)
static constexpr const uint8_t ALL_DDR = MULTIPLEXER::DDR_MASK;
static constexpr const uint8_t BUTTONS_MASK = FastPinType<SELECT>::MASK | FastPinType<START_STOP>::MASK;
static constexpr const uint8_t ALL_PORT = MULTIPLEXER::PORT_MASK | BUTTONS_MASK;

//NOTE: on the stripboards-based circuit, rows and columns are inverted
static constexpr const uint8_t SMILEY[] =
{
	0B01110000,
	0B10001000,
	0B10000100,
	0B01000010,
	0B01000010,
	0B10000100,
	0B10001000,
	0B01110000
};

// OPEN POINTS/TODO
// - Improve (use templates) to allow larger matrix size (eg 16x8, 16x16)
int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
#if HAS_TRACE
	// Setup traces (Arduino only)
	uatx.begin(57600);
	trace.width(0);
#endif
	
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
			row = 7 - (row_input.sample() >> 5);
			col = 7 - (column_input.sample() >> 5);
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
	// First we clear multiplexer display, then we wait for one second
	mux.clear();
	Time::delay_ms(DELAY_BEFORE_END_GAME_MS);
	while (true)
	{
		Time::delay_us(REFRESH_PERIOD_US);
		mux.refresh(BlinkMode::BLINK_ALL_DATA);
	}
	return 0;
}
