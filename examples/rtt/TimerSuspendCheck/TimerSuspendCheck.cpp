//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * This program verifies new Timer API suspend_timer/resumer_timer just displaying
 * ticks upon various conditions.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - standard USB connected to console for display
 * - on Arduino MEGA:
 *   - standard USB connected to console for display
 * - on Arduino LEONARDO:
 *   - standard USB connected to console for display
 * - on ATtinyX4 based boards:
 *   - D1 (PA1, TX): connected through USB Serial converter to console for display
 */

#include <fastarduino/flash.h>
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>
#include <fastarduino/tests/assertions.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HAS_UART 1
#define UART_NUM 0
static constexpr const board::USART UART = board::USART::USART0;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#elif defined (ARDUINO_MEGA)
#define HAS_UART 1
#define UART_NUM 0
static constexpr const board::USART UART = board::USART::USART0;
#define TIMER_NUM 4
static constexpr const board::Timer NTIMER = board::Timer::TIMER4;
#elif defined (ARDUINO_LEONARDO)
#define HAS_UART 1
#define UART_NUM 1
static constexpr const board::USART UART = board::USART::USART1;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#elif defined (BREADBOARD_ATTINYX4)
#define HAS_UART 0
static constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#else
#error "Current target is not yet supported!"
#endif

#if HAS_UART
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(UART_NUM)
#else
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 32;
#include <fastarduino/soft_uart.h>
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Timer stuff
using TIMER = timer::Timer<NTIMER>;
using TYPE = TIMER::TYPE;
// Try to get 100us per tick
static constexpr const uint32_t PRECISION = 100UL;
using CALC = timer::Calculator<NTIMER>;
static constexpr const TIMER::PRESCALER PRESCALER = CALC::tick_prescaler(PRECISION);
static constexpr TYPE TICKS_PER_MS = CALC::us_to_ticks(PRESCALER, 1000UL);
static_assert(TICKS_PER_MS >= 10, "TICKS_PER_MS >= 10");

using namespace tests;
using streams::dec;
using streams::endl;

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	// Start UART
#if HAS_UART
	serial::hard::UATX<UART> uatx{output_buffer};
#else
	serial::soft::UATX<TX> uatx{output_buffer};
#endif
	uatx.begin(115200);

	streams::ostream out = uatx.out();
	out.width(0);
	out << F("Started") << endl;
	
	TIMER timer{timer::TimerMode::NORMAL, PRESCALER};
	timer.begin();

	// Initial situation: check ticks go on and interrupts occur
	out << F("Check normal timer...") << endl;
	timer.reset();
	TYPE start = timer.ticks();
	time::delay_us(1000U);
	TYPE end = timer.ticks();
	out << F("Ticks in 1ms = ") << dec << (end - start) << endl;
	assert_true(out, F("Normal timer: (end - start) >= TICKS_PER_MS"), (end - start) >= TICKS_PER_MS);

	// Specific situation: suspend timer, check ticks stop, resume timer
	out << F("Check suspended timer...") << endl;
	timer.reset();
	timer.suspend_timer();
	start = timer.ticks();
	time::delay_us(1000U);
	end = timer.ticks();
	out << F("Ticks in 1ms (expected 0) = ") << dec << (end - start) << endl;
	assert_equals(out, F("Suspended timer: (end - start)"), (end - start), 0U);

	// Check resume works
	out << F("Check resume timer...") << endl;
	timer.reset();
	timer.resume_timer();
	start = timer.ticks();
	time::delay_us(1000U);
	end = timer.ticks();
	out << F("Ticks in 1ms = ") << dec << (end - start) << endl;
	assert_true(out, F("Resumed timer: (end - start) >= TICKS_PER_MS"), (end - start) >= TICKS_PER_MS);
}
