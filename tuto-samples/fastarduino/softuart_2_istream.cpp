#include <fastarduino/soft_uart.h>

constexpr const board::DigitalPin RX = board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 0

REGISTER_UART_PCI_ISR(RX, PCI_NUM)

// Buffers for UARX
static const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

int main()
{
    board::init();
    sei();
	
    // Start UART
	serial::soft::UARX<RX> uarx{input_buffer};
	uarx.register_rx_handler();
	typename interrupt::PCIType<RX>::TYPE pci;
	pci.enable();
	uarx.begin(pci, 115200);

    streams::istream in = uarx.in();

    // Wait for a char
    char value1;
    in >> streams::skipws >> value1;

    // Wait for an uint16_t
    uint16_t value2;
    in >> streams::skipws >> value2;

    return 0;
}
