//   Copyright 2016-2020 Jean-Francois Poilpret
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

#include <fastarduino/array.h>
#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include <fastarduino/streams.h>
#include <fastarduino/iomanip.h>

// MAIN IDEA:
// - have a queue of "I2C commands" records
// - each command is either a read or a write and contains important flags for handling the command
// - handling of each command is broken down into sequential steps (State)
// - dequeue and execute each command from TWI ISR, call back when the last step of 
//   a command is finished or an error occurred
// - consecutive commands in the queue are chained with repeat start conditions
// - the last command in the queue is finished with a stop condition
// - for sent or received data, a system of Future (independent API) is
//   used to hold data until it is not needed anymore and can be released
// - the device API shall return a Future that can be used asynchronously later on
// NOTE: no dynamic allocation shall be used!

// OPEN POINTS:
// - add event supplier as callback handler?

// - ATtiny support

// I2C async specific definitions
//================================
#if defined(TWCR)
#define REGISTER_I2C_ISR(MODE)                                      \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change<MODE>();                           \
}
#define REGISTER_I2C_ISR_FUNCTION(MODE, CALLBACK)                   \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change_function<MODE, CALLBACK>();        \
}
#define REGISTER_I2C_ISR_METHOD(MODE, HANDLER, CALLBACK)            \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change_method<MODE, HANDLER, CALLBACK>(); \
}
#else
#define REGISTER_I2C_ISR(MODE)
//TODO How to force ompile error if one fo the following macros is used?
//TODO Or how to simulate callbacks in a sync way from i2c_handler?
#define REGISTER_I2C_ISR_FUNCTION(MODE, CALLBACK)
#define REGISTER_I2C_ISR_METHOD(MODE, HANDLER, CALLBACK)
#endif

namespace i2c
{
	enum class I2CErrorPolicy : uint8_t
	{
		CLEAR_ALL_COMMANDS,
		CLEAR_TRANSACTION_COMMANDS
	};

	// Used by TWI ISR to potentially call a registered callback
	enum class I2CCallback : uint8_t
	{
		NONE = 0,
		END_COMMAND,
		END_TRANSACTION,
		ERROR
	};

