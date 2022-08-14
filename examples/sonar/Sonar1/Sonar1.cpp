//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino MEGA:
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino LEONARDO:
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on ATtinyX4 based boards:
 *   - D9 (PB1): connected to sonar trigger pin
 *   - D10 (PB2): connected to sonar echo pin
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - D2 (PA2): connected to sonar trigger pin
 *   - D3 (PA3): connected to sonar echo pin
 *   - D25 (PD1): TX output connected to SerialUSB allowing traces display on a PC terminal
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/devices/sonar.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PD3;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PE4;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PE5;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD1;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PD0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D9_PB1;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D10_PB2;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PA2;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PA3;
#else
#error "Current target is not yet supported!"
#endif

#if HARDWARE_UART
	REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
	REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#endif

REGISTER_RTT_ISR(TIMER_NUM)

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<NTIMER>;
using SONAR = devices::sonar::HCSR04<NTIMER, TRIGGER, ECHO, devices::sonar::SonarType::BLOCKING>;
static constexpr const uint16_t TIMEOUT = SONAR::DEFAULT_TIMEOUT_MS;

using devices::sonar::echo_us_to_distance_mm;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	auto out = uart.out();

	// Start RTT & sonar
	RTT rtt;
	SONAR sonar{rtt};
	rtt.begin();
	
	out << F("Starting...") << streams::endl;
	
	while (true)
	{
		uint16_t us = sonar.echo_us(TIMEOUT);
		uint16_t mm = echo_us_to_distance_mm(us);
		// trace value to output
		out << F("Pulse: ") << us << F("us. Distance: ") << mm << F("mm") << streams::endl;
		time::delay_ms(1000);
	}
}
