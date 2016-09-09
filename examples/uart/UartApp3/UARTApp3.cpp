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
	Soft::UART<RX, TX> uart{input_buffer, output_buffer};
	PCI<uart.PCIPORT> pci{&uart};
	pci.enable();

	// Start UART
	// Uncomment the line with the configuration you want to test
//	uart.begin(pci, 9600);
	uart.begin(pci, 115200);
//	uart.begin(pci, 230400);
//	uart.begin(pci, 230400, Serial::Parity::NONE, Serial::StopBits::TWO);
//	uart.begin(pci, 230400, Serial::Parity::EVEN, Serial::StopBits::TWO);
//	uart.begin(pci, 230400, Serial::Parity::EVEN);
//	uart.begin(pci, 115200, Serial::Parity::ODD);
//	uart.begin(pci, 115200, Serial::Parity::EVEN);

	InputBuffer& in = uart.in();
	FormattedOutput<OutputBuffer> out = uart.fout();

	while (true)
	{
		int value = in.get();
		if (value != InputBuffer::EOF)
			out.put(value);
		if (uart.has_errors())
		{
			out.put(' ');
			out.put(uart.frame_error() ?  'F' : '-');
			out.put(uart.data_overrun() ?  'O' : '-');
			out.put(uart.parity_error() ?  'P' : '-');
			out.put(uart.queue_overflow() ?  'Q' : '-');
			out.put('\n');
			uart.clear_errors();
		}
		_delay_ms(10.0);
	}
}
