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
 * Asynchronous sonar (PCINT pins) sensors read and conversion.
 * This program shows usage of FastArduino HCSR04 device API with PCINT ISR on 2 pins
 * (same PCI port).
 * 2 sonars are attached to the board but their trigger pins are attached to only
 * one MCU pin, hence they are always triggered at the same time. 
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2: connected to sonar 1 & 2 trigger pins
 *   - D3: connected to sonar 1 echo pin
 *   - D5: connected to sonar 3 echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino MEGA:
 *   - D2: connected to sonar 1 & 2 trigger pins
 *   - D53: connected to sonar 1 echo pin
 *   - D52: connected to sonar 2 echo pin
 *   - Standard USB connected to console for measures display
 * - on Arduino LEONARDO:
 *   - D2: connected to sonar 1 & 2 trigger pins
 *   - D8: connected to sonar 1 echo pin
 *   - D9: connected to sonar 2 echo pin
 *   - Standard USB connected to console for measures display
 * - on ATtinyX4 based boards:
 *   - D0 (PA0): connected to sonar 1 & 2 trigger pins
 *   - D10 (PB2): connected to sonar 1 echo pin
 *   - D9 (PB1): connected to sonar 1 echo pin
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - D2 (PA2): connected to sonar 1 & 2 trigger pin
 *   - D3 (PA3): connected to sonar 1 echo pin
 *   - D5 (PA5): connected to sonar 2 echo pin
 *   - D25 (PD1): TX output connected to SerialUSB allowing traces display on a PC terminal
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/pci.h>
#include <fastarduino/devices/sonar.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define PCI_NUM 2
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::InterruptPin ECHO1 = board::InterruptPin::D3_PD3_PCI2;
static constexpr const board::InterruptPin ECHO2 = board::InterruptPin::D5_PD5_PCI2;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define PCI_NUM 0
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PE4;
static constexpr const board::InterruptPin ECHO1 = board::InterruptPin::D53_PB0_PCI0;
static constexpr const board::InterruptPin ECHO2 = board::InterruptPin::D52_PB1_PCI0;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define PCI_NUM 0
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD1;
static constexpr const board::InterruptPin ECHO1 = board::InterruptPin::D8_PB4_PCI0;
static constexpr const board::InterruptPin ECHO2 = board::InterruptPin::D9_PB5_PCI0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define PCI_NUM 1
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D0_PA0;
static constexpr const board::InterruptPin ECHO1 = board::InterruptPin::D10_PB2_PCI1;
static constexpr const board::InterruptPin ECHO2 = board::InterruptPin::D9_PB1_PCI1;
#elif defined (BREADBOARD_ATMEGA644P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define PCI_NUM 0
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PA2;
static constexpr const board::InterruptPin ECHO1 = board::InterruptPin::D3_PA3_PCI0;
static constexpr const board::InterruptPin ECHO2 = board::InterruptPin::D5_PA5_PCI0;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_RTT_ISR(TIMER_NUM)

using RTT = timer::RTT<NTIMER>;

static constexpr const uint8_t ECHO_MASK = 
	gpio::FastPinType<board::PCI_PIN<ECHO1>()>::MASK |
	gpio::FastPinType<board::PCI_PIN<ECHO2>()>::MASK;

using SONAR1 = devices::sonar::ASYNC_PCINT_HCSR04<NTIMER, TRIGGER, ECHO1>;
using SONAR2 = devices::sonar::ASYNC_PCINT_HCSR04<NTIMER, TRIGGER, ECHO2>;
static constexpr const uint16_t TIMEOUT = SONAR1::DEFAULT_TIMEOUT_MS;

using devices::sonar::echo_us_to_distance_mm;

// Register all needed ISR
REGISTER_HCSR04_PCI_ISR(NTIMER, PCI_NUM, TRIGGER, ECHO1, ECHO2)

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
	SONAR1 sonar1{rtt};
	SONAR2 sonar2{rtt};
	rtt.begin();
	
	interrupt::PCI_SIGNAL<ECHO1> signal;
	signal.set_enable_pins(ECHO_MASK);
	signal.enable();
	
	out << F("Starting...") << streams::endl;
	
	while (true)
	{
		// Trigger both sensors in first call
		sonar1.async_echo(true);
		sonar2.async_echo(false);
		// time::delay_ms(25);
		uint16_t us1 = sonar1.await_echo_us(TIMEOUT);
		uint16_t us2 = sonar2.await_echo_us(TIMEOUT);
		uint16_t mm1 = echo_us_to_distance_mm(us1);
		uint16_t mm2 = echo_us_to_distance_mm(us2);
		// trace value to output
		out << F("Time1: ") << us1 << F("us. Distance: ") << mm1 << F("mm") << streams::endl;
		out << F("Time2: ") << us2 << F("us. Distance: ") << mm2 << F("mm") << streams::endl;
		time::delay_ms(1000);
	}
}
