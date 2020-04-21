/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * It does not do anything interesting as far as hardware is concerned.
 * It is just try-and-throw-away code.
 */

#include <util/delay_basic.h>

#include <fastarduino/boards/board.h>
#include <fastarduino/i2c.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// MAIN IDEA:
// - have a queue of "I2C commands" (variable-length)
// - dequeue and execute each command from TWI ISR, call back when last command is finished or error occurred
// - transmission starts aonce a STOP command has been enqueued
// - prepare a Promise/Future concept library to hold results asynchronously (to be further thought of)

// TODO OPEN POINTS:
// - DS1307 seems not work currently (even with synchronous I2C on FastArduino!)
// - implement callback registration (only one callback, can be an event supplier)
// - optimize queue size by allowing multiple read/writes in one command
// - optimize queue size by reducing number of commands: start+slaw/r+send/receive
//		(repeat start or stop should be automatically added)
//		(will need some kind of FSM for handling several stages of a single command)
//		(maybe we could use a "command builder" for that?)
// - should sync and async handler code cohabitate? or use only async but allow devices to await (sync) or not (async) 
//
// - ATtiny support
// - short addresses (1 byte) for some ATtiny devices?

// I2C async specific definitions
//================================

//TODO embed in I2cHandler class!
// Type of commands in queue
// Each command is at least 1 byte (command type), optionally followed by command-specific data
enum class I2CCommand : uint8_t
{
	// No command at all
	NONE = 0,
	// Send start condition on I2C bus: 0 byte
	START,
	// Send repeat start condition on I2C bus: 0 byte
	REPEAT_START,
	// Send device address for reading: 1 byte (device address)
	SLAR,
	// Send device address for writing: 1 byte (device address)
	SLAW,
	// Write data to I2C bus: 1 byte (data to write)
	WDATA,
	// Read data from I2C bus: 2 bytes (pointer to data buffer that will get the read byte)
	RDATA,
	// Read data from I2C bus: 2 bytes (pointer to data buffer that will get the read byte)
	RDATA_LAST,
	// Send stop condition on I2C bus: 0 byte
	STOP
};

// Used by TWI ISR to potentially call a registered callback
enum class I2CCallback : uint8_t
{
	NONE = 0,
	NORMAL_STOP,
	ERROR
};

//TODO DEBUG ONLY, remove after
static constexpr uint8_t STATUS_BUFFER_SIZE = 64;
static uint8_t expected_status[STATUS_BUFFER_SIZE];
static uint8_t actual_status[STATUS_BUFFER_SIZE];
static uint8_t status_index = 0;

