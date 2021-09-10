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
 * Potentiometer value reading example.
 * This program shows usage of FastArduino AnalogInput API.
 * It reads and converts the analog level on a pin and displays it to the UART console.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO, but not NANO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on Arduino NANO:
 *   - A7: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on Arduino LEONARDO:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on ATtinyX4 based boards:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATtinyX5 based boards:
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATmega644 based boards:
 *   - A7 (PA7): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D25 (PD1): TX connected to a Serial2USB converter
 */

#include <fastarduino/time.h>
#include <fastarduino/analog_input.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined (ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A7;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#elif defined (BREADBOARD_ATTINYX5)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PB1;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::AnalogPin POT = board::AnalogPin::A7;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using ANALOG_INPUT = analog::AnalogInput<POT, uint16_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_200KHz>;

using streams::endl;

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);

	auto out = uart.out();
	
	// Declare Analog input
	ANALOG_INPUT pot;
	analog::PowerVoltage<> power;

	out << "Prescaler: " << ANALOG_INPUT::PRESCALER << endl;
	
	// Loop of samplings
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		out << value << endl;
		out << power.sample() << "u" << endl;
		out << power.voltage_mV() << " mV" << endl;
		time::delay_ms(1000);
	}
	return 0;
}
