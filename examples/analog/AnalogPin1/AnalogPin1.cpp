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
 * Potentiometer value reading example.
 * This program shows usage of FastArduino AnalogInput API.
 * It reads and converts the analog level on a pin and displays it to the UART console.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on Arduino MEGA:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 * - on ATtinyX4 based boards:
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <avr/interrupt.h>
#include <fastarduino/time.h>
#include <fastarduino/analog_input.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1_PA1;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

//using ANALOG_INPUT = AnalogInput<POT, Board::AnalogReference::AVCC, uint16_t, Board::AnalogClock::MAX_FREQ_200KHz>;
//using ANALOG_INPUT = AnalogInput<POT, Board::AnalogReference::AVCC, uint8_t, Board::AnalogClock::MAX_FREQ_200KHz>;
using ANALOG_INPUT = AnalogInput<POT, Board::AnalogReference::AVCC, uint8_t, Board::AnalogClock::MAX_FREQ_1MHz>;

int main()
{
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	UATX<Board::USART::USART0> uart{output_buffer};
	uart.register_handler();
#else
	Soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);

	FormattedOutput<OutputBuffer> out = uart.fout();
	
	// Declare Analog input
	ANALOG_INPUT pot;
	PowerVoltage<> power;

	out << "Prescaler: " << ANALOG_INPUT::PRESCALER << endl << flush;
	
	// Loop of samplings
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		out << value << " (" << power.voltage_mV() << " mV)\n" << flush;
		Time::delay_ms(1000);
	}
	return 0;
}
