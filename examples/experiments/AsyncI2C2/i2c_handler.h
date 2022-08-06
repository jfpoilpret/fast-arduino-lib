//   Copyright 2016-2022 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

#ifndef I2C_HANDLER_HH
#define I2C_HANDLER_HH

#include <util/delay_basic.h>

#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>

#include "array.h"

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

// Debugging stuff
//=================
enum class DebugStatus : uint8_t
{
	START = 0,
	REPEAT_START,
	SLAW,
	SLAR,
	SEND,
	RECV,
	RECV_LAST,
	STOP,

	SEND_OK,
	SEND_ERROR,
	RECV_OK,
	RECV_ERROR,

	REGISTER_OK,
	REGISTER_ERROR
};

// Add utility ostream manipulator for FutureStatus
static const flash::FlashStorage* convert(DebugStatus s)
{
	switch (s)
	{
		case DebugStatus::START:
		return F("START");

		case DebugStatus::REPEAT_START:
		return F("REPEAT_START");

		case DebugStatus::SLAW:
		return F("SLAW");

		case DebugStatus::SLAR:
		return F("SLAR");

		case DebugStatus::SEND:
		return F("SEND");

		case DebugStatus::RECV:
		return F("RECV");

		case DebugStatus::RECV_LAST:
		return F("RECV_LAST");

		case DebugStatus::STOP:
		return F("STOP");

		case DebugStatus::SEND_OK:
		return F("SEND_OK");

		case DebugStatus::SEND_ERROR:
		return F("SEND_ERROR");

		case DebugStatus::RECV_OK:
		return F("RECV_OK");

		case DebugStatus::RECV_ERROR:
		return F("RECV_ERROR");

		case DebugStatus::REGISTER_OK:
		return F("REGISTER_OK");

		case DebugStatus::REGISTER_ERROR:
		return F("REGISTER_ERROR");
	}
}

streams::ostream& operator<<(streams::ostream& out, DebugStatus s)
{
	return out << convert(s);
}

static constexpr uint8_t MAX_DEBUG = 128;
static DebugStatus debug_status[MAX_DEBUG];
static uint8_t debug_index = 0;

static void trace_states(streams::ostream& out, bool reset = true)
{
	for (uint8_t i = 0; i < debug_index; ++i)
		out << debug_status[i] << streams::endl;
	if (reset)
		debug_index = 0;
}

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
class I2CCommandType
{
public:
	constexpr I2CCommandType()
		: none{true}, write{false}, force_stop{false}, finish_future{false} {}
	constexpr I2CCommandType(const I2CCommandType&) = default;
	constexpr I2CCommandType& operator=(const I2CCommandType&) = default;

private:
	constexpr I2CCommandType(bool write, bool force_stop, bool finish_future)
		: none{false}, write{write}, force_stop{force_stop}, finish_future{finish_future} {}

	// true if this is an empty command
	bool none : 1;
	// true if this is a write command, false for a read command
	bool write : 1;
	// true if a STOP condition must absolutely be forced at the end of this command
	bool force_stop : 1;
	// true if aasociated future is void and must eb forced finished after this command
	bool finish_future : 1;

	friend class I2CCommand;
	template<i2c::I2CMode> friend class I2CHandler;
};

// Command in the queue
class I2CCommand
{
public:
	constexpr I2CCommand() = default;
	constexpr I2CCommand(const I2CCommand&) = default;
	constexpr I2CCommand& operator=(const I2CCommand&) = default;

private:
	static constexpr I2CCommand none()
	{
		return I2CCommand{};
	}
	static constexpr I2CCommand read(uint8_t target, bool force_stop, uint8_t future_id, bool finish_future)
	{
		return I2CCommand{I2CCommandType{false, force_stop, finish_future}, target, future_id};
	}
	static constexpr I2CCommand write(uint8_t target, bool force_stop, uint8_t future_id, bool finish_future)
	{
		return I2CCommand{I2CCommandType{true, force_stop, finish_future}, target, future_id};
	}

	constexpr I2CCommand(I2CCommandType type, uint8_t target, uint8_t future_id)
		: type{type}, target{target}, future_id{future_id} {}

	// Type of this command
	I2CCommandType type = I2CCommandType{};
	// Address of the target device (on 8 bits, already left-shifted)
	uint8_t target = 0;
	uint8_t future_id = 0;

	template<i2c::I2CMode> friend class I2CHandler;
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

	bool ensure_num_commands_(uint8_t num_commands)
	{
		return commands_.free_() >= num_commands;
	}
	bool write_(uint8_t target, uint8_t future_id, bool force_stop, bool finish_future)
	{
		return push_command_(I2CCommand::write(target, force_stop, future_id, finish_future));
	}
	bool read_(uint8_t target, uint8_t future_id, bool force_stop, bool finish_future)
	{
		return push_command_(I2CCommand::read(target, force_stop, future_id, finish_future));
	}

private:
	enum class State : uint8_t
	{
		NONE = 0,
		START,
		SLAW,
		SLAR,
		SEND,
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
		synchronized return push_command_(command);
	}

	bool push_command_(const I2CCommand& command)
	{
		if (commands_.push_(command))
		{
			// Check if need to initiate transmission (i.e no current command is executed)
			if (command_.type.none)
				// Dequeue first pending command and start TWI operation
				dequeue_command_(true);
			return true;
		}
		else
			return false;
	}