// This is an asynchronous I2C handler
template<i2c::I2CMode MODE_> class I2CHandler
{
public:
	static constexpr const i2c::I2CMode MODE = MODE_;

	I2CHandler(const I2CHandler<MODE_>&) = delete;
	I2CHandler<MODE_>& operator=(const I2CHandler<MODE_>&) = delete;

	template<uint8_t SIZE>
	explicit I2CHandler(uint8_t (&buffer)[SIZE]) : commands_{buffer}
	{
		interrupt::register_handler(*this);
	}

	void begin()
	{
		synchronized begin_();
	}
	void end()
	{
		synchronized end_();
	}

	void begin_()
	{
		// 1. set SDA/SCL pullups
		TRAIT::PORT |= TRAIT::SCL_SDA_MASK;
		// 2. set I2C frequency
		TWBR_ = TWBR_VALUE;
		TWSR_ = 0;
		// 3. Enable TWI
		// TWCR_ = bits::BV8(TWEN);
	}
	void end_()
	{
		// 1. Disable TWI
		TWCR_ = 0;
		// 2. remove SDA/SCL pullups
		TRAIT::PORT &= bits::COMPL(TRAIT::SCL_SDA_MASK);
	}

	uint8_t status() const
	{
		return status_;
	}

	//TODO DEBUG ONLY!
	uint8_t control() const
	{
		return TWCR_;
	}

	//TODO is that really useful?
	bool is_ready() const
	{
		return commands_.empty();
	}

	bool check_queue_(uint8_t num_starts, uint8_t num_sends, uint8_t num_receives)
	{
		// - each start (including repeat start) is 1 byte, plus 2 bytes for the following slaw/slar
		// - each send is 2 bytes
		// - each receive is 3 bytes
		// - one stop is mandatory to end the I2C transaction
		return commands_.free_() >= (num_starts * 3 + num_sends * 2 + num_receives * 3 + 1);
	}

	bool start_()
	{
		return push_byte_(uint8_t(I2CCommand::START));
	}
	bool repeat_start_()
	{
		return push_byte_(uint8_t(I2CCommand::REPEAT_START));
	}
	bool send_slar_(uint8_t address)
	{
		return push_byte_(uint8_t(I2CCommand::SLAR)) && push_byte_(address);
	}
	bool send_slaw_(uint8_t address)
	{
		return push_byte_(uint8_t(I2CCommand::SLAW)) && push_byte_(address);
	}
	//TODO shall we handle sending N bytes (from one address)?
	bool send_data_(uint8_t data)
	{
		return push_byte_(uint8_t(I2CCommand::WDATA)) && push_byte_(data);
	}
	//TODO shall we handle receiving N bytes (to one address)?
	bool receive_data_(uint8_t& data, bool last_byte)
	{
		return	push_byte_(uint8_t(last_byte ? I2CCommand::RDATA_LAST : I2CCommand::RDATA))
			&&	push_byte_(utils::high_byte(reinterpret_cast<uint16_t>(&data)))
			&&	push_byte_(utils::low_byte(reinterpret_cast<uint16_t>(&data)));
	}
	bool stop_()
	{
		return push_byte_(uint8_t(I2CCommand::STOP), true);
	}

private:
	using TRAIT = board_traits::TWI_trait;
	using REG8 = board_traits::REG8;

	static constexpr const REG8 TWBR_{TWBR};
	static constexpr const REG8 TWSR_{TWSR};
	static constexpr const REG8 TWCR_{TWCR};
	static constexpr const REG8 TWDR_{TWDR};

	// Push one byte of a command to the queue, and possibly initiate a new transmission right away
	bool push_byte_(uint8_t data, bool finished = false)
	{
		bool ok = commands_.push_(data);
		// Check if need to initiate transmission
		// if (ok && finished && !(TWCR_ & bits::BV8(TWIE)))
		if (ok && finished && current_ == I2CCommand::NONE)
			// Dequeue first pending command and start TWI operation
			dequeue_command_();
		return ok;
	}

	// Dequeue the next command in the queue and process it immediately
	void dequeue_command_()
	{
		uint8_t command = 0;
		if (!commands_.pull_(command))
		{
			// No more I2C command to execute
			current_ = I2CCommand::NONE;
			TWCR_ = bits::BV8(TWINT);
			return;
		}
		current_ = I2CCommand(command);
		// Check command type and read more
		switch (current_)
		{
			case I2CCommand::START:
			exec_start_();
			break;
			
			case I2CCommand::REPEAT_START:
			exec_repeat_start_();
			break;
			
			case I2CCommand::STOP:
			exec_stop_();
			break;
			
			case I2CCommand::SLAR:
			exec_send_slar_();
			break;

			case I2CCommand::SLAW:
			exec_send_slaw_();
			break;

			case I2CCommand::WDATA:
			exec_send_data_();
			break;
			
			case I2CCommand::RDATA:
			exec_receive_data_(false);
			break;
			
			case I2CCommand::RDATA_LAST:
			exec_receive_data_(true);
			break;

			default:
			break;
		}
	}

	// Low-level methods to handle the bus in an asynchronous way
	void exec_start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::START_TRANSMITTED;
	}
	void exec_repeat_start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::REPEAT_START_TRANSMITTED;
	}
	void exec_send_slar_()
	{
		// Read device address from queue
		uint8_t address;
		commands_.pull_(address);
		TWDR_ = address | 0x01U;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_R_TRANSMITTED_ACK;
	}
	void exec_send_slaw_()
	{
		// Read device address from queue
		uint8_t address;
		commands_.pull_(address);
		TWDR_ = address;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_W_TRANSMITTED_ACK;
	}
	void exec_send_data_()
	{
		// Read data from queue
		uint8_t data;
		commands_.pull_(data);
		TWDR_ = data;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::DATA_TRANSMITTED_ACK;
	}
	void exec_receive_data_(bool last_byte)
	{
		// Read buffer address from queue
		uint8_t high_address = 0;
		commands_.pull_(high_address);
		uint8_t low_address = 0;
		commands_.pull_(low_address);
		const uint16_t address = utils::as_uint16_t(high_address, low_address);
		payload_ = reinterpret_cast<uint8_t*>(address);

		// Then a problem occurs for the last byte we want to get, which should have NACK instead!
		// Send ACK for previous data (including SLA-R)
		if (last_byte)
		{
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			expected_status_ = i2c::Status::DATA_RECEIVED_NACK;
		}
		else
		{
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			expected_status_ = i2c::Status::DATA_RECEIVED_ACK;
		}
	}
	void exec_stop_(bool error = false)
	{
		// TWCR_ = bits::BV8(TWINT, TWSTO);
		TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		if (!error)
		{
			expected_status_ = 0;
			stopped_ = true;
		}
		// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
		// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
		//TODO we can reduce delay by accounting for dequeue time!
		_delay_loop_1(DELAY_AFTER_STOP);

		// Check if there is one more command in queue (i.e. new I2C transaction)
		dequeue_command_();
	}

	I2CCallback i2c_change()
	{
		// Check status Vs. expected status
		status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
		actual_status[status_index] = status_;
		expected_status[status_index++] = expected_status_;
		if (status_ != expected_status_)
		{
			// Clear all pending transactions from queue
			//TODO that behavior could be customizable (clear all, or clear only current I2C transaction)
			commands_.clear_();
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			return I2CCallback::ERROR;
		}
		
		// Handle TWI interrupt when data received
		switch (current_)
		{
			case I2CCommand::RDATA:
			case I2CCommand::RDATA_LAST:
			// Push received data to buffer
			*payload_++ = TWDR_;
			break;

			default:
			break;
		}

		// Handle next command if any
		dequeue_command_();
		// If it was a stop then special behavior is required
		if (stopped_)
		{
			// We need to let ISR execute any callbacks!
			stopped_ = false;
			return I2CCallback::NORMAL_STOP;
		}
		return I2CCallback::NONE;
	}

	static constexpr const uint32_t STANDARD_FREQUENCY = (F_CPU / ONE_MHZ - 16UL) / 2;
	static constexpr const uint32_t FAST_FREQUENCY = (F_CPU / 400000UL - 16UL) / 2;
	static constexpr const uint8_t TWBR_VALUE = (MODE == i2c::I2CMode::STANDARD ? STANDARD_FREQUENCY : FAST_FREQUENCY);

	static constexpr const float STANDARD_DELAY_AFTER_STOP_US = 4.0 + 4.7;
	static constexpr const float FAST_DELAY_AFTER_STOP_US = 0.6 + 1.3;
	static constexpr const float DELAY_AFTER_STOP_US =
		(MODE == i2c::I2CMode::STANDARD ? STANDARD_DELAY_AFTER_STOP_US : FAST_DELAY_AFTER_STOP_US);
	static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(DELAY_AFTER_STOP_US);

	containers::Queue<uint8_t, uint8_t> commands_;

	// Status of current command processing
	I2CCommand current_ = I2CCommand::NONE;
	uint8_t expected_status_ = 0;
	bool stopped_ = false;
	// For read command only
	uint8_t* payload_ = nullptr;

	// Latest I2C status
	uint8_t status_ = 0;

	DECL_TWI_FRIENDS
	//FOR DEBUG ONLY!
	friend int main();
};

