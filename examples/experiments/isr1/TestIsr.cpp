/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();
// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// DS1307 specific
const uint8_t DEVICE_ADDRESS = 0x68 << 1;
struct BCD
{
	struct
	{
		uint8_t high	:4;
		uint8_t low		:4;
	};
	uint8_t data;
};
struct RealTime
{
	BCD seconds;
	BCD minutes;
	BCD hours;
	uint8_t weekday;
	BCD day;
	BCD month;
	BCD	year;
};

//TODO define constants for expected results

const uint32_t I2C_FREQUENCY = 100000;

// NOTE we use prescaler = 1 everywhere
constexpr uint8_t calculate_TWBR(uint32_t frequency)
{
	return (F_CPU / frequency - 16UL) / 2;
}

static bool wait_twi(uint8_t expected_status)
{
	out << "W " << streams::hex << expected_status << " #1" << streams::flush;
	loop_until_bit_is_set(TWCR, TWINT);
	out << " #2" << streams::flush;
	if ((TWSR & 0xF8) == expected_status)
	{
		out << "- " << streams::dec << streams::flush;
		return true;
	}
	//TODO ERROR handling and trace
	out << " X " << streams::hex << TWSR << streams::dec << ' ' << streams::flush;
	return false;
}

// TODO traits for: SDA, SCL pins
int main() __attribute__((OS_main));
int main()
{
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(0);
	out << "Start\n" << streams::flush;
	
	// Start TWI interface
	//====================
	// 1. set SDA/SCL pullups (TODO: in specific TRAIT)
	PORTC |= _BV(PORTC4) | _BV(PORTC5);
	// 2. set I2C frequency
	TWBR = calculate_TWBR(I2C_FREQUENCY);
	TWSR = 0;
	// 3. Enable TWI
	TWCR = _BV(TWEN);

	out << "I2C interface started\n" << streams::flush;
	time::delay_ms(1000);
	
	// Send a byte to a slave
	//=======================
	// 1. send START
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
	wait_twi(0x08);
	// 2. send SLA+W
	TWDR = DEVICE_ADDRESS;
	TWCR = _BV(TWEN) | _BV(TWINT);
	wait_twi(0x18);
	// 3. send data
	TWDR = 0;
	TWCR = _BV(TWEN) | _BV(TWINT);
	wait_twi(0x28);
//	// 4. send STOP (normally not needed (or harmful?) with DS1307
//	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);	

	RealTime time;
	// Read a byte from a slave
	//=========================
	// 1. send REPEATING START
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
	wait_twi(0x10);
//	wait_twi(0x08);
	// 2. send SLA+R
	TWDR = DEVICE_ADDRESS | 0x01;
	TWCR = _BV(TWEN) | _BV(TWINT);
	wait_twi(0x40);
	// 3. Read data & send ACK
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.seconds.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.minutes.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.hours.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.weekday = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.day.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.month.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	wait_twi(0x50);
	time.year.data = TWDR;
	TWCR = _BV(TWEN) | _BV(TWINT);
	wait_twi(0x58);
	// 4. send STOP
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
	
	out	<< "RTC: " 
		<< time.day.high << time.day.low << '.'
		<< time.month.high << time.month.low << '.'
		<< time.year.high << time.year.low << ' '
		<< time.hours.high << time.hours.low << ':'
		<< time.minutes.high << time.minutes.low << ':'
		<< time.seconds.high << time.seconds.low << '\n'
		<< streams::flush;
	
	// Stop TWI interface
	//===================
	// 1. Disable TWI
	TWCR = 0;
	// 2. remove SDA/SCL pullups
	PORTC &= ~(_BV(PORTC4) | _BV(PORTC5));
	
	out << "End\n" << streams::flush;
}
