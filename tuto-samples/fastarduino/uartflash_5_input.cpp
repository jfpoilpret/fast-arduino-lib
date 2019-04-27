#include <fastarduino/uart.h>

static constexpr const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

REGISTER_UARX_ISR(0)

int main()
{
    board::init();
    sei();
	
    serial::hard::UARX<board::USART::USART0> uart{input_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::istream in = uart.in();

    // Wait until a character is ready and get it
    char value;
    in.get(value);

    // Wait until a complete string is ready and get it
    char str[64+1];
	in.get(str, 64+1);

    return 0;
}
