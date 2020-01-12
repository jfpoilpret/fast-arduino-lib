#include <fastarduino/soft_uart.h>

constexpr const board::InterruptPin RX = board::InterruptPin::D0_PD0_PCI2;
#define PCI_NUM 2

REGISTER_UARX_PCI_ISR(RX, PCI_NUM)

// Buffers for UARX
static const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

int main()
{
    board::init();
    sei();
	
    // Start UART
	interrupt::PCI_SIGNAL<RX> pci;
	serial::soft::UARX_PCI<RX> uarx{input_buffer, pci};
	pci.enable();
	uarx.begin(115200);

    streams::istream in = uarx.in();

    // Wait for a char
    char value1;
    in >> streams::skipws >> value1;

    // Wait for an uint16_t
    uint16_t value2;
    in >> streams::skipws >> value2;

    return 0;
}
