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

//TODO rework I2CCommandType to include flag "force stop"
//TODO add callback stuff
//TODO add promises and futures
//TODO support both ACK/NACK on sending? (also, error in case not all bytes sent but NACK is received)
//TODO add policies for behavior on error (retry, clear queue...)

// MAIN IDEA:
// - have a queue of "I2C commands" records
// - each command is either a read or a write and contains all necessary data
// - handling of each command is broken down into sequential steps (State)
// - dequeue and execute each command from TWI ISR, call back when the last step of 
//   a command is finished or an error occurred
// - consecutive commands in the queue are chained with repeat start conditions
// - the last command in the queue is finished with a stop condition
// - for sent or received data, a system of Promise & Future (independent API) is
//   used to hold data until it is not needed anymore and can be released
// - the device API shall return a Promise/Future that can be used asynchronously later on
// NOTE: no dynamic allocation shall be used!

// OPEN POINTS:
// - implement proper callback registration (only one callback, can be an event supplier)
// - should sync and async handler code cohabitate? or use only async but allow devices to await (sync) or not (async) 
//
// - ATtiny support: what's feasible with USI?

// I2C async specific definitions
//================================

// Used by TWI ISR to potentially call a registered callback
enum class I2CCallback : uint8_t
{
	NONE = 0,
	NORMAL_STOP,
	ERROR
};

// Type of commands in queue
enum class I2CCommandType : uint8_t
{
	// No command at all
	NONE = 0,
	// Read command (includes start/repeat start, slar, receive data)
	READ,
	// Write command (includes start/repeat start, slaw, write 1 constant byte)
	WRITE_1,
	// Write command (includes start/repeat start, slaw, write 2 constant bytes)
	WRITE_2,
	// Write command (includes start/repeat start, slaw, write 3 constant bytes)
	WRITE_3,
	// Write command (includes start/repeat start, slaw, write bytes from buffer)
	WRITE_N,
};

//TODO maybe change to class and protect members appropriately
// Command in the queue
struct I2CCommand
{
	static I2CCommand read(uint8_t target, uint8_t* payload, uint8_t size)
	{
		return I2CCommand{I2CCommandType::READ, target, payload, size};
	}
	static I2CCommand write1(uint8_t target, uint8_t data1)
	{
		return I2CCommand{I2CCommandType::WRITE_1, target, data1, 0, 0};
	}
	static I2CCommand write2(uint8_t target, uint8_t data1, uint8_t data2)
	{
		return I2CCommand{I2CCommandType::WRITE_2, target, data1, data2, 0};
	}
	static I2CCommand write3(uint8_t target, uint8_t data1, uint8_t data2, uint8_t data3)
	{
		return I2CCommand{I2CCommandType::WRITE_3, target, data1, data2, data3};
	}
	static I2CCommand writeN(uint8_t target, const uint8_t* payload, uint8_t size)
	{
		return I2CCommand{I2CCommandType::WRITE_N, target, const_cast<uint8_t*>(payload), size};
	}

	constexpr I2CCommand() : type{I2CCommandType::NONE}, target{0}, payload{0}, size{0} {}
	I2CCommand(I2CCommandType type, uint8_t target, uint8_t data1, uint8_t data2, uint8_t data3)
		: type{type}, target{target}, data1{data1}, data2{data2}, data3{data3} {}
	I2CCommand(I2CCommandType type, uint8_t target, uint8_t* payload, uint8_t size)
		: type{type}, target{target}, payload{payload}, size{size} {}

	I2CCommand(const I2CCommand&) = default;

	// Type of this command
	I2CCommandType type = I2CCommandType::NONE;
	// Address of the target device (on 8 bits, already left-shifted)
	uint8_t target;
	union
	{
		struct
		{
			uint8_t data1;
			uint8_t data2;
			uint8_t data3;
		};
		struct
		{
			uint8_t* payload;
			uint8_t size;
		};
	};
};

