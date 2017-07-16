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
 * Blocking sonar sensor read and conversion.
 * This program shows usage of FastArduino HCSR04 device API.
 * 
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 * - on Arduino MEGA:
 * - on ATtinyX4 based boards:
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/devices/hcsr04.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 0
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PD3;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 0
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PE4;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PE5;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 0
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD1;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PD0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 0
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D10_PB2;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<TIMER>;
using PROXIM = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO>;
using devices::sonar::echo_us_to_distance_mm;

// Register all needed ISR
REGISTER_RTT_ISR(TIMER_NUM)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
	uart.register_handler();
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	auto out = uart.fout();
	
	RTT rtt;
	rtt.register_rtt_handler();
	rtt.begin();
	PROXIM sensor{rtt, false};
	
	out << F("Starting...\n") << streams::flush;
	
	while (true)
	{
		uint16_t pulse = sensor.echo_us();
		uint32_t timing = rtt.millis();
		uint16_t mm = echo_us_to_distance_mm(pulse);
		// trace value to output
		out << F("Pulse: ") << pulse  << F(" us. Distance: ") << mm << F(" mm (duration = ") << timing << F(" ms)\n") << streams::flush;
		time::delay_ms(1000);
	}
}
