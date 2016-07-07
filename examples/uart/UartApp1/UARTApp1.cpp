/*
 * Hardware UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/uart.hh>

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

//FIXME WHY doesn't compiler find out template parameters alone????
//static AbstractUART uart = AbstractUART::create<INPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE>(
//	Board::USART::USART_0, input_buffer, output_buffer);

int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	AbstractUART uart = AbstractUART::create<INPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE>(
		Board::USART::USART_0, input_buffer, output_buffer);
	uart.begin(230400);
	InputBuffer& in = uart.in();
	OutputBuffer& out = uart.out();

	// Event Loop
	while (true)
	{
		out.puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
		out.flush();
		_delay_ms(1000);
	}
}
