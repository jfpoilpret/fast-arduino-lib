#include <fastarduino/soft_uart.h>

constexpr const board::DigitalPin TX = board::DigitalPin::D1_PD1;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

int main()
{
    board::init();
    sei();

    serial::soft::UATX<TX> uart{output_buffer};
    uart.begin(115200);

    ostream out = uart.out();
    uint16_t value = 0x8000;
    out << F("value = 0x") << hex << value 
        << F(", ") << dec << value 
        << F(", 0") << oct << value 
        << F(", B") << bin << value << endl;
    return 0;
}