// This is an asynchronous I2C handler
template<i2c::I2CMode MODE_> class I2CHandler
{
public:
	static constexpr const i2c::I2CMode MODE = MODE_;

	I2CHandler(const I2CHandler<MODE_>&) = delete;
	I2CHandler<MODE_>& operator=(const I2CHandler<MODE_>&) = delete;

	template<uint8_t SIZE>
	explicit I2CHandler(I2CCommand (&buffer)[SIZE]) : commands_{buffer}
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

	bool write(uint8_t target, uint8_t data)
	{
		return push_command(I2CCommand::write1(target, data));
	}
	bool write(uint8_t target, uint8_t data1, uint8_t data2)
	{
		return push_command(I2CCommand::write2(target, data1, data2));
	}
	bool write(uint8_t target, uint8_t data1, uint8_t data2, uint8_t data3)
	{
		return push_command(I2CCommand::write3(target, data1, data2, data3));
	}
	bool write(uint8_t target, const uint8_t* data, uint8_t size)
	{
		return push_command(I2CCommand::writeN(target, data, size));
	}
	bool read(uint8_t target, uint8_t* data, uint8_t size = 1)
	{
		return push_command(I2CCommand::read(target, data, size));
	}

private:
	static constexpr I2CCommand NONE = I2CCommand{};

	enum class State : uint8_t
	{
		NONE = 0,
		START,
		SLAW,
		SLAR,
		SEND1,
		SEND2,
		SEND3,
		SENDN,
		RECV,
		RECV_LAST,
		STOP
	};

	using TRAIT = board_traits::TWI_trait;
	using REG8 = board_traits::REG8;

	static constexpr const REG8 TWBR_{TWBR};
	static constexpr const REG8 TWSR_{TWSR};
	static constexpr const REG8 TWCR_{TWCR};
	static constexpr const REG8 TWDR_{TWDR};

	// Push one byte of a command to the queue, and possibly initiate a new transmission right away
	bool push_command(const I2CCommand& command)
	{
		synchronized
		{
			if (commands_.push_(command))
			{
				// Check if need to initiate transmission (i.e no current command is executed)
				if (command_.type == I2CCommandType::NONE)
					// Dequeue first pending command and start TWI operation
					dequeue_command_(true);
				return true;
			}
			else
				return false;
		}
	}

	// Dequeue the next command in the queue and process it immediately
	void dequeue_command_(bool first)
	{
		if (!commands_.pull_(command_))
		{
			command_ = NONE;
			current_ = State::NONE;
			// No more I2C command to execute
			TWCR_ = bits::BV8(TWINT);
			return;
		}

		// Start new commmand
		current_ = State::START;
		if (first)
			exec_start_();
		else
			exec_repeat_start_();
	}

	// Method to compute next state
	State next_state_()
	{
		switch (current_)
		{
			case State::START:
			return ((command_.type == I2CCommandType::READ) ? State::SLAR : State::SLAW);

			case State::SLAR:
			case State::RECV:
			return ((command_.size > 1) ? State::RECV : State::RECV_LAST);

			case State::RECV_LAST:
			return State::STOP;

			case State::SLAW:
			return ((command_.type == I2CCommandType::WRITE_N) ? State::SENDN : State::SEND1);
			
			case State::SEND1:
			return ((command_.type == I2CCommandType::WRITE_1) ? State::STOP : State::SEND2);

			case State::SEND2:
			return ((command_.type == I2CCommandType::WRITE_2) ? State::STOP : State::SEND3);

			case State::SEND3:
			return State::STOP;

			case State::SENDN:
			return ((command_.size > 1) ? State::SENDN : State::STOP);

			case State::STOP:
			case State::NONE:
			return State::NONE;
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
		TWDR_ = command_.target | 0x01U;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_R_TRANSMITTED_ACK;
	}
	void exec_send_slaw_()
	{
		// Read device address from queue
		TWDR_ = command_.target;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_W_TRANSMITTED_ACK;
	}
	void exec_send_data_()
	{
		// Determine next data byte
		switch (current_)
		{
			case State::SEND1:
			TWDR_ = command_.data1;
			break;
			
			case State::SEND2:
			TWDR_ = command_.data2;
			break;
			
			case State::SEND3:
			TWDR_ = command_.data3;
			break;
			
			case State::SENDN:
			TWDR_ = *command_.payload++;
			--command_.size;
			break;

			default:
			// Impossible case!
			break;
		}
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		//TODO it is possible to get NACK on the last sent byte! That should not be an error!
		expected_status_ = i2c::Status::DATA_TRANSMITTED_ACK;
	}
	void exec_receive_data_()
	{
		// Is this the last byte to receive?
		bool last = (command_.size == 1);

		// Then a problem occurs for the last byte we want to get, which should have NACK instead!
		if (last)
		{
			// Send NACK for the last data byte we want
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			expected_status_ = i2c::Status::DATA_RECEIVED_NACK;
		}
		else
		{
			// Send ACK for data byte if not the last one we want
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			expected_status_ = i2c::Status::DATA_RECEIVED_ACK;
		}
	}
	void exec_stop_(bool error = false)
	{
		TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		if (!error)
			expected_status_ = 0;
		command_ = NONE;
		current_ = State::NONE;
		// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
		// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
		_delay_loop_1(DELAY_AFTER_STOP);
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
			//TODO possibly retry command instead
			return I2CCallback::ERROR;
		}
		
		// Handle TWI interrupt when data received
		if (current_ == State::RECV || current_ == State::RECV_LAST)
		{
			*command_.payload++ = TWDR_;
			--command_.size;
		}

		// Handle next step in current command
		bool end_transaction = false;
		current_ = next_state_();
		switch (current_)
		{
			case State::NONE:
			case State::START:
			// This cannot happen
			break;

			case State::SLAR:
			exec_send_slar_();
			break;

			case State::RECV:
			case State::RECV_LAST:
			exec_receive_data_();
			break;

			case State::SLAW:
			exec_send_slaw_();
			break;
						
			case State::SEND1:
			case State::SEND2:
			case State::SEND3:
			case State::SENDN:
			exec_send_data_();
			break;

			case State::STOP:
			end_transaction = true;
			break;
		}

		// Check if current transaction is finished
		if (end_transaction)
		{
			// Check if we need to STOP or REPEAT START
			if (commands_.empty_())
				exec_stop_();
			else
			{
				//TODO try to fix issue between set_ram and get_ram
				// if it works, then introduce a force stop flag in I2CCommand
				exec_stop_();
				dequeue_command_(true);
				// Handle next command
				// dequeue_command_(false);
			}
			return I2CCallback::NORMAL_STOP;
		}

		return I2CCallback::NONE;
	}

	static constexpr const uint32_t STANDARD_FREQUENCY = (F_CPU / 100'000UL - 16UL) / 2;
	static constexpr const uint32_t FAST_FREQUENCY = (F_CPU / 400'000UL - 16UL) / 2;
	static constexpr const uint8_t TWBR_VALUE = (MODE == i2c::I2CMode::STANDARD ? STANDARD_FREQUENCY : FAST_FREQUENCY);

	static constexpr const float STANDARD_DELAY_AFTER_STOP_US = 4.0 + 4.7;
	static constexpr const float FAST_DELAY_AFTER_STOP_US = 0.6 + 1.3;
	static constexpr const float DELAY_AFTER_STOP_US =
		(MODE == i2c::I2CMode::STANDARD ? STANDARD_DELAY_AFTER_STOP_US : FAST_DELAY_AFTER_STOP_US);
	static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(DELAY_AFTER_STOP_US);

	containers::Queue<I2CCommand> commands_;

	// Status of current command processing
	I2CCommand command_;
	State current_;
	uint8_t expected_status_ = 0;

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
		return handler_.write(DEVICE_ADDRESS, address, data);
	}

	bool get_ram(uint8_t address, uint8_t& data)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return false;
		//FIXME need pre-check to ensure queue is big enough for 2 I2C commands!
		return handler_.write(DEVICE_ADDRESS, address) && handler_.read(DEVICE_ADDRESS, &data, 1);
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

static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static I2CCommand i2c_buffer[I2C_BUFFER_SIZE];

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
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
	out << boolalpha << showbase;

	handler.begin();
	// time::delay_ms(500);

	//TODO Test 1: write one byte, read one byte, delay
	//TODO Test 2: add ready() method in I2CHandler
	//TODO Test 3: add simple callback (function or method, in charge of dispatching)
	// while (true)
	{
		constexpr uint8_t RAM_SIZE = rtc.ram_size();
	
		out << F("TEST #1 write and read RAM bytes, one by one") << endl;
		uint8_t data1[RAM_SIZE];
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
			data1[i] = 0;
		for (uint8_t i = 0; i < RAM_SIZE; ++i)
		{
			bool ok1 = rtc.set_ram(i, i + 1);
			bool ok2 = rtc.get_ram(i, data1[i]);
			out << F("set_ram(") << dec << i << F(") => ") << ok1 << endl;
			out << F("get_ram(") << dec << i << F(") => ") << ok2 << endl;
			out << F("get_ram() data = ") << dec << data1[i] << endl;
		}
		time::delay_ms(1000);
		out << F("all data after 1s = [") << data1[0] << flush;
		for (uint8_t i = 1; i < RAM_SIZE; ++i)
			out << F(", ") << data1[i] << flush;
		out << ']' << endl;

		// The following test works properly
		// out << F("TEST #2 write all RAM bytes, one by one, then read all, one by one") << endl;
		// for (uint8_t i = 0; i < RAM_SIZE; ++i)
		// {
		// 	bool ok = rtc.set_ram(i, i * 2 + 1);
		// 	out << F("set_ram(") << dec << i << F(") => ") << ok << endl;
		// }
		// for (uint8_t i = 0; i < RAM_SIZE; ++i)
		// {
		// 	uint8_t data = 0;
		// 	bool ok = rtc.get_ram(i, data);
		// 	out << F("get_ram(") << dec << i << F(") => ") << ok << endl;
		// 	out << F("get_ram() data = ") << dec << data << endl;
		// }
		// time::delay_ms(1000);
	}
}
