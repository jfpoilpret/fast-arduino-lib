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
#include <fastarduino/int.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

// MAIN IDEA:
// - have a queue of "I2C commands" (variable-length)
// - special command: callback?
// - dequeue and execute each command from TWI ISR, call back when last command is finished 

// TODO OPEN POINTS:
// - when to start transmission? at STOP command request? or at first request? or specific API?
// - how to automatically go ahead after STOP, if queue not empty (START)?
// - what kind of callbacks?
// - targets of callbacks (device? => several instances possible for same device class!)?
// - improve API to allow writing/reading a whole buffer as only one command

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
		// 1. set SDA/SCL pullups
		TRAIT::PORT |= TRAIT::SCL_SDA_MASK;
		// 2. set I2C frequency
		TWBR_ = TWBR_VALUE;
		TWSR_ = 0;
		// 3. Enable TWI
		TWCR_ = bits::BV8(TWEN);
	}
	void end()
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

	//TODO do we really need to have synchronized code here? or do we need both flavours?
	bool start()
	{
		synchronized return push_byte_(uint8_t(I2CCommand::START), true);
	}
	bool repeat_start()
	{
		synchronized return push_byte_(uint8_t(I2CCommand::REPEAT_START), true);
	}
	bool send_slar(uint8_t address)
	{
		synchronized return push_byte_(uint8_t(I2CCommand::SLAR), false) && push_byte_(address, true);
	}
	bool send_slaw(uint8_t address)
	{
		synchronized return push_byte_(uint8_t(I2CCommand::SLAW), false) && push_byte_(address, true);
	}
	bool send_data(uint8_t data)
	{
		synchronized return push_byte_(uint8_t(I2CCommand::WDATA), false) && push_byte_(data, true);
	}
	bool receive_data(uint8_t& data, bool last_byte)
	{
		synchronized return	push_byte_(uint8_t(last_byte ? I2CCommand::RDATA_LAST : I2CCommand::RDATA), false)
						&&	push_byte_(utils::high_byte(&data), false)
						&&	push_byte_(utils::low_byte(&data), true);
	}
	void stop()
	{
		synchronized return push_byte_(uint8_t(I2CCommand::STOP), true);
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
		{
			//TODO dequeue first command and start TWI operation
		}
		return ok;
	}

	// Dequeue the next command in the queue and process it immediately
	// returns false in queue is empty
	bool dequeue_command_()
	{
		current_ = NONE;
		commands_.pull_(current_)
		// Check command type and read more
		switch (current_)
		{
			case I2CCommand::START:
			return start_();
			break;
			
			case I2CCommand::REPEAT_START:
			return repeat_start_();
			break;
			
			case I2CCommand::STOP:
			return stop_();
			break;
			
			case I2CCommand::SLAW:
			return send_slaw_();
			break;

			case I2CCommand::WDATA:
			return send_data_();
			break;
			
			case I2CCommand::RDATA:
			return receive_data_(false);
			break;
			
			case I2CCommand::RDATA_LAST:
			return receive_data_(true);
			break;

			default:
			TWCR_ = bits::BV8(TWEN, TWINT);
			return false;
		}
	}

	// Low-level methods to handle the bus in an asynchronous way
	bool start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::START_TRANSMITTED;
		return true;
	}
	bool repeat_start_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::REPEAT_START_TRANSMITTED;
		return true;
	}
	bool send_slar_()
	{
		// Read device address from queue
		uint8_t address;
		if (commands_.pull_(address)) return false;
		TWDR_ = address | 0x01U;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_R_TRANSMITTED_ACK);
		return true;
	}
	bool send_slaw_()
	{
		// Read device address from queue
		uint8_t address;
		if (commands_.pull_(address)) return false;
		TWDR_ = address;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_W_TRANSMITTED_ACK);
		return true;
	}
	bool send_data_()
	{
		// Read data from queue
		uint8_t data;
		if (commands_.pull_(data)) return false;
		TWDR_ = data;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::DATA_TRANSMITTED_ACK);
		return true;
	}
	bool receive_data_(bool last_byte)
	{
		// Read buffer address from queue
		uint8_t high_address = 0;
		if (commands_.pull_(high_address)) return false;
		uint8_t low_address = 0;
		if (commands_.pull_(low_address)) return false;
		const uint16_t address = utils::as_uint16_t(high_address, low_address);
		payload_ = static_cast<uint8_t*>(address);

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
	bool stop_()
	{
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTO);
		expected_status_ = 0;
		// Check if there is one command in queue
		if (commands_.empty_()) return false;

		// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
		// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
		//TODO this can be reduced by the time it takes to pull the next command!
		_delay_loop_1(DELAY_AFTER_STOP);

		// Handle next command
		return dequeue_command_();
	}

	//TODO infer result to include info for REGISTERED ISR & callbacks:
	// - error handling, end of transmission frame,...
	I2CCallback i2c_change()
	{
		// Check status Vs. expected status
		status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
		if (status_ != expected_status_)
		{
			//TODO how to handle errors?
			// Acknowledge TWI interrupt
			TWCR_ |= bits::BV8(TWINT);
			return I2CCallback::ERROR;
		}
		
		//TODO Handle TWI interrupt
		// - if data receive, push byte to buffer
		// - go to next command from queue if any

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
};

//TODO Add callbacks registration
#define REGISTER_ASYNC_I2C(MODE)                                            \
ISR(TWI_vect)                                                               \
{                                                                           \
	auto callback =                                                         \
		interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_change();\
}

REGISTER_ASYNC_I2C(i2c::I2CMode::FAST)

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	//TODO select simple I2C device (RTC?) and drive it asynchronously

	while (true)
	{
	}
}