//TODO Add callbacks registration
#define REGISTER_ASYNC_I2C(MODE)                                            \
ISR(TWI_vect)                                                               \
{                                                                           \
	I2CCallback callback =                                                  \
		interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_change();\
}

// RTC-specific definitions (for testing purpose)
//================================================
class RTC
{
	public:
	RTC(I2CHandler<i2c::I2CMode::STANDARD>& handler) : handler_{handler} {}

	static constexpr uint8_t ram_size()
	{
		return RAM_SIZE;
	}

	bool set_ram(uint8_t address, uint8_t data)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return false;
		synchronized return		handler_.check_queue_(1, 2, 0)
							&&	handler_.start_()
							&&	handler_.send_slaw_(DEVICE_ADDRESS)
							&&	handler_.send_data_(address)
							&&	handler_.send_data_(data)
							&&	handler_.stop_();
	}

	bool get_ram(uint8_t address, uint8_t& data)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return false;
		synchronized return		handler_.check_queue_(2, 1, 1)
							&&	handler_.start_()
							&&	handler_.send_slaw_(DEVICE_ADDRESS)
							&&	handler_.send_data_(address)
							&&	handler_.repeat_start_()
							&&	handler_.send_slar_(DEVICE_ADDRESS)
							&&	handler_.receive_data_(data, true)
							&&	handler_.stop_();
	}

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;

	I2CHandler<i2c::I2CMode::STANDARD>& handler_;
};

