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
#include <fastarduino/uart.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/flash.h>
#include <fastarduino/pci.h>
#include <fastarduino/devices/hcsr04.h>

static constexpr const board::DigitalPin TRIGGER1 = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO1 = board::InterruptPin::D3_PD3_PCI2;
static constexpr const board::DigitalPin TRIGGER2 = board::DigitalPin::D4_PD4;
static constexpr const board::DigitalPin ECHO2 = board::InterruptPin::D5_PD5_PCI2;
static constexpr const board::Timer TIMER = board::Timer::TIMER2;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<TIMER>;
using PROXIM1 = devices::sonar::HCSR04<TIMER, TRIGGER1, ECHO1>;
using PROXIM2 = devices::sonar::HCSR04<TIMER, TRIGGER2, ECHO2>;
using devices::sonar::echo_us_to_distance_mm;

// Register all needed ISR
REGISTER_RTT_ISR(2)
REGISTER_UATX_ISR(0)
//TODO Improve HCSR04 to directly provide API for using several servos on same PCINT vector
ISR(PCINT2_vect)
{
	CALL_HANDLER_(PROXIM1, &PROXIM1::on_echo)();
	CALL_HANDLER_(PROXIM2, &PROXIM2::on_echo)();
}

int main() __attribute__((OS_main));
int main()
{
	sei();
	
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.register_handler();
	uart.begin(115200);
	auto out = uart.fout();
	
	RTT rtt;
	rtt.register_rtt_handler();
	rtt.begin();
	typename interrupt::PCIType<ECHO1>::TYPE signal;
	//TODO replace with only one call to enable_pins()
	signal.enable_pin<ECHO1>();
	signal.enable_pin<ECHO2>();
	signal.enable();
	PROXIM1 sensor1{rtt};
	PROXIM2 sensor2{rtt};
	
	out << F("Starting...\n") << streams::flush;
	
	while (true)
	{
		sensor1.async_echo();
		sensor2.async_echo();
		uint16_t pulse1 = sensor1.await_echo_us();
		uint16_t mm1 = echo_us_to_distance_mm(pulse1);
		uint16_t pulse2 = sensor2.await_echo_us();
		uint16_t mm2 = echo_us_to_distance_mm(pulse2);
		// trace value to output
		out << F("Pulse1: ") << pulse1  << F(" us. Distance: ") << mm1 << F(" mm\n") << streams::flush;
		out << F("Pulse2: ") << pulse2  << F(" us. Distance: ") << mm2 << F(" mm\n") << streams::flush;
		time::delay_ms(1000);
	}
}
