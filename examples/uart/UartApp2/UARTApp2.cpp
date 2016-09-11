/*
 * Software UART test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/softuart.hh>
#include <fastarduino/utilities.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::InterruptPin RX = Board::InterruptPin::PCI0;
// Define vectors we need in the example
USE_PCI2()
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D52;
constexpr const Board::InterruptPin RX = Board::InterruptPin::PCI0;
// Define vectors we need in the example
USE_PCI0()
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::InterruptPin RX = Board::InterruptPin::PCI0;
// Define vectors we need in the example
USE_PCI0()
#else
#error "Current target is not yet supported!"
#endif


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
	
	// Setup UART
	Soft::UATX<TX> uatx{output_buffer};
	Soft::UARX<RX> uarx{input_buffer};
	PCI<uarx.PCIPORT> pci{&uarx};
	pci.enable();

	// Start UART
	// Following configurations have been tested successfully
//	uatx.begin(9600);
//	uarx.begin(pci, 9600);

	uatx.begin(115200);
	uarx.begin(pci, 115200);
	
//	uatx.begin(230400);
//	uarx.begin(pci, 230400);
	
//	uatx.begin(230400, Serial::Parity::NONE, Serial::StopBits::TWO);
//	uarx.begin(pci, 230400, Serial::Parity::NONE, Serial::StopBits::TWO);
	
//	uatx.begin(230400, Serial::Parity::EVEN, Serial::StopBits::TWO);
//	uarx.begin(pci, 230400, Serial::Parity::EVEN, Serial::StopBits::TWO);
	
//	uatx.begin(230400, Serial::Parity::EVEN);
//	uarx.begin(pci, 230400, Serial::Parity::EVEN);
	
//	uatx.begin(115200, Serial::Parity::ODD);
//	uarx.begin(pci, 115200, Serial::Parity::ODD);
	
//	uatx.begin(115200, Serial::Parity::EVEN, Serial::StopBits::TWO);
//	uarx.begin(pci, 115200, Serial::Parity::EVEN, Serial::StopBits::TWO);

	InputBuffer& in = uarx.in();
	FormattedOutput<OutputBuffer> out = uatx.fout();

	while (true)
	{
		int value = in.get();
		if (value != InputBuffer::EOF)
			out.put(value);
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
		_delay_ms(10.0);
	}
}
