/*
 * Software UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/softuart.hh>
#include <fastarduino/utilities.hh>

// Define vectors we need in the example
USE_PCI2()

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	// Start UART
	Soft::UATX<Board::DigitalPin::D1> uatx{output_buffer};
	Soft::UARX<Board::InterruptPin::PCI0> uarx{input_buffer};
	// Following configurations have been tested successfully
//	uatx.begin(9600);
//	uatx.begin(115200);
//	uatx.begin(230400);
//	uatx.begin(230400, Serial::Parity::NONE, Serial::StopBits::TWO);
	uatx.begin(115200, Serial::Parity::EVEN);
//	uatx.begin(115200, Serial::Parity::ODD);
	PCI<uarx.PCIPORT> pci{&uarx};
	pci.enable();
//	uarx.begin(pci, 9600);
//	uarx.begin(pci, 115200);
	uarx.begin(pci, 115200, Serial::Parity::EVEN);
//	uarx.begin(pci, 230400);

	InputBuffer& in = uarx.in();
//	FormattedInput<InputBuffer> in = uarx.fin();
	FormattedOutput<OutputBuffer> out = uatx.fout();

	// Event Loop
//	while (true)
//	{
//		char line[INPUT_BUFFER_SIZE];
//		if (::gets(in, line, INPUT_BUFFER_SIZE, '\n'))
//			out.puts(line);
////		_delay_ms(100.0);
//	}
	
	while (true)
	{
		int value = in.get();
		if (value != InputBuffer::EOF)
			out.put(value);
//		out.put(::get(in));
		if (uarx.has_errors())
		{
			out.put(' ');
			out.put(uarx.frame_error() ?  'F' : '-');
			out.put(uarx.data_overrun() ?  'O' : '-');
			out.put(uarx.parity_error() ?  'P' : '-');
			out.put(uarx.queue_overflow() ?  'Q' : '-');
			out.put('\n');
			uarx.clear_errors();
		}
		_delay_ms(1.0);
	}
}
