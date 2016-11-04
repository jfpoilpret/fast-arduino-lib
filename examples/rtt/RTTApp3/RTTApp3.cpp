/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/RTT.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include <fastarduino/uart.hh>
#include <util/delay.h>
static const uint8_t INPUT_BUFFER_SIZE = 16;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;

// Define vectors we need in the example
USE_RTT_TIMER0()
USE_UART0();
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.hh>
//TODO
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/softuart.hh>
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
static const uint8_t INPUT_BUFFER_SIZE = 0;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
//TODO
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main()
{
	// Enable interrupts at startup time
	sei();
	// Start UART
#if defined (BREADBOARD_ATTINYX4)
	Soft::UATX<TX> uart{output_buffer};
	uart.begin(115200);
#else
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.begin(115200);
#endif

	FormattedOutput<OutputBuffer> out = uart.fout();
	out << "Started\n";
	
	RTT<Board::Timer::TIMER0> rtt;

	rtt.begin();
	// Event Loop
	while (true)
	{
		rtt.millis(0);
		_delay_us(666);
		RTTTime time = rtt.time();
		out << time.millis << "ms " << time.micros << "us\n" << flush;
	}
}
