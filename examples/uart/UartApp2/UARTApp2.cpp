/*
 * Software UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/softuart.hh>

// Define vectors we need in the example
//TODO LATER WHEN WE IMPLEMENT UART

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	Soft::UAT<Board::DigitalPin::D1> uart{output_buffer};
	// Following configurations have been tested successfully
//	uart.begin(115200);
//	uart.begin(9600);
//	uart.begin(230400);
	uart.begin(230400, Soft::AbstractUAT::Parity::NONE, Soft::AbstractUAT::StopBits::TWO);
//	uart.begin(115200, Soft::AbstractUAT::Parity::EVEN);
//	uart.begin(115200, Soft::AbstractUAT::Parity::ODD);
	_delay_ms(1000.0);
//	InputBuffer& in = uart.in();
//	FormattedInput<InputBuffer> in = uart.fin();
	FormattedOutput<OutputBuffer> out = uart.fout();

	// Event Loop
	while (true)
	{
		out.puts("Jean-Francois Poilpret @ Renens (Switzerland)\n");
		_delay_ms(10000.0);
//		for (char c = 'A'; c <= 'Z'; ++c)
//		{
//			out.put(c);
////			out.put('A');
////			out.puts("Enter a letter: ");
////			out.flush();
////			int input = 123;
////			int input = in.get();
////			out.put(input);
////			out.put('\n');
////			out << (char) input << ' ' << dec << input << ' ' << oct << input << ' ' << hex << input << ' ' << bin << input << endl;
////			out.flush();
//			_delay_ms(10000.0);
//		}
	}
}
