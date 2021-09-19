//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Asynchronous sonar (PCINT pin) sensor read and conversion.
 * Note that, although this example uses a Sonar in asynchronous mode, it actually
 * blocks until echo is received before displaying the measures.
 * This program shows usage of FastArduino HCSR04 device API.
 * 
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino MEGA:
 *   - D2: connected to sonar trigger pin
 *   - D10: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino LEONARDO:
 *   - D2: connected to sonar trigger pin
 *   - D8: connected to sonar echo pin
 *   - Standard USB connected to console for measures display
 * - on ATtinyX4 based boards:
 *   - D9 (PB1): connected to sonar trigger pin
 *   - D10 (PB2): connected to sonar echo pin
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - D0 (PA0): connected to sonar trigger pin
 *   - D1 (PA1): connected to sonar echo pin
 *   - D25 (PD1): TX output connected to SerialUSB allowing traces display on a PC terminal
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/pci.h>
#include <fastarduino/devices/sonar.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 0
#define PCI_NUM 2
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::InterruptPin ECHO = board::InterruptPin::D3_PD3_PCI2;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
#define PCI_NUM 0
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PE4;
static constexpr const board::InterruptPin ECHO = board::InterruptPin::D10_PB4_PCI0;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 1
#define PCI_NUM 0
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD1;
static constexpr const board::InterruptPin ECHO = board::InterruptPin::D8_PB4_PCI0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 1
#define PCI_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D9_PB1;
static constexpr const board::InterruptPin ECHO = board::InterruptPin::D10_PB2_PCI1;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 0
#define PCI_NUM 0
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D0_PA0;
static constexpr const board::InterruptPin ECHO = board::InterruptPin::D1_PA1_PCI0;
#else
#error "Current target is not yet supported!"
#endif

REGISTER_HCSR04_PCI_ISR(NTIMER, PCI_NUM, TRIGGER, ECHO)
REGISTER_RTT_ISR(TIMER_NUM)

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<NTIMER>;
using SONAR = devices::sonar::ASYNC_PCINT_HCSR04<NTIMER, TRIGGER, ECHO>;
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
	
	interrupt::PCI_SIGNAL<ECHO> signal;
	signal.enable_pin<ECHO>();
	signal.enable();

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
