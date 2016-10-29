/*
 * Software UART test sample.
 */

#include <fastarduino/WinBond.hh>
#include <util/delay.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#include <fastarduino/uart.hh>

constexpr const Board::DigitalPin CS = Board::DigitalPin::D7;
static const uint8_t INPUT_BUFFER_SIZE = 16;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 256;

// Define vectors we need in the example
USE_UART0();
#elif defined (ARDUINO_MEGA)
#include <fastarduino/uart.hh>
//TODO
#elif defined (BREADBOARD_ATTINYX4)
#include <fastarduino/softuart.hh>

constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::DigitalPin CS = Board::DigitalPin::D7;
static const uint8_t INPUT_BUFFER_SIZE = 0;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const size_t DATA_SIZE = 128;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

static uint8_t data[DATA_SIZE];

constexpr const uint32_t PAGE = 0x010000;

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();

	// Start UART
#if defined (BREADBOARD_ATTINYX4)
	Soft::UATX<TX> uart{output_buffer};
	uart.begin(115200);
#else
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.begin(115200);
#endif
	FormattedOutput<OutputBuffer> out = uart.fout();
	
	out << "Started\n";

	SPI::SPIDevice::init();
	WinBond flash{CS};
	_delay_ms(1000);
	
	out << "S: " << hex << flash.status().value << endl << flush;
	uint64_t id = flash.read_unique_ID();
	out << "UID: " << hex << uint16_t(id >> 48) << ' ' << uint16_t(id >> 32) << ' ' 
					<< uint16_t(id >> 16) << ' ' << uint16_t(id) << endl;
	WinBond::Device device = flash.read_device();
	out << "M ID: " << hex << device.manufacturer_ID << endl;
	out << "D ID: " << hex << device.device_ID << endl << flush;

	out << "B4 RD 1 pg, S: " << hex << flash.status().value << endl << flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "Af RD, S: " << hex << flash.status().value << endl << flush;

	out << "Pg RD:" << endl << flush;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << endl << flush;
	}
	out << endl << flush;
	
	out << "B4 erase, S: " << hex << flash.status().value << endl << flush;
	flash.enable_write();
	out << "Af enable WR, S: " << hex << flash.status().value << endl << flush;
	flash.erase_sector(PAGE);
	out << "Af erase, S: " << hex << flash.status().value << endl << flush;

	flash.wait_until_ready(10);
	out << "Af wait, S: " << hex << flash.status().value << endl << flush;

	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = uint8_t(i);

	out << "B4 WR, S: " << hex << flash.status().value << endl << flush;
	flash.enable_write();
	flash.write_page(PAGE, data, sizeof data);
	out << "Af WR, S: " << hex << flash.status().value << endl << flush;
	
	flash.wait_until_ready(10);
	out << "Af wait, S: " << hex << flash.status().value << endl << flush;
	
	for (uint16_t i = 0; i < sizeof data; ++i)
		data[i] = 0;
	
	out << "B4 RD 1 byte, S: " << hex << flash.status().value << endl << flush;
	uint8_t value = flash.read_data(PAGE + sizeof(data) / 2);
	out << "RD " << value << ", S: " << hex << flash.status().value << endl << flush;
	
	out << "B4 RD 1 pg, S: " << hex << flash.status().value << endl << flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "Af RD, S: " << hex << flash.status().value << endl << flush;

	out << "Pg RD:" << endl << flush;
	for (uint16_t i = 0; i < sizeof data; ++i)
	{
		out << hex << data[i] << ' ';
		if ((i + 1) % 16 == 0)
			out << endl << flush;
	}

	out << "\nFinished\n" << flush;
}
