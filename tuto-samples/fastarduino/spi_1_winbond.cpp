#include <fastarduino/devices/winbond.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>

static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)

using namespace streams;

constexpr const board::DigitalPin CS = board::DigitalPin::D7_PD7;
constexpr const size_t DATA_SIZE = 256;
static uint8_t data[DATA_SIZE];
constexpr const uint32_t PAGE = 0x010000;

int main()
{
	board::init();
	sei();

	serial::hard::UATX<UART> uart{output_buffer};
	uart.register_handler();
	uart.begin(115200);
	ostream out = uart.out();

	// Initialize SPI and device
	spi::init();
	devices::WinBond<CS> flash;
	time::delay_ms(1000);	
	out << F("S: ") << hex << flash.status().value << endl;

	// Read and display one page of flash memory
	flash.read_data(PAGE, data, sizeof data);
	out << F("RD, S: ") << hex << flash.status().value << endl;
	out << F("Pg RD:") << endl;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0) out << endl;
	}
	out << endl;
	
	// Erase one page of flash memory before writing
	flash.enable_write();
	flash.erase_sector(PAGE);
	out << F("Erase, S: ") << hex << flash.status().value << endl;
	flash.wait_until_ready(10);
	out << F("Wait, S: ") << hex << flash.status().value << endl;

	// Write one page of flash memory
	for (uint16_t i = 0; i < sizeof data; ++i) data[i] = uint8_t(i);
	flash.enable_write();
	flash.write_page(PAGE, data, (DATA_SIZE >= 256 ? 0 : DATA_SIZE));
	out << F("Write, S: ") << hex << flash.status().value << endl;
	flash.wait_until_ready(10);
	out << F("Wait, S: ") << hex << flash.status().value << endl;
	
	// Read back and display page of flash memory just written
	for (uint16_t i = 0; i < sizeof data; ++i) data[i] = 0;
	flash.read_data(PAGE, data, sizeof data);
	out << F("Read, S: ") << hex << flash.status().value << endl;
	out << F("Pg RD:") << endl;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0) out << endl;
	}
	out << endl;
}
