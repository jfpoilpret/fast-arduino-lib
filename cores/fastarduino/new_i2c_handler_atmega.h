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

/**
 * @file 
 * ATmega I2C Manager API. This defines the asynchronous I2CManager for ATmega 
 * architecture.
 */
#ifndef I2C_HANDLER_ATMEGA_HH
#define I2C_HANDLER_ATMEGA_HH

#include <util/delay_basic.h>

#include "i2c.h"
#include "future.h"
#include "queue.h"
#include "interrupts.h"
#include "bits.h"
#include "utilities.h"
#include "new_i2c_handler_common.h"

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
//TODO - add event supplier as callback handler?

//TODO DOC
#define REGISTER_I2C_ISR(MODE)                                      \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change<MODE>();                           \
}
//TODO DOC
#define REGISTER_I2C_ISR_FUNCTION(MODE, CALLBACK)                   \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change_function<MODE, CALLBACK>();        \
}
//TODO DOC
#define REGISTER_I2C_ISR_METHOD(MODE, HANDLER, CALLBACK)            \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change_method<MODE, HANDLER, CALLBACK>(); \
}

namespace i2c
{
	/**
	 * General I2C Manager for ATmega architecture, on which it handles all I2C
	 * commands asynchronously.
	 * It is used by all I2C devices for transmission.
	 * 
	 * TODO provide approach details (already in above comment)
	 * TODO mention need to register ISR
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_ = I2CMode::STANDARD>
	class I2CManager : public AbstractI2CManager<MODE_>
	{
		using SUPER = AbstractI2CManager<MODE_>;

	public:
		/**
		 * Create an I2C Manager for ATmega MCUs, with an optional hook function
		 * for debugging.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param error_policy the policy used to handle queued command when an 
		 * error occurs
		 * @param hook an optional hook function that will be called back after
		 * each transmission operation.
		 */
		template<uint8_t SIZE> explicit 
		I2CManager(	I2CCommand (&buffer)[SIZE],
					I2CErrorPolicy error_policy = I2CErrorPolicy::CLEAR_ALL_COMMANDS,
					I2C_DEBUG_HOOK hook = nullptr)
			:	SUPER{error_policy, hook}, commands_{buffer}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Prepare and enable the MCU for I2C transmission.
		 * Preparation includes setup of I2C pins (SDA and SCL).
		 * This method is synchronized.
		 * @sa end()
		 * @sa begin_()
		 */
		void begin()
		{
			synchronized begin_();
		}

		/**
		 * Disable MCU I2C transmission.
		 * This method is synchronized.
		 * @sa begin()
		 * @sa end_()
		 */
		void end()
		{
			synchronized end_();
		}

		/**
		 * Prepare and enable the MCU for I2C transmission.
		 * Preparation includes setup of I2C pins (SDA and SCL).
		 * This method is NOT synchronized.
		 * @sa end_()
		 * @sa begin()
		 */
		void begin_()
		{
			// 1. set SDA/SCL pullups
			SUPER::TRAIT::PORT |= SUPER::TRAIT::SCL_SDA_MASK;
			// 2. set I2C frequency
			TWBR_ = TWBR_VALUE;
			TWSR_ = 0;
			// 3. Enable TWI
			TWCR_ = bits::BV8(TWEN);
		}

