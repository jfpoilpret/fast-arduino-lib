/*
 * Software UART example. Take #1
 * This program demonstrates usage of FastArduino Software (emulated) UART support and formatted output streams.
 * In this example, UATX and UARX are used individually.
 * Serial errors are traced as they occur.
 * 
 * It can be modified and recompiled in order to check various serial configurations:
 * - speed (tested up to 115200 bps)
 * - parity (non, odd or even)
 * - stop bits (1 or 2)
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - Use standard TX/RX but without hardware UART
 * - on Arduino MEGA:
 *   - TODO
 * - on ATmega328P based boards:
 *   - Use standard TX/RX but without hardware UART, connected to an Serial-USB converter
 * - on ATtinyX4 based boards:
 *   - Use D1-D0 as TX-RX, connected to an Serial-USB converter
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/softuart.hh>
#include <fastarduino/utilities.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::DigitalPin RX = Board::DigitalPin::D0;
// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, 2)
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D52;
constexpr const Board::DigitalPin RX = Board::DigitalPin::PCI0;
// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, 0)
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::DigitalPin RX = Board::DigitalPin::D0;
// Define vectors we need in the example
REGISTER_UART_PCI_ISR(RX, 0)
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main()
{
	// Enable interrupts at startup time
	sei();
	
	// Setup UART
	Soft::UATX<TX> uatx{output_buffer};
	Soft::UARX<RX> uarx{input_buffer};
	uarx.register_rx_handler();
	typename PCIType<RX>::TYPE pci;
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
