/*
 * Software UART test sample.
 */

#include <fastarduino/uart.hh>
#include <fastarduino/WinBond.hh>
#include <util/delay.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin CS = Board::DigitalPin::D7;
#elif defined (ARDUINO_MEGA)
#elif defined (BREADBOARD_ATTINYX4)
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
USE_UART0();

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 16;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

static uint8_t data[256];

static const uint32_t PAGE = 0x010000;

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();

	// Start UART
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.begin(115200);
	FormattedOutput<OutputBuffer> out = uart.fout();
	
	out << "main starting...\n";

	SPI::SPIDevice::init();
	WinBond flash{CS};
	_delay_ms(1000);
	
	out << "status: " << hex << flash.status().value << endl << flush;
	uint64_t id = flash.read_unique_ID();
	out << "Unique ID: " << hex << uint16_t(id >> 48) << " " << uint16_t(id >> 32) << " " 
								<< uint16_t(id >> 16) << " " << uint16_t(id) << endl;
	WinBond::Device device = flash.read_device();
	out << "Manufacturer ID: " << hex << device.manufacturer_ID << endl;
	out << "Device ID: " << hex << device.device_ID << endl << flush;

	out << "Before read 1 page, status: " << hex << flash.status().value << endl << flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "After read, status: " << hex << flash.status().value << endl << flush;

	out << "Page read content:" << endl << flush;
	for (uint16_t i = 0; i < 256; ++i)
	{
		out << hex << data[i] << " ";
		if ((i + 1) % 16 == 0)
			out << endl << flush;
	}
	out << endl << flush;
	
	out << "Before erase, status: " << hex << flash.status().value << endl << flush;
	flash.enable_write();
	out << "After enable write, status: " << hex << flash.status().value << endl << flush;
	flash.erase_sector(PAGE);
	out << "After erase, status: " << hex << flash.status().value << endl << flush;

	flash.wait_until_ready(10);
	out << "After wait, status: " << hex << flash.status().value << endl << flush;

	for (uint16_t i = 0; i < 256; ++i)
		data[i] = uint8_t(i);

	out << "Before write, status: " << hex << flash.status().value << endl << flush;
	flash.enable_write();
	flash.write_page(PAGE, data, sizeof data);
	out << "After write, status: " << hex << flash.status().value << endl << flush;
	
	flash.wait_until_ready(10);
	out << "After wait, status: " << hex << flash.status().value << endl << flush;
	
	for (uint16_t i = 0; i < 256; ++i)
		data[i] = 0;
	
	out << "Before read 1 byte, status: " << hex << flash.status().value << endl << flush;
	uint8_t value = flash.read_data(PAGE + 128);
	out << "Read " << value << ", status: " << hex << flash.status().value << endl << flush;
	
	out << "Before read 1 page, status: " << hex << flash.status().value << endl << flush;
	flash.read_data(PAGE, data, sizeof data);
	out << "After read, status: " << hex << flash.status().value << endl << flush;

	out << "Page read content:" << endl << flush;
	for (uint16_t i = 0; i < 256; ++i)
	{
		out << hex << data[i] << " ";
		if ((i + 1) % 16 == 0)
			out << endl << flush;
	}

	out << endl;
	out << "Finished\n";
	out.flush();
}
