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
// - special command: callback?
// - dequeue and execute each command from TWI ISR, call back when last command is finished 

// TODO OPEN POINTS:
// - when to start transmission? at STOP command request? or at first request? or specific API?
// - how to automatically go ahead after STOP, if queue not empty (START)?

// - what kind of callbacks?
// - targets of callbacks (device? => several instances possible for same device class!)?
// - how to handle callbacks without virtual methods? => events? must be optional!
// - develop promise/future library to handle future values of devices?
// - even promises will need some kind of callback (events? but events are virtual-based!)
// - improve API to allow writing/reading a whole buffer as only one command
// - allow to predetermine that buffer is big enough to hold a complete chain of commands!

// - should sync and async handler code cohabitate? or use only asyn but allow devices to await (sync) or not (async) 
// - ATtiny support
// - short addresses (1 byte) for some ATtiny devices?

// - Optimization of command size: combine starts with slar/slaw? (2 bytes instead of 3)
// - Optimization of command size: support multibyte send/receive from/to an address (4 bytes per command)
// - Optimization of command size: more commands for send/receive 2, 3, 4... bytes (3 bytes per command)


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
		TWCR_ = bits::BV8(TWEN);
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

	bool is_ready() const
	{
		return commands_.empty();
		// return !(TWCR_ & bits::BV8(TWIE));
	}

	//TODO do we really need synchronized version of all these methods?
	// Anyway, the whole chain of commands should be encapsulated in a sync block!
	bool check_queue(uint8_t num_starts, uint8_t num_sends, uint8_t num_receives)
	{
		synchronized return check_queue_(num_starts, num_sends, num_receives);
	}

	bool start()
	{
		synchronized return start_();
	}
	bool repeat_start()
	{
		synchronized return repeat_start_();
	}
	bool send_slar(uint8_t address)
	{
		synchronized return send_slar_(address);
	}
	bool send_slaw(uint8_t address)
	{
		synchronized return send_slaw_(address);
	}
	//TODO shall we handle sending N bytes (from one address)?
	bool send_data(uint8_t data)
	{
		synchronized return send_data_(data);
	}
	//TODO shall we handle receiving N bytes (to one address)?
	bool receive_data(uint8_t& data, bool last_byte)
	{
		synchronized return	receive_data_(data, last_byte);
	}
	bool stop()
	{
		synchronized return stop_();
	}

	bool check_queue_(uint8_t num_starts, uint8_t num_sends, uint8_t num_receives)
	{
		// - each start (including repeat start) is 1 byte, plus 2 bytes for the following slaw/slar
		// - each send is 2 bytes
		// - each receive is 3 bytes
		// - one stop is mandatory to end the I2C chain of commands
		return commands_.free_() >= (num_starts * 3 + num_sends * 2 + num_receives * 3 + 1);
	}

	bool start_()
	{
		return push_byte_(uint8_t(I2CCommand::START), true);
	}
	bool repeat_start_()
	{
		return push_byte_(uint8_t(I2CCommand::REPEAT_START), true);
	}
	bool send_slar_(uint8_t address)
	{
		return push_byte_(uint8_t(I2CCommand::SLAR), false) && push_byte_(address, true);
	}
	bool send_slaw_(uint8_t address)
	{
		return push_byte_(uint8_t(I2CCommand::SLAW), false) && push_byte_(address, true);
	}
	//TODO shall we handle sending N bytes (from one address)?
	bool send_data_(uint8_t data)
	{
		return push_byte_(uint8_t(I2CCommand::WDATA), false) && push_byte_(data, true);
	}
	//TODO shall we handle receiving N bytes (to one address)?
	bool receive_data_(uint8_t& data, bool last_byte)
	{
		return	push_byte_(uint8_t(last_byte ? I2CCommand::RDATA_LAST : I2CCommand::RDATA), false)
			&&	push_byte_(utils::high_byte(reinterpret_cast<uint16_t>(&data)), false)
			&&	push_byte_(utils::low_byte(reinterpret_cast<uint16_t>(&data)), true);
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
	bool push_byte_(uint8_t data, bool finished)
	{
		bool ok = commands_.push(data);
		// Check if need to initiate transmission
		if (ok && finished && !(TWCR_ & bits::BV8(TWIE)))
			// Dequeue first command and start TWI operation
			//FIXME if this is a STOP command, we have to perform an additional dequeue!
			dequeue_command_();
		return ok;
	}

	// Dequeue the next command in the queue and process it immediately
	// returns false if queue is empty
	bool dequeue_command_()
	{
		uint8_t command = 0;
		commands_.pull_(command);
		current_ = I2CCommand(command);
		// Check command type and read more
		switch (current_)
		{
			case I2CCommand::START:
			return exec_start_();
			
			case I2CCommand::REPEAT_START:
			return exec_repeat_start_();
			
			case I2CCommand::STOP:
			return exec_stop_();
			
			case I2CCommand::SLAR:
			return exec_send_slar_();

			case I2CCommand::SLAW:
			return exec_send_slaw_();

			case I2CCommand::WDATA:
			return exec_send_data_();
			
			case I2CCommand::RDATA:
			return exec_receive_data_(false);
			
			case I2CCommand::RDATA_LAST:
			return exec_receive_data_(true);

			default:
			// No more I2C command to execute
			TWCR_ = bits::BV8(TWEN, TWINT);
			return false;
		}
	}

	// Low-level methods to handle the bus in an asynchronous way
	bool exec_start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::START_TRANSMITTED;
		return true;
	}
	bool exec_repeat_start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::REPEAT_START_TRANSMITTED;
		return true;
	}
	bool exec_send_slar_()
	{
		// Read device address from queue
		uint8_t address;
		if (commands_.pull_(address)) return false;
		TWDR_ = address | 0x01U;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_R_TRANSMITTED_ACK;
		return true;
	}
	bool exec_send_slaw_()
	{
		// Read device address from queue
		uint8_t address;
		if (commands_.pull_(address)) return false;
		TWDR_ = address;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_W_TRANSMITTED_ACK;
		return true;
	}
	bool exec_send_data_()
	{
		// Read data from queue
		uint8_t data;
		if (commands_.pull_(data)) return false;
		TWDR_ = data;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::DATA_TRANSMITTED_ACK;
		return true;
	}
	bool exec_receive_data_(bool last_byte)
	{
		// Read buffer address from queue
		uint8_t high_address = 0;
		if (commands_.pull_(high_address)) return false;
		uint8_t low_address = 0;
		if (commands_.pull_(low_address)) return false;
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
		return true;
	}
	bool exec_stop_()
	{
		//TODO check if this code works fine (i.e. clear TWIE with TWSTO)
		// TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTO);
		TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		expected_status_ = 0;
		// Check if there is one command in queue
		if (commands_.empty_())
			// No more I2C command to execute
			return false;

		// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
		// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
		//TODO this can be reduced by the time it takes to pull the next command!
		_delay_loop_1(DELAY_AFTER_STOP);
		return true;
	}

	I2CCallback i2c_change()
	{
		// Check status Vs. expected status
		status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
		if (status_ != expected_status_)
		{
			// Acknowledge TWI interrupt
			TWCR_ |= bits::BV8(TWINT);
			//TODO how to handle errors? normally send STOP (master mode)
			//TODO What to do with pending commands in queue?
			// Note: that behavior could be customizable (clear all, do nothing, or clear current chain until stop)
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

		// Acknowledge TWI interrupt
		TWCR_ |= bits::BV8(TWINT);

		// Handle next command if any
		// If it was a stop then special behavior is required
		if (dequeue_command_() && current_ == I2CCommand::STOP)
		{
			// We need to dequeue another command if any
			dequeue_command_();
			// We need to let ISR execute any callbacks!
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
// enum class WeekDay : uint8_t
// {
// 	SUNDAY = 1,
// 	MONDAY,
// 	TUESDAY,
// 	WEDNESDAY,
// 	THURSDAY,
// 	FRIDAY,
// 	SATURDAY
// };

// struct tm
// {
// 	uint8_t tm_sec;  /**< seconds after the minute - [ 0 to 59 ] */
// 	uint8_t tm_min;  /**< minutes after the hour - [ 0 to 59 ] */
// 	uint8_t tm_hour; /**< hours since midnight - [ 0 to 23 ] */
// 	WeekDay tm_wday; /**< days since Sunday - [ 1 to 7 ] */
// 	uint8_t tm_mday; /**< day of the month - [ 1 to 31 ] */
// 	uint8_t tm_mon;  /**< months since January - [ 1 to 12 ] */
// 	uint8_t tm_year; /**< years since 2000 */
// };

// enum class SquareWaveFrequency : uint8_t
// {
// 	FREQ_1HZ = 0x00,
// 	FREQ_4096HZ = 0x01,
// 	FREQ_8192HZ = 0x02,
// 	FREQ_32768HZ = 0x03
// };

// Actual test example
//=====================
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

	// bool set_ram(uint8_t address, const uint8_t* data, uint8_t size)
	// {
	// 	address += RAM_START;
	// 	if ((address + size) <= RAM_END)
	// 		return write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
	// 				&& write(DEVICE_ADDRESS, data, size, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
	// 	else
	// 		return false;
	// }

	// bool get_ram(uint8_t address, uint8_t* data, uint8_t size)
	// {
	// 	address += RAM_START;
	// 	if ((address + size) <= RAM_END)
	// 		return write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
	// 				&& read(DEVICE_ADDRESS, data, size, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK;
	// 	else
	// 		return false;
	// }

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	// static constexpr const uint8_t TIME_ADDRESS = 0x00;
	// static constexpr const uint8_t CLOCK_HALT = 0x80;
	// static constexpr const uint8_t CONTROL_ADDRESS = 0x07;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;

	// union ControlRegister
	// {
	// 	explicit ControlRegister(uint8_t data = 0) : data{data} {}

	// 	uint8_t data;
	// 	struct
	// 	{
	// 		uint8_t rs : 2;
	// 		uint8_t res1 : 2;
	// 		uint8_t sqwe : 1;
	// 		uint8_t res2 : 2;
	// 		uint8_t out : 1;
	// 	};
	// };

	I2CHandler<i2c::I2CMode::STANDARD>& handler_;
};

using I2CHANDLER = I2CHandler<i2c::I2CMode::STANDARD>;

REGISTER_ASYNC_I2C(i2c::I2CMode::STANDARD)

static constexpr uint8_t I2C_BUFFER_SIZE = 128;
static uint8_t i2c_buffer[I2C_BUFFER_SIZE];

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace streams;

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

	//TODO Test 1: write one byte, read one byte, delay
	//TODO Test 2: add ready() method in I2CHandler
	//TODO Test 3: add simple callback (function or method, in charge of dispatching)
	while (true)
	{
		constexpr uint8_t RAM_SIZE = rtc.ram_size();
		// Write all RAM bytes
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			bool ok = rtc.set_ram(i, i + 1);
			uint8_t status = handler.status();
			out << F("set_ram(") << dec << i << F(") => ") << ok << F(", status = 0x") << hex << status
				<< F(", items = ") << dec << handler.commands_.items() << endl;
			// if not OK (queue full), wait until ready
			if (!ok)
			{
				out << F("waiting...") << flush;
				while (!handler.is_ready())
				{
					out << '.' << flush;
					time::delay_ms(100);
				}
				out << endl;
			}
		}
		time::delay_ms(1000);
		//TODO Read all RAM bytes
	}
}
