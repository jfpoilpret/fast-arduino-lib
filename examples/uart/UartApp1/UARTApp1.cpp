/*
 * Hardware UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay_basic.h>

#include <fastarduino/uart.hh>

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

static inline void delay_millis(uint16_t millis)
{
	const uint16_t LOOP_COUNT = F_CPU / 4L / 1000L;
	for (uint16_t i = 0; i < millis; ++i)
	{
		_delay_loop_2(LOOP_COUNT);
	}
}

int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	AbstractUART uart{Board::USART::USART_0, input_buffer, output_buffer};
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
		char toto[32];
//		in.gets(toto, sizeof toto);
		int input = 125;
//		int input = in.get();
//		while ((input = in.get()) == InputBuffer::EOF)
//			;
		out.put(input);
		out.put('\n');
		out << hex << input << " " << dec << 123 << " " << oct << 123 << " " << hex << 123 << " " << bin << 123 << endl;
//		out.flush();
		//FIXME Why does it seem to take 20-40 times longer than expected (namely more than 20s instead of 1s)
		// Measured time 1'25" !!!
//		delay_millis(1000);
	}
}
