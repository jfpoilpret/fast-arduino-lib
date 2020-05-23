/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling specificaloly for
 * ATtiny architecture.
 * As a matter of fact, ATtiny USI feature is not very well suited for asynchronous 
 * I2C handling as I2C master (this is easier for slaves).
 * This PoC will try to demonstrate working with DS1307 RTC chip from an ATtiny84 MCU, 
 * using Timer0 as clock source for USI SCL clock.
 */

#ifndef BREADBOARD_ATTINYX4
#error "This PoC is for ATtiny84 exclusively"
#endif

#include <util/delay_basic.h>

#include <fastarduino/boards/board.h>
#include <fastarduino/i2c.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include <fastarduino/soft_uart.h>
#include <fastarduino/iomanip.h>

// #define DEBUG_STEPS
// #define DEBUG_REGISTER_OK
// #define DEBUG_REGISTER_ERR
// #define DEBUG_SEND_OK
// #define DEBUG_SEND_ERR
// #define DEBUG_RECV_OK
// #define DEBUG_RECV_ERR
// #define DEBUG_DATA_RECV
// #define DEBUG_DATA_SEND
#if defined(DEBUG_STEPS) || defined(DEBUG_REGISTER_OK) || defined(DEBUG_REGISTER_ERR) || defined(DEBUG_SEND_OK) || defined(DEBUG_SEND_ERR) || defined(DEBUG_RECV_OK) || defined(DEBUG_RECV_ERR)
#define DEBUG_STATUS
#endif
// Pin used for debugging
constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;

#include "i2c_handler.h"

template<i2c::I2CMode MODE_> class I2CHandler
{
public:
	// Low-level methods to handle the bus, used by friend class I2CDevice
	bool start()
	{
		return send_start(i2c::Status::START_TRANSMITTED);
	}
	bool repeat_start()
	{
		return send_start(i2c::Status::REPEAT_START_TRANSMITTED);
	}
	bool send_slar(uint8_t address)
	{
		return send_byte(address | 0x01U, i2c::Status::SLA_R_TRANSMITTED_ACK, i2c::Status::SLA_R_TRANSMITTED_NACK);
	}
	bool send_slaw(uint8_t address)
	{
		return send_byte(address, i2c::Status::SLA_W_TRANSMITTED_ACK, i2c::Status::SLA_W_TRANSMITTED_NACK);
	}
	bool send_data(uint8_t data)
	{
		return send_byte(data, i2c::Status::DATA_TRANSMITTED_ACK, i2c::Status::DATA_TRANSMITTED_NACK);
	}
	bool receive_data(uint8_t& data, bool last_byte = false)
	{
		SDA_INPUT();
		data = transfer(USISR_DATA);
		// Send ACK (or NACK if last byte)
		USIDR_ = (last_byte ? UINT8_MAX : 0x00);
		uint8_t good_status = (last_byte ? i2c::Status::DATA_RECEIVED_NACK : i2c::Status::DATA_RECEIVED_ACK);
		transfer(USISR_ACK);
		return callback_hook(true, good_status, good_status);
	}
	void stop()
	{
		// Pull SDA low
		SDA_LOW();
		// Release SCL
		SCL_HIGH();
		_delay_loop_1(T_SU_STO);
		// Release SDA
		SDA_HIGH();
		_delay_loop_1(T_BUF);
	}

private:

	bool send_start(uint8_t good_status)
	{
		// Ensure SCL is HIGH
		SCL_HIGH();
		// Wait for Tsu-sta
		_delay_loop_1(T_SU_STA);
		// Now we can generate start condition
		// Force SDA low for Thd-sta
		SDA_LOW();
		_delay_loop_1(T_HD_STA);
		// Pull SCL low
		SCL_LOW();
		//			_delay_loop_1(T_LOW());
		// Release SDA (force high)
		SDA_HIGH();
		//TODO check START transmission with USISIF flag?
		//			return callback_hook(USISR & bits::BV8(USISIF), good_status, i2c::Status::ARBITRATION_LOST);
		return callback_hook(true, good_status, i2c::Status::ARBITRATION_LOST);
	}
	bool send_byte(uint8_t data, uint8_t ACK, uint8_t NACK)
	{
		// Set SCL low TODO is this line really needed for every byte transferred?
		SCL_LOW();
		// Transfer address byte
		USIDR_ = data;
		transfer(USISR_DATA);
		// For acknowledge, first set SDA as input
		SDA_INPUT();
		return callback_hook((transfer(USISR_ACK) & 0x01U) == 0, ACK, NACK);
	}
	//TODO This is the method to change!
	uint8_t transfer(uint8_t USISR_count)
	{
		// Init counter (8 bits or 1 bit for acknowledge)
		USISR_ = USISR_count;
		do
		{
			_delay_loop_1(T_LOW);
			// clock strobe (SCL raising edge)
			USICR_ |= bits::BV8(USITC);
			TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
			_delay_loop_1(T_HIGH);
			// clock strobe (SCL falling edge)
			USICR_ |= bits::BV8(USITC);
		}
		while ((USISR_ & bits::BV8(USIOIF)) == 0);
		_delay_loop_1(T_LOW);
		// Read data
		uint8_t data = USIDR_;
		USIDR_ = UINT8_MAX;
		// Release SDA
		SDA_OUTPUT();
		return data;
	}
	bool callback_hook(bool ok, uint8_t good_status, uint8_t bad_status)
	{
		if (hook_) hook_(good_status, (ok ? good_status : bad_status));
		status_ = (ok ? i2c::Status::OK : bad_status);
		return ok;
	}

	void i2c_overflow()
	{
		//TODO
	}

	// Constant values for USISR
	// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
	static constexpr const uint8_t USISR_DATA = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
	// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
	static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

	// Timing constants for current mode (as per I2C specifications)
	static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.0 : 0.6);
	static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.7 : 1.3);
	static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.0 : 0.6);
	static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.7 : 0.6);
	static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.0 : 0.6);
	static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(MODE == i2c::I2CMode::STANDARD ? 4.7 : 1.3);

	uint8_t status_ = 0;
	const i2c::I2C_STATUS_HOOK hook_;

	DECL_TWI_FRIENDS
};

// I2C async specific definitions
//================================

#define REGISTER_I2C_ISR(MODE)                                              \
ISR(USI_OVF_vect)                                                           \
{                                                                           \
	interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_overflow();  \
}

// Actual test example
//=====================

REGISTER_I2C_ISR(i2c::I2CMode::STANDARD)

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using I2CHANDLER = I2CHandler<i2c::I2CMode::STANDARD>;
using namespace streams;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
	serial::soft::UATX<TX> uatx{output_buffer};

	// Start UART
	uatx.begin(115200);
	ostream out = uatx.out();

	// Initialize I2C async handler
	I2CHANDLER handler{nullptr};
	out << F("Before handler.begin()") << endl;
	out << boolalpha << showbase;

	handler.begin();

	//TODO set one byte
	//TODO get one byte
	
	handler.end();
}
