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
 * Asynchronous sonar sensor read and conversion.
 * This program shows usage of FastArduino HCSR04 device API with PCINT ISR om 2 pins.
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
#include <fastarduino/timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/pci.h>
#include <fastarduino/devices/hcsr04.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
#define PCI_NUM 2
static constexpr const board::DigitalPin TRIGGER1 = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO1 = board::InterruptPin::D3_PD3_PCI2;
static constexpr const board::DigitalPin TRIGGER2 = board::DigitalPin::D4_PD4;
static constexpr const board::DigitalPin ECHO2 = board::InterruptPin::D5_PD5_PCI2;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
#define PCI_NUM 0
static constexpr const board::DigitalPin TRIGGER1 = board::DigitalPin::D2_PE4;
static constexpr const board::DigitalPin ECHO1 = board::InterruptPin::D53_PB0_PCI0;
static constexpr const board::DigitalPin TRIGGER2 = board::DigitalPin::D3_PE5;
static constexpr const board::DigitalPin ECHO2 = board::InterruptPin::D52_PB1_PCI0;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
#define PCI_NUM 0
static constexpr const board::DigitalPin TRIGGER1 = board::DigitalPin::D2_PD1;
static constexpr const board::DigitalPin ECHO1 = board::InterruptPin::D8_PB4_PCI0;
static constexpr const board::DigitalPin TRIGGER2 = board::DigitalPin::D3_PD0;
static constexpr const board::DigitalPin ECHO2 = board::InterruptPin::D9_PB5_PCI0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
#define PCI_NUM 1
static constexpr const board::DigitalPin TRIGGER1 = board::DigitalPin::D0_PA0;
static constexpr const board::DigitalPin ECHO1 = board::InterruptPin::D10_PB2_PCI1;
static constexpr const board::DigitalPin TRIGGER2 = board::DigitalPin::D1_PA1;
static constexpr const board::DigitalPin ECHO2 = board::InterruptPin::D9_PB1_PCI1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using TIMER_TYPE = timer::Timer<TIMER>;
using CALC = timer::Calculator<TIMER>;
using devices::sonar::SonarType;
using SONAR1 = devices::sonar::HCSR04<TIMER, TRIGGER1, ECHO1, SonarType::ASYNC_PCINT>;
using SONAR2 = devices::sonar::HCSR04<TIMER, TRIGGER2, ECHO2, SonarType::ASYNC_PCINT>;
static constexpr const uint32_t PRECISION = SONAR1::DEFAULT_TIMEOUT_MS * 1000UL;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::CTC_prescaler(PRECISION);
static constexpr const SONAR1::TYPE TIMEOUT = CALC::us_to_ticks(PRESCALER, PRECISION);

using devices::sonar::echo_us_to_distance_mm;

// Register all needed ISR
REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, SONAR1, SONAR2)

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
	
	TIMER_TYPE timer{timer::TimerMode::NORMAL, PRESCALER};
	timer.begin();
	SONAR1 sonar1{timer};
	sonar1.register_handler();
	SONAR2 sonar2{timer};
	sonar2.register_handler();

	typename interrupt::PCIType<ECHO1>::TYPE signal;
	//TODO replace with only one call to enable_pins()
	signal.enable_pin<ECHO1>();
	signal.enable_pin<ECHO2>();
	signal.enable();
	
	out << F("Starting...\n") << streams::flush;
	
	while (true)
	{
		// When we use one timer with 2 triggers, we cannot operate both at the same time
		sonar1.async_echo();
		SONAR1::TYPE pulse1 = sonar1.await_echo_ticks(TIMEOUT);
		uint32_t us1 = CALC::ticks_to_us(PRESCALER, pulse1);
		uint16_t mm1 = echo_us_to_distance_mm(us1);
		sonar2.async_echo();
		SONAR2::TYPE pulse2 = sonar2.await_echo_ticks(TIMEOUT);
		uint32_t us2 = CALC::ticks_to_us(PRESCALER, pulse2);
		uint16_t mm2 = echo_us_to_distance_mm(us2);
		// trace value to output
		out << F("Pulse1: ") << pulse1 << F(" ticks, ") << us1 << F("us. Distance: ") << mm1 << F("mm\n") << streams::flush;
		out << F("Pulse2: ") << pulse2 << F(" ticks, ") << us2 << F("us. Distance: ") << mm2 << F("mm\n") << streams::flush;
		time::delay_ms(1000);
	}
}
