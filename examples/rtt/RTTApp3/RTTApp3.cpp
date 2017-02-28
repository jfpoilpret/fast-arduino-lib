//   Copyright 2016-2017 Jean-Francois Poilpret
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
 * Real Time Timer example. Take #3
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support, with microsecond precision.
 * It traces elapsed microseconds after a hard-coded microsecond delay.
 * 
 * Wiring:
 * - on Arduino UNO and ATmega328P based boards:
 *   - use TX pin to send traces to (115200 bps)
 * - on Arduino MEGA:
 *   - NOT SUPPORTED YET
 *   - use TX pin to send traces to (115200 bps)
 * - on ATtinyX4 based boards:
 *   - D1 (PA1) as TX to a Serial-USB converter
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/realtime_timer.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include <fastarduino/uart.h>
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.h>
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/soft_uart.h>
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main()
{
	// Enable interrupts at startup time
	sei();
	// Start UART
#if defined (BREADBOARD_ATTINYX4)
	serial::soft::UATX<TX> uatx{output_buffer};
	uatx.begin(115200);
#else
	serial::UATX<board::USART::USART0> uatx{output_buffer};
	uatx.register_handler();
	uatx.begin(115200);
#endif

	streams::FormattedOutput<streams::OutputBuffer> out = uatx.fout();
	out << "Started\n";
	
	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();

	rtt.begin();
	// Event Loop
	while (true)
	{
		rtt.millis(0);
		_delay_us(666);
		time::RTTTime time = rtt.time();
		out << time.millis << "ms " << time.micros << "us\n" << streams::flush;
	}
}