// Actual test example
//=====================
using I2CHANDLER = I2CHandler<i2c::I2CMode::STANDARD>;

REGISTER_ASYNC_I2C(i2c::I2CMode::STANDARD)

static constexpr uint8_t I2C_BUFFER_SIZE = 128;
static uint8_t i2c_buffer[I2C_BUFFER_SIZE];

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

void display_status(ostream& out)
{
	out << F("Status history") << endl;
	out << F("expected  actual") << endl;
	for (uint8_t i = 0; i != status_index; ++i)
		out << showbase << hex << right << setw(8) << expected_status[i]
			<< F("  ") << right << setw(6) << actual_status[i] << endl;
	out << endl;
	status_index = 0;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();

	// Initialize I2C asyn handler
	I2CHANDLER handler{i2c_buffer};
	RTC rtc{handler};
	out << F("Before handler.begin()") << endl;
	out << boolalpha;

	handler.begin();
	time::delay_ms(500);

	//TODO Test 1: write one byte, read one byte, delay
	//TODO Test 2: add ready() method in I2CHandler
	//TODO Test 3: add simple callback (function or method, in charge of dispatching)
	// while (true)
	{
		constexpr uint8_t RAM_SIZE = rtc.ram_size();
	
		out << F("TEST #1 write and read RAM bytes, one by one") << endl;
		// Write all RAM bytes
		uint8_t i = 1;
		// for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			uint8_t data = 0;
			bool ok1 = rtc.set_ram(i, i + 1);
			uint8_t status1 = handler.status();
			uint8_t control1 = handler.control();
			uint8_t expected1 = handler.expected_status_;
			uint8_t items1 = handler.commands_.items();
			bool ok2 = rtc.get_ram(i, data);
			uint8_t status2 = handler.status();
			uint8_t control2 = handler.control();
			uint8_t expected2 = handler.expected_status_;
			uint8_t items2 = handler.commands_.items();
			out << F("set_ram(") << dec << i << F(") => ") << ok1 << F(", status = 0x") << hex << status1
				<< F(", expected = 0x") << expected1 << F(", control = 0x") << control1
				<< F(", items = ") << dec << items1 << endl;
			out << F("get_ram(") << dec << i << F(") => ") << ok2 << F(", status = 0x") << hex << status2
				<< F(", expected = 0x") << expected2 << F(", control = 0x") << control2
				<< F(", items = ") << dec << items2 << endl;
			out << F("get_ram() data = ") << dec << data << endl;
			time::delay_ms(10);
			out << F("get_ram() after 10ms data = ") << dec << data << endl;

			display_status(out);

		}
		time::delay_ms(1000);
		//TODO Read all RAM bytes
	}
}
