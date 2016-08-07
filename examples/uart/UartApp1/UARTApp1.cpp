/*
 * Hardware UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/uart.hh>

// Define vectors we need in the example
USE_UART0()

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
//	AbstractUART uart{Board::USART::USART0, input_buffer, output_buffer};
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.begin(115200);
//	uart.begin(230400);
	InputBuffer& in = uart.in();
//	FormattedInput<InputBuffer> in = uart.fin();
	FormattedOutput<OutputBuffer> out = uart.fout();

	// Event Loop
	while (true)
	{
		out.puts("Enter a letter: ");
		out.flush();
//		int input = 123;
		int input = in.get();
		out.put(input);
		out.put('\n');
		out << (char) input << " " << dec << input << " " << oct << input << " " << hex << input << " " << bin << input << endl;
		out.flush();
		//FIXME Why does this delay last ~2'30" instead of just 1"?
//		_delay_ms(1000.0);
		// The following shall take 100,000 cycles x 160 = 1 second
		//TODO Add LED traces here to understand what happens really
		// What i might be: a forever occurring interrupt that was not cleared?
		// That seems confirmed by ATmega328P datasheet, p178, $20.6.3, bug seems to be inside 
		// UART_DataRegisterEmpty method that does not clear DRE interrupt...
		for (int i = 0; i < 160; ++i)
			_delay_loop_2(25000);
	}
}
