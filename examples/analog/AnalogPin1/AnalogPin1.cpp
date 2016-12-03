/*
 * Potentiometer value reading example.
 * This program shows usage of FastArduino AnalogInput API.
 * TODO Further description
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - TODO
 * - on Arduino MEGA:
 *   - TODO
 * - on ATtinyX4 based boards:
 *   - TODO
 */

#include <avr/interrupt.h>
#include <fastarduino/time.hh>
#include <fastarduino/AnalogInput.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include <fastarduino/uart.hh>
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
USE_UART0();
#elif defined (ARDUINO_MEGA)
//TODO
#elif defined (BREADBOARD_ATTINYX4)
//TODO
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
	UATX<Board::USART::USART0> uart{output_buffer};
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
		//TODO display on UART (HW or SW))
		Time::delay_ms(1000);
	}
	return 0;
}
