#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UARX_ISR(0)

// Buffers for UARX
static const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

using INPUT = streams::istream;

int main()
{
    board::init();
    sei();
	
    // Start UART
    serial::hard::UARX<board::USART::USART0> uarx{input_buffer};
    uarx.register_handler();
    uarx.begin(115200);
    INPUT in = uarx.in();

    // Wait for a char
    char value1;
    in >> streams::skipws >> value1;

    // Wait for an uint16_t
    uint16_t value2;
    in >> streams::skipws >> value2;

    return 0;
}