		/**
		 * Disable MCU I2C transmission.
		 * This method is NOT synchronized.
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			// 1. Disable TWI
			TWCR_ = 0;
			// 2. remove SDA/SCL pullups
			SUPER::TRAIT::PORT &= bits::COMPL(SUPER::TRAIT::SCL_SDA_MASK);
		}

	private:
		bool ensure_num_commands_(uint8_t num_commands)
		{
			return commands_.free_() >= num_commands;
		}

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

		static constexpr const typename SUPER::REG8 TWBR_{TWBR};
		static constexpr const typename SUPER::REG8 TWSR_{TWSR};
		static constexpr const typename SUPER::REG8 TWCR_{TWCR};
		static constexpr const typename SUPER::REG8 TWDR_{TWDR};

		void send_byte(uint8_t data)
		{
			TWDR_ = data;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		}

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
			if (this->command_.type.none)
			{
				// Dequeue first pending command and start TWI operation
				dequeue_command_(true);
			}
		}

		// Dequeue the next command in the queue and process it immediately
		void dequeue_command_(bool first)
		{
			if (!commands_.pull_(this->command_))
			{
				this->command_ = I2CCommand::none();
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
				return (this->command_.type.write ? State::SLAW : State::SLAR);

				case State::SLAR:
				case State::RECV:
				if (future::AbstractFutureManager::instance().get_future_value_size_(this->command_.future_id) > 1)
					return State::RECV;
				else
					return State::RECV_LAST;

				case State::RECV_LAST:
				return State::STOP;

				case State::SLAW:
				return State::SEND;
				
				case State::SEND:
				if (future::AbstractFutureManager::instance().get_storage_value_size_(this->command_.future_id) >= 1)
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
			this->call_hook(DebugStatus::START);
			this->expected_status_ = Status::START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_repeat_start_()
		{
			this->call_hook(DebugStatus::REPEAT_START);
			this->expected_status_ = Status::REPEAT_START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_send_slar_()
		{
			this->call_hook(DebugStatus::SLAR, this->command_.target);
			// Read device address from queue
			this->expected_status_ = Status::SLA_R_TRANSMITTED_ACK;
			send_byte(this->command_.target | 0x01U);
		}
		void exec_send_slaw_()
		{
			this->call_hook(DebugStatus::SLAW, this->command_.target);
			// Read device address from queue
			this->expected_status_ = Status::SLA_W_TRANSMITTED_ACK;
			send_byte(this->command_.target);
		}
		void exec_send_data_()
		{
			// Determine next data byte
			uint8_t data = 0;
			bool ok = future::AbstractFutureManager::instance().get_storage_value_(this->command_.future_id, data);
			this->call_hook(DebugStatus::SEND, data);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (!ok)
				future::AbstractFutureManager::instance().set_future_error_(this->command_.future_id, errors::EILSEQ);
			this->call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			this->expected_status_ = Status::DATA_TRANSMITTED_ACK;
			send_byte(data);
		}
		void exec_receive_data_()
		{
			// Is this the last byte to receive?
			if (future::AbstractFutureManager::instance().get_future_value_size_(this->command_.future_id) == 1)
			{
				this->call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				this->expected_status_ = Status::DATA_RECEIVED_NACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			}
			else
			{
				this->call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				this->expected_status_ = Status::DATA_RECEIVED_ACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			}
		}
		void exec_stop_(bool error = false)
		{
			this->call_hook(DebugStatus::STOP);
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
			if (!error)
				this->expected_status_ = 0;
			this->command_ = I2CCommand::none();
			current_ = State::NONE;
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(DELAY_AFTER_STOP);
		}

		bool is_end_transaction() const
		{
			I2CCommand command;
			return !(commands_.peek_(command) && command.future_id == this->command_.future_id);
		}

		bool handle_no_error()
		{
			if (this->check_no_error()) return true;

			switch (this->error_policy_)
			{
				case I2CErrorPolicy::CLEAR_ALL_COMMANDS:
				// Clear all pending transactions from queue
				commands_.clear_();
				break;

				case I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS:
				// Clear command belonging to the same transaction (i.e. same future)
				{
					const uint8_t id = this->command_.future_id;
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
			this->status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
			if (!handle_no_error())
				return I2CCallback::ERROR;
			
			// Handle TWI interrupt when data received
			if (current_ == State::RECV || current_ == State::RECV_LAST)
			{
				const uint8_t data = TWDR_;
				bool ok = future::AbstractFutureManager::instance().set_future_value_(this->command_.future_id, data);
				// This should only happen in case there are 2 concurrent providers for this future
				if (!ok)
					future::AbstractFutureManager::instance().set_future_error_(
						this->command_.future_id, errors::EILSEQ);
				this->call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
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
				if (this->command_.type.finish_future)
					future::AbstractFutureManager::instance().set_future_finish_(this->command_.future_id);
				result = (is_end_transaction() ? I2CCallback::END_TRANSACTION : I2CCallback::END_COMMAND);
				// Check if we need to STOP (no more pending commands in queue)
				if (commands_.empty_())
					exec_stop_();
				// Check if we need to STOP or REPEAT START (current command requires STOP)
				else if (this->command_.type.force_stop)
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

		static constexpr const uint8_t TWBR_VALUE =
			(MODE_ == I2CMode::STANDARD ? SUPER::STANDARD_FREQUENCY : SUPER::FAST_FREQUENCY);

		static constexpr const float STANDARD_DELAY_AFTER_STOP_US = 4.0 + 4.7;
		static constexpr const float FAST_DELAY_AFTER_STOP_US = 0.6 + 1.3;
		static constexpr const float DELAY_AFTER_STOP_US =
			(MODE_ == I2CMode::STANDARD ? STANDARD_DELAY_AFTER_STOP_US : FAST_DELAY_AFTER_STOP_US);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(DELAY_AFTER_STOP_US);

		containers::Queue<I2CCommand> commands_;

		// Status of current command processing
		State current_;

		template<I2CMode> friend class I2CDevice;
		friend struct isr_handler;
	};

	/// @cond notdocumented
	struct isr_handler
	{
		template<I2CMode MODE_>
		static void i2c_change()
		{
			using HANDLER = I2CManager<MODE_>;
			interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
		}

		template<I2CMode MODE_, void (*CALLBACK_)(I2CCallback)>
		static void i2c_change_function()
		{
			using HANDLER = I2CManager<MODE_>;
			I2CCallback callback =  interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE) CALLBACK_(callback);
		}

		template<I2CMode MODE_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(I2CCallback)>
		static void i2c_change_method()
		{
			using HANDLER = I2CManager<MODE_>;
			I2CCallback callback =  interrupt::HandlerHolder<HANDLER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE)
				interrupt::CallbackHandler<void (HANDLER_::*)(I2CCallback), CALLBACK_>::call(callback);
		}
	};
	/// @endcond
}

#endif /* I2C_HANDLER_ATMEGA_HH */
/// @endcond