	// Usef to transmit operating information to hook if registered
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
		RECV_ERROR
	};
	using I2C_DEBUG_HOOK = void (*)(DebugStatus status, uint8_t data);

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
		template<I2CMode> friend class I2CHandler;
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

		template<I2CMode> friend class I2CHandler;
		template<I2CMode> friend class AbstractDevice;
	};

	// This is an asynchronous I2C handler
	template<I2CMode MODE_> class I2CHandler
	{
	public:
		static constexpr const I2CMode MODE = MODE_;

		I2CHandler(const I2CHandler<MODE_>&) = delete;
		I2CHandler<MODE_>& operator=(const I2CHandler<MODE_>&) = delete;

		template<uint8_t SIZE> explicit 
		I2CHandler(	I2CCommand (&buffer)[SIZE],
					I2CErrorPolicy error_policy = I2CErrorPolicy::CLEAR_ALL_COMMANDS,
					I2C_DEBUG_HOOK hook = nullptr)
			:	commands_{buffer}, error_policy_{error_policy}, hook_{hook}
		{
			interrupt::register_handler(*this);
			init_impl();
		}

		void begin()
		{
			synchronized begin_();
		}
		void end()
		{
			synchronized end_();
		}

#if defined(TWCR)
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
#else
		void begin_()
		{
			// 1. Force 1 to data
			USIDR_ = UINT8_MAX;
			// 2. Enable TWI
			// Set USI I2C mode, enable software clock strobe (USITC)
			USICR_ = bits::BV8(USIWM1, USICS1, USICLK);
			// Clear all interrupt flags
			USISR_ = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
			// 3. Set SDA as output
			SDA_OUTPUT();
		}
		void end_()
		{
			// Disable TWI
			USICR_ = 0;
			//TODO should we set SDA back to INPUT?
		}
#endif

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

#if defined(TWCR)
		static constexpr const REG8 TWBR_{TWBR};
		static constexpr const REG8 TWSR_{TWSR};
		static constexpr const REG8 TWCR_{TWCR};
		static constexpr const REG8 TWDR_{TWDR};

		void init_impl() {}

		uint8_t get_status_impl()
		{
			return TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
		}

		void start_impl()
		{
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}

		void send_byte_impl(uint8_t data)
		{
			TWDR_ = data;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		}

		void receive_impl(bool last_byte)
		{
			if (last_byte)
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			else
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
		}

		uint8_t get_byte_impl()
		{
			return TWDR_;
		}

		void stop_impl()
		{
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		}

		void finish_impl()
		{
			TWCR_ = bits::BV8(TWINT);
		}

#else
		static constexpr const REG8 USIDR_{USIDR};
		static constexpr const REG8 USISR_{USISR};
		static constexpr const REG8 USICR_{USICR};

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		// Timing constants for current mode (as per I2C specifications)
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.7 : 1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.7 : 0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(MODE == I2CMode::STANDARD ? 4.7 : 1.3);

		// This is needed to pass information from receive_impl() to get_byte_impl()
		uint8_t data_received_ = 0;

		void SCL_HIGH()
		{
			TRAIT::PORT |= bits::BV8(TRAIT::BIT_SCL);
			TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
		}

		void SCL_LOW()
		{
			TRAIT::PORT &= bits::CBV8(TRAIT::BIT_SCL);
		}

		void SDA_HIGH()
		{
			TRAIT::PORT |= bits::BV8(TRAIT::BIT_SDA);
		}

		void SDA_LOW()
		{
			TRAIT::PORT &= bits::CBV8(TRAIT::BIT_SDA);
		}

		void SDA_INPUT()
		{
			TRAIT::DDR &= bits::CBV8(TRAIT::BIT_SDA);
			// TRAIT::PORT |= bits::BV8(TRAIT::BIT_SDA);
		}

		void SDA_OUTPUT()
		{
			TRAIT::DDR |= bits::BV8(TRAIT::BIT_SDA);
		}

		void init_impl()
		{
			// set SDA/SCL default directions
			TRAIT::DDR &= bits::CBV8(TRAIT::BIT_SDA);
			// TRAIT::PORT |= bits::BV8(TRAIT::BIT_SDA);
			TRAIT::DDR |= bits::BV8(TRAIT::BIT_SCL);
			TRAIT::PORT |= bits::BV8(TRAIT::BIT_SCL);
		}

		uint8_t get_status_impl()
		{
			// status_ has already been updated by all xxx_impl() methods
			return status_;
		}

		void start_impl()
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
			bool ok = USISR & bits::BV8(USISIF);
			status_ = (ok ? expected_status_ : Status::ARBITRATION_LOST);
		}

		void send_byte_impl(uint8_t data)
		{
			// Set SCL low TODO is this line really needed for every byte transferred?
			SCL_LOW();
			// Transfer address byte
			USIDR_ = data;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			SDA_INPUT();
			bool ok = ((transfer(USISR_ACK) & 0x01U) == 0);
			// The expected status is one Status _ACK values
			// When not OK, it shall be changed to the _NACK matching value
			// This can be done by simply adding 0x08 to the ACK value.
			status_ = expected_status_ + (ok ? 0 : 0x08);
		}

		void receive_impl(bool last_byte)
		{
			SDA_INPUT();
			data_received_ = transfer(USISR_DATA);
			// Send ACK (or NACK if last byte)
			USIDR_ = (last_byte ? UINT8_MAX : 0x00);
			transfer(USISR_ACK);
		}

		uint8_t get_byte_impl()
		{
			return data_received_;
		}

		void stop_impl()
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

		void finish_impl()
		{
			// Nothing to do for ATtiny when there is no more I2C command to send
		}

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
#endif

		// Push one byte of a command to the queue, and possibly initiate a new transmission right away
		bool push_command(const I2CCommand& command)
		{
			synchronized return push_command_(command);
		}

		bool push_command_(const I2CCommand& command)
		{
			return commands_.push_(command);
		}

		void last_command_pushed_()
		{
			// Check if need to initiate transmission (i.e no current command is executed)
			if (command_.type.none)
			{
				// Dequeue first pending command and start TWI operation
				dequeue_command_(true);
				// Loop here until command is executed
				while (true)
				{
					switch (i2c_change())
					{
						case I2CCallback::NONE:
						case I2CCallback::END_COMMAND:
						break;;

						case I2CCallback::ERROR:
						case I2CCallback::END_TRANSACTION:
						return;
					}
				}
			}
		}

		// Dequeue the next command in the queue and process it immediately
		void dequeue_command_(bool first)
		{
			if (!commands_.pull_(command_))
			{
				command_ = I2CCommand::none();
				current_ = State::NONE;
				// No more I2C command to execute
				finish_impl();
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
			call_hook(DebugStatus::START);
			expected_status_ = Status::START_TRANSMITTED;
			start_impl();
		}
		void exec_repeat_start_()
		{
			call_hook(DebugStatus::REPEAT_START);
			expected_status_ = Status::REPEAT_START_TRANSMITTED;
			start_impl();
		}
		void exec_send_slar_()
		{
			call_hook(DebugStatus::SLAR, command_.target);
			// Read device address from queue
			expected_status_ = Status::SLA_R_TRANSMITTED_ACK;
			send_byte_impl(command_.target | 0x01U);
		}
		void exec_send_slaw_()
		{
			call_hook(DebugStatus::SLAW, command_.target);
			// Read device address from queue
			expected_status_ = Status::SLA_W_TRANSMITTED_ACK;
			send_byte_impl(command_.target);
		}
		void exec_send_data_()
		{
			// Determine next data byte
			uint8_t data = 0;
			bool ok = future::AbstractFutureManager::instance().get_storage_value_(command_.future_id, data);
			call_hook(DebugStatus::SEND, data);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (!ok)
				future::AbstractFutureManager::instance().set_future_error_(command_.future_id, errors::EILSEQ);
			call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			expected_status_ = Status::DATA_TRANSMITTED_ACK;
			send_byte_impl(data);
		}
		void exec_receive_data_()
		{
			// Is this the last byte to receive?
			if (future::AbstractFutureManager::instance().get_future_value_size_(command_.future_id) == 1)
			{
				call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				expected_status_ = Status::DATA_RECEIVED_NACK;
				receive_impl(true);
			}
			else
			{
				call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				expected_status_ = Status::DATA_RECEIVED_ACK;
				receive_impl(false);
			}
		}
		void exec_stop_(bool error = false)
		{
			call_hook(DebugStatus::STOP);
			stop_impl();
			if (!error)
				expected_status_ = 0;
			command_ = I2CCommand::none();
			current_ = State::NONE;
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(DELAY_AFTER_STOP);
		}

		bool is_end_transaction() const
		{
			I2CCommand command;
			return !(commands_.peek_(command) && command.future_id == command_.future_id);
		}

		bool check_no_error()
		{
			if (status_ == expected_status_) return true;
			// Handle special case of last transmitted byte possibly not acknowledged by device
			if (	(expected_status_ == Status::DATA_TRANSMITTED_ACK)
				&&	(status_ == Status::DATA_TRANSMITTED_NACK)
				&&	(future::AbstractFutureManager::instance().get_storage_value_size_(command_.future_id) == 0))
				return true;

			// The future must be marked as error
			future::AbstractFutureManager::instance().set_future_error_(command_.future_id, errors::EPROTO);
			switch (error_policy_)
			{
				case I2CErrorPolicy::CLEAR_ALL_COMMANDS:
				// Clear all pending transactions from queue
				commands_.clear_();
				break;

				case I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS:
				// Clear command belonging to the same transaction (i.e. same future)
				{
					const uint8_t id = command_.future_id;
					I2CCommand command;
					while (commands_.peek_(command))
					{
						if (command.future_id != id)
							break;
						commands_.pull_(command);
					}
				}
				break;
			}
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			dequeue_command_(true);
			return false;
		}

		I2CCallback i2c_change()
		{
			// Check status Vs. expected status
			status_ = get_status_impl();
			if (!check_no_error())
				return I2CCallback::ERROR;
			
			// Handle TWI interrupt when data received
			if (current_ == State::RECV || current_ == State::RECV_LAST)
			{
				const uint8_t data = get_byte_impl();
				bool ok = future::AbstractFutureManager::instance().set_future_value_(command_.future_id, data);
				// This should only happen in case there are 2 concurrent providers for this future
				if (!ok)
					future::AbstractFutureManager::instance().set_future_error_(command_.future_id, errors::EILSEQ);
				call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
			}

			// Handle next step in current command
			I2CCallback result = I2CCallback::NONE;
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
				result = (is_end_transaction() ? I2CCallback::END_TRANSACTION : I2CCallback::END_COMMAND);
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
			}
			return result;
		}

		void call_hook(DebugStatus status, uint8_t data = 0)
		{
			if (hook_ != nullptr)
				hook_(status, data);
		}

		static constexpr const uint32_t STANDARD_FREQUENCY = (F_CPU / 100'000UL - 16UL) / 2;
		static constexpr const uint32_t FAST_FREQUENCY = (F_CPU / 400'000UL - 16UL) / 2;
		static constexpr const uint8_t TWBR_VALUE = (MODE == I2CMode::STANDARD ? STANDARD_FREQUENCY : FAST_FREQUENCY);

		static constexpr const float STANDARD_DELAY_AFTER_STOP_US = 4.0 + 4.7;
		static constexpr const float FAST_DELAY_AFTER_STOP_US = 0.6 + 1.3;
		static constexpr const float DELAY_AFTER_STOP_US =
			(MODE == I2CMode::STANDARD ? STANDARD_DELAY_AFTER_STOP_US : FAST_DELAY_AFTER_STOP_US);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(DELAY_AFTER_STOP_US);

		containers::Queue<I2CCommand> commands_;
		I2CErrorPolicy error_policy_;
		const I2C_DEBUG_HOOK hook_;

		// Status of current command processing
		I2CCommand command_;
		State current_;
		uint8_t expected_status_ = 0;

		// Latest I2C status
		uint8_t status_ = 0;

		template<I2CMode> friend class AbstractDevice;
		friend struct isr_handler;
	};

	struct isr_handler
	{
		template<I2CMode MODE_>
		static void i2c_change()
		{
			using HANDLER = I2CHandler<MODE_>;
			interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
		}

		template<I2CMode MODE_, void (*CALLBACK_)(I2CCallback)>
		static void i2c_change_function()
		{
			using HANDLER = I2CHandler<MODE_>;
			I2CCallback callback =  interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE) CALLBACK_(callback);
		}

		template<I2CMode MODE_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(I2CCallback)>
		static void i2c_change_method()
		{
			using HANDLER = I2CHandler<MODE_>;
			I2CCallback callback =  interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE)
				interrupt::CallbackHandler<void (HANDLER_::*)(I2CCallback), CALLBACK_>::call(callback);
		}
	};
}

#endif /* I2C_HANDLER_HH */
/// @endcond
