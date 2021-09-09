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
 * Asynchronous sonar (EXT pin) sensor read and conversion.
 * When a sonar range is obtained, a callback is notified and switches a LED on 
 * if the distance is less than 150mm.
 * 
 * Wiring: 
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - D13: LED connected to GND through 330 Ohm resistor
 *   - Standard USB connected to console for measures display
 * - on Arduino MEGA:
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - D13: LED connected to GND through 330 Ohm resistor
 *   - Standard USB connected to console for measures display
 * - on Arduino LEONARDO:
 *   - D2: connected to sonar trigger pin
 *   - D3: connected to sonar echo pin
 *   - D13: LED connected to GND through 330 Ohm resistor
 *   - Standard USB connected to console for measures display
 * - on ATtinyX4 based boards:
 *   - D9 (PB1): connected to sonar trigger pin
 *   - D10 (PB2): connected to sonar echo pin
 *   - D7 (PA7): LED connected to GND through 330 Ohm resistor
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - D26 (PD2): connected to sonar trigger pin
 *   - D27 (PD3): connected to sonar echo pin
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
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
#define INT_NUM 1
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::ExternalInterruptPin ECHO = board::ExternalInterruptPin::D3_PD3_EXT1;
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define INT_NUM 5
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PE4;
static constexpr const board::ExternalInterruptPin ECHO = board::ExternalInterruptPin::D3_PE5_EXT5;
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(1)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define INT_NUM 0
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD1;
static constexpr const board::ExternalInterruptPin ECHO = board::ExternalInterruptPin::D3_PD0_EXT0;
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define INT_NUM 0
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D9_PB1;
static constexpr const board::ExternalInterruptPin ECHO = board::ExternalInterruptPin::D10_PB2_EXT0;
#elif defined (BREADBOARD_ATMEGA644P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define INT_NUM 1
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D26_PD2;
static constexpr const board::ExternalInterruptPin ECHO = board::ExternalInterruptPin::D27_PD3_EXT1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<NTIMER>;

using SONAR = devices::sonar::ASYNC_INT_HCSR04<NTIMER, TRIGGER, ECHO>;
static constexpr const uint16_t TIMEOUT = SONAR::DEFAULT_TIMEOUT_MS;

using devices::sonar::echo_us_to_distance_mm;
using devices::sonar::distance_mm_to_echo_us;

static constexpr const uint16_t DISTANCE_THRESHOLD_MM = 150;

class SonarListener
{
public:
	SonarListener(const SONAR& sonar, uint16_t min_mm)
	:	MIN_US{distance_mm_to_echo_us(min_mm)}, 
		sonar_{sonar},
		led_{gpio::PinMode::OUTPUT}
	{
		interrupt::register_handler(*this);
	}
	
	void on_sonar()
	{
		uint16_t latest_echo_us = sonar_.latest_echo_us();
		if (latest_echo_us && latest_echo_us <= MIN_US)
			led_.set();
		else
			led_.clear();
	}
	
private:
	const uint16_t MIN_US;
	const SONAR& sonar_;
	gpio::FAST_PIN<board::DigitalPin::LED> led_;
};

REGISTER_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, SonarListener, &SonarListener::on_sonar, SONAR)
REGISTER_HCSR04_INT_ISR_METHOD(NTIMER, INT_NUM, TRIGGER, ECHO, SonarListener, &SonarListener::on_sonar)

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

	SonarListener listener{sonar, DISTANCE_THRESHOLD_MM};

	typename interrupt::INTSignal<ECHO> signal;
	signal.enable();
	
	out << F("Starting...") << streams::endl;
	
	while (true)
	{
		sonar.async_echo(TIMEOUT);
		uint16_t us = sonar.await_echo_us(TIMEOUT);
		uint16_t mm = echo_us_to_distance_mm(us);
		// trace value to output
		out << F("Time: ") << us << F("us. Distance: ") << mm << F("mm") << streams::endl;
		time::delay_ms(1000);
	}
}
