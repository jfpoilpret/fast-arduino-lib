/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * It does not do anything interesting as far as hardware is concerned.
 * It is just try-and-throw-away code.
 */

#include <fastarduino/analog_input.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

using namespace streams;

static constexpr const board::DigitalPin INPUT = board::DigitalPin::D2_PD2;
static constexpr const board::AnalogPin AINPUT = board::AnalogPin::A0;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	gpio::FAST_PIN<INPUT> input{gpio::PinMode::INPUT};
	analog::AnalogInput<AINPUT> ainput;

	serial::hard::UATX<UART> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	out << boolalpha;
	out << F("Snippet started") << endl;

	while (true)
	{
		bool val = input.value();
		// uint16_t val = ainput.sample();
		out << F("Input = ") << val << endl;
		time::delay_ms(100);
	}
}
