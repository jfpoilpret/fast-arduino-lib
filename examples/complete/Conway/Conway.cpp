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

/*
 * Conway's Game of Life implementation with less than 1KB code.
 * Prototype is developed first with Arduino then ported to ATtiny84A.
 * 
 * Description:
 * - cells are displayed on an 8x8 LED matrix
 * - initial setup is set through 2 pots (X and Y) and one button to select/unselect a cell
 * - starting/suspending the game is done by a second push button
 * - when the game has started, the Y pot allows speed tuning
 * - the end of game is detected when:
 *   - no cells are alive: in this case a smiley is blinking
 *   - the last generation is stable (still life): in this case the last generation is blinking
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

#include <fastarduino/time.h>
#include <fastarduino/analog_input.h>

#include "Multiplexer.hh"
#include "Button.hh"
#include "Game.hh"

#if defined(ARDUINO_UNO)
static constexpr const board::DigitalPin CLOCK = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin LATCH = board::DigitalPin::D3_PD3;
static constexpr const board::DigitalPin DATA = board::DigitalPin::D4_PD4;

static constexpr const board::AnalogPin ROW = board::AnalogPin::A0;
static constexpr const board::AnalogPin COLUMN = board::AnalogPin::A1;
static constexpr const board::AnalogPin SPEED_PIN = board::AnalogPin::A0;

static constexpr const board::DigitalPin SELECT = board::DigitalPin::D5_PD5;
static constexpr const board::DigitalPin START_STOP = board::DigitalPin::D6_PD6;

#define HAS_TRACE 1
#elif defined (ARDUINO_LEONARDO)
static constexpr const board::DigitalPin CLOCK = board::DigitalPin::D0_PD2;
static constexpr const board::DigitalPin LATCH = board::DigitalPin::D1_PD3;
static constexpr const board::DigitalPin DATA = board::DigitalPin::D2_PD1;

static constexpr const board::AnalogPin ROW = board::AnalogPin::A0;
static constexpr const board::AnalogPin COLUMN = board::AnalogPin::A1;
static constexpr const board::AnalogPin SPEED_PIN = board::AnalogPin::A1;

static constexpr const board::DigitalPin SELECT = board::DigitalPin::D3_PD0;
static constexpr const board::DigitalPin START_STOP = board::DigitalPin::D4_PD4;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::DigitalPin CLOCK = board::DigitalPin::D2_PA2;
static constexpr const board::DigitalPin LATCH = board::DigitalPin::D1_PA1;
static constexpr const board::DigitalPin DATA = board::DigitalPin::D0_PA0;

static constexpr const board::AnalogPin ROW = board::AnalogPin::A6;
static constexpr const board::AnalogPin COLUMN = board::AnalogPin::A7;
static constexpr const board::AnalogPin SPEED_PIN = board::AnalogPin::A7;

static constexpr const board::DigitalPin SELECT = board::DigitalPin::D4_PA4;
static constexpr const board::DigitalPin START_STOP = board::DigitalPin::D5_PA5;
#else
#error "Current target is not yet supported!"
#endif

// Trace is used only for Arduino UNO if needed
#if HAS_TRACE
#include <fastarduino/uart.h>
REGISTER_UATX_ISR(0)
// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uatx{output_buffer};
streams::ostream trace = uatx.out();
#endif

// Uncomment these lines if you want to quickly generate a program for a 16x16 LED matrix (default is 8x8))
//#define ROW_COUNT 16
//#define COLUMN_COUNT 16

// Those defines can be overridden in command line to support bigger LED matrices (eg 16x16)
#ifndef ROW_COUNT
#define ROW_COUNT 8
#endif
#ifndef COLUMN_COUNT
#define COLUMN_COUNT 8
#endif

// Single port used by this circuit
static constexpr const board::Port PORT = gpio::FastPinType<CLOCK>::PORT;

// Check at compile time that all pins are on the same port
static_assert(gpio::FastPinType<LATCH>::PORT == PORT, "LATCH must be on same port as CLOCK");
static_assert(gpio::FastPinType<DATA>::PORT == PORT, "DATA must be on same port as CLOCK");
static_assert(gpio::FastPinType<SELECT>::PORT == PORT, "SELECT must be on same port as CLOCK");
static_assert(gpio::FastPinType<START_STOP>::PORT == PORT, "START_STOP must be on same port as CLOCK");

// Utility function to find the exponent of the nearest power of 2 for an integer
constexpr uint16_t LOG2(uint16_t n)
{
	return (n < 2) ? 0 : 1 + LOG2(n / 2);
}

// Timing constants
// Multiplexing is done one row every 1ms, ie 8 rows in 8ms
static constexpr const uint16_t REFRESH_PERIOD_MS = 1;
static constexpr const uint16_t REFRESH_PERIOD_US = 1000 * REFRESH_PERIOD_MS;
// Blinking LEDs are toggled every 250ms
static constexpr const uint16_t BLINKING_HALF_TIME_MS = 250;
static constexpr const uint16_t BLINKING_COUNTER = BLINKING_HALF_TIME_MS / REFRESH_PERIOD_MS;
// Buttons debouncing is done on a duration of 20ms
static constexpr const uint16_t DEBOUNCE_TIME_MS = 20;
static constexpr const uint8_t DEBOUNCE_COUNTER = DEBOUNCE_TIME_MS / REFRESH_PERIOD_MS;
// Minimum delay between 2 generations during phase 2 (must be a power of 2)
static constexpr const uint16_t MIN_PROGRESS_PERIOD_MS = 256;
// Delay between phase 2 (game) and phase 3 (end of game)
static constexpr const uint16_t DELAY_BEFORE_END_GAME_MS = 1000;

// Useful constants and types
static constexpr const uint8_t ROWS = ROW_COUNT;
static constexpr const uint8_t COLUMNS = COLUMN_COUNT;
using MULTIPLEXER = MatrixMultiplexer<CLOCK, LATCH, DATA, BLINKING_COUNTER, ROWS, COLUMNS>;
using ROW_TYPE = MULTIPLEXER::ROW_TYPE;
using GAME = GameOfLife<ROWS, ROW_TYPE>;

// Calculate direction of pins (3 output, 2 input with pullups)
static constexpr const uint8_t ALL_DDR = MULTIPLEXER::DDR_MASK;
static constexpr const uint8_t BUTTONS_MASK = gpio::FastPinType<SELECT>::MASK | gpio::FastPinType<START_STOP>::MASK;
static constexpr const uint8_t ALL_PORT = MULTIPLEXER::PORT_MASK | BUTTONS_MASK;

//NOTE: on the stripboards-based circuit, rows and columns are inverted
static constexpr const ROW_TYPE SMILEY[] =
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

static uint16_t game_period()
{
	analog::AnalogInput<SPEED_PIN, uint8_t, board::AnalogReference::AVCC> speed_input;
	uint8_t period = speed_input.sample() >> 4;
	return (MIN_PROGRESS_PERIOD_MS * (period + 1)) >> LOG2(REFRESH_PERIOD_MS);
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
#if HAS_TRACE
	// Setup traces (Arduino only)
	uatx.begin(57600);
	trace.width(0);
#endif
	
	// Initialize all pins (only one port)
	gpio::FastPort<PORT>{ALL_DDR, ALL_PORT};
	
	// Initialize Multiplexer
	MULTIPLEXER mux;
	
	// STOP button is used during both phase 1 and 2, hence declare it in global main() scope
	Button<START_STOP, DEBOUNCE_COUNTER> stop;
	
	// Step #1: Initialize board with 1st generation
	//===============================================
	{
		Button<SELECT, DEBOUNCE_COUNTER> select;
		analog::AnalogInput<ROW, uint8_t, board::AnalogReference::AVCC> row_input;
		analog::AnalogInput<COLUMN, uint8_t, board::AnalogReference::AVCC> column_input;
		uint8_t row = 0;
		uint8_t col = 0;
		mux.blinks()[0] = BV8(0);
		while (true)
		{
			// Update selected cell
			mux.blinks()[row] = 0;
			row = ROWS - 1 - (row_input.sample() >> (8 - LOG2(ROWS)));
			col = COLUMNS - 1 - (column_input.sample() >> (8 - LOG2(COLUMNS)));
			mux.blinks()[row] = BV8(col);
			// Check button states
			if (stop.unique_press())
				break;
			if (select.unique_press())
				mux.data()[row] ^= BV8(col);
			mux.refresh(BlinkMode::BLINK_ALL_BLINKS);
			time::delay_us(REFRESH_PERIOD_US);
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
			time::delay_us(REFRESH_PERIOD_US);
			if (stop.unique_press())
				pause = !pause;
			if (!pause && ++progress_counter >= game_period())
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
	time::delay_ms(DELAY_BEFORE_END_GAME_MS);
	while (true)
	{
		time::delay_us(REFRESH_PERIOD_US);
		mux.refresh(BlinkMode::BLINK_ALL_DATA);
	}
	return 0;
}

#if defined (BREADBOARD_ATTINYX4)
// Since we use -nostartfiles option we must manually set the startup code (at address 0x00)
void __jumpMain() __attribute__((naked)) __attribute__((section (".init9")));

// Startup code just clears R1 (this is expected by GCC) and calls main()
void __jumpMain()
{    
	asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );
	asm volatile ( "clr __zero_reg__" );	// r1 set to 0
	asm volatile ( "rjmp main");			// jump to main()
}
#endif
