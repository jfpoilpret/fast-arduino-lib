/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <string.h>
#include <fastarduino/eeprom.h>
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UART_ISR(0)
static const board::USART USART = board::USART::USART0;

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

static const uint8_t MAX_LEN = 64;
char wifi_name[MAX_LEN+1] EEMEM = "";
char wifi_password[MAX_LEN+1] EEMEM = "";

using namespace eeprom;
using streams::endl;
using streams::flush;
using streams::noskipws;
using streams::skipws;

// int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UART<USART> uart{input_buffer, output_buffer};
	uart.register_handler();
	uart.begin(115200);

	streams::istream in = uart.in();
	streams::ostream out = uart.out();

	// Get current WIFI name/password from EEPROM
	char wifi[MAX_LEN+1];
	EEPROM::read(wifi_name, wifi, MAX_LEN+1);
	char password[MAX_LEN+1];
	EEPROM::read(wifi_password, password, MAX_LEN+1);

	// If WIFI present check if user wants to keep it
	bool ask = true;
	if (strlen(wifi))
	{
		char answer;
		out << F("Do you want to use WIFI `") << wifi << F("`? [Y/n] :") << flush;
		in >> skipws >> answer;
		in.ignore(0, '\n');
		ask = (toupper(answer) == 'N');
	}

	// Ask for WIKI name & password if needed
	if (ask)
	{
		out << F("Enter WIFI name: ") << flush;
		in.getline(wifi, MAX_LEN+1);
		out << F("Enter WIFI password: ") << flush;
		in.getline(password, MAX_LEN+1);
		// Store to EEPROM
		EEPROM::write(wifi_name, wifi, strlen(wifi) + 1);
		EEPROM::write(wifi_password, password, strlen(password) + 1);
	}

	// Start real program here, using wifi and password
	//...
	out << F("WIFI: ") << wifi << endl;
	out << F("Password: ") << password << endl;
}