	// Dequeue the next command in the queue and process it immediately
	void dequeue_command_(bool first)
	{
		if (!commands_.pull_(command_))
		{
			command_ = I2CCommand::none();
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
			return (command_.type.write ? State::SLAW : State::SLAR);

			case State::SLAR:
			case State::RECV:
			if (future::AbstractFutureManager::instance().get_future_value_size_(command_.future_id) > 1)
				return State::RECV;
			else
				return State::RECV_LAST;

			case State::RECV_LAST:
			return State::STOP;

			case State::SLAW:
			return State::SEND;
			
			case State::SEND:
			if (future::AbstractFutureManager::instance().get_storage_value_size_(command_.future_id) >= 1)
				return State::SEND;
			else
				return State::STOP;

			case State::STOP:
			case State::NONE:
			return State::NONE;
		}
	}

	// Low-level methods to handle the bus in an asynchronous way
	void exec_start_()
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::START;
#endif
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::START_TRANSMITTED;
	}
	void exec_repeat_start_()
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::REPEAT_START;
#endif
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		expected_status_ = i2c::Status::REPEAT_START_TRANSMITTED;
	}
	void exec_send_slar_()
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::SLAR;
#endif
		// Read device address from queue
		TWDR_ = command_.target | 0x01U;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_R_TRANSMITTED_ACK;
	}
	void exec_send_slaw_()
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::SLAW;
#endif
		// Read device address from queue
		TWDR_ = command_.target;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		expected_status_ = i2c::Status::SLA_W_TRANSMITTED_ACK;
	}
	void exec_send_data_()
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::SEND;
#endif
		// Determine next data byte
		uint8_t data = 0;
		bool ok = future::AbstractFutureManager::instance().get_storage_value_(command_.future_id, data);
#ifdef DEBUG_SEND_OK
		if (ok)
			debug_status[debug_index++] = DebugStatus::SEND_OK;
#endif
#ifdef DEBUG_SEND_ERR
		if (!ok)
			debug_status[debug_index++] = DebugStatus::SEND_ERROR;
#endif
		TWDR_ = data;
		TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		//NOTE it is possible to get NACK on the last sent byte! That should not be an error!
		expected_status_ = i2c::Status::DATA_TRANSMITTED_ACK;
	}
	void exec_receive_data_()
	{
		// Is this the last byte to receive?
		if (future::AbstractFutureManager::instance().get_future_value_size_(command_.future_id) == 1)
		{
#ifdef DEBUG_STEPS
			debug_status[debug_index++] = DebugStatus::RECV_LAST;
#endif
			// Send NACK for the last data byte we want
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			expected_status_ = i2c::Status::DATA_RECEIVED_NACK;
		}
		else
		{
#ifdef DEBUG_STEPS
			debug_status[debug_index++] = DebugStatus::RECV;
#endif
			// Send ACK for data byte if not the last one we want
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			expected_status_ = i2c::Status::DATA_RECEIVED_ACK;
		}
	}
	void exec_stop_(bool error = false)
	{
#ifdef DEBUG_STEPS
		debug_status[debug_index++] = DebugStatus::STOP;
#endif
		TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		if (!error)
			expected_status_ = 0;
		command_ = I2CCommand::none();
		current_ = State::NONE;
		// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
		// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
		_delay_loop_1(DELAY_AFTER_STOP);
	}

	I2CCallback i2c_change()
	{
		// Check status Vs. expected status
		status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
		if (status_ != expected_status_)
		{
			// Clear all pending transactions from queue
			//NOTE that behavior could be customizable (clear all, or clear only current I2C transaction)
			commands_.clear_();
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			//NOTE possibly retry command instead
			return I2CCallback::ERROR;
		}
		
		// Handle TWI interrupt when data received
		if (current_ == State::RECV || current_ == State::RECV_LAST)
		{
			const uint8_t data = TWDR_;
			bool ok = future::AbstractFutureManager::instance().set_future_value_(command_.future_id, data);
#ifdef DEBUG_RECV_OK
			if (ok)
				debug_status[debug_index++] = DebugStatus::RECV_OK;
#endif
#ifdef DEBUG_RECV_ERR
			if (!ok)
				debug_status[debug_index++] = DebugStatus::RECV_ERROR;
#endif
		}

		// Handle next step in current command
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
						
			case State::SEND:
			exec_send_data_();
			break;

			case State::STOP:
			// Check if we need to finish the current future
			if (command_.type.finish_future)
				future::AbstractFutureManager::instance().set_future_finish_(command_.future_id);
			// Check if we need to STOP (no more pending commands in queue)
			if (commands_.empty_())
				exec_stop_();
			// Check if we need to STOP or REPEAT START (current command requires STOP)
			else if (command_.type.force_stop)
			{
				exec_stop_();
				// Handle next command
				dequeue_command_(true);
			}
			else
				// Handle next command
				dequeue_command_(false);
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
};

#define REGISTER_I2C_ISR(MODE)                                              \
ISR(TWI_vect)                                                               \
{                                                                           \
	interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_change();    \
}
#define REGISTER_I2C_ISR_FUNCTION(MODE, CALLBACK)                           \
ISR(TWI_vect)                                                               \
{                                                                           \
	I2CCallback callback =                                                  \
		interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_change();\
	if (callback != I2CCallback::NONE) CALLBACK(callback);                  \
}
#define REGISTER_I2C_ISR_METHOD(MODE, HANDLER, CALLBACK)                                        \
ISR(TWI_vect)                                                                                   \
{                                                                                               \
	I2CCallback callback =                                                                      \
		interrupt::HandlerHolder<I2CHandler<MODE>>::handler()->i2c_change();                    \
	if (callback != I2CCallback::NONE)                                                          \
		interrupt::CallbackHandler<void (HANDLER::*)(I2CCallback), CALLBACK>::call(callback);	\
}

#endif /* I2C_HANDLER_HH */
/// @endcond
