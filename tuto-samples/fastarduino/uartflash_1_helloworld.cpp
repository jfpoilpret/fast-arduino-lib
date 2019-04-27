#include <fastarduino/uart.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

int main() __attribute__((OS_main));
int main()
{
    board::init();
    sei();
	
    serial::hard::UATX<board::USART::USART0> uart{output_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::ostream out = uart.out();
    out.write("Hello, World!\n");
    out.flush();
    return 0;
}
