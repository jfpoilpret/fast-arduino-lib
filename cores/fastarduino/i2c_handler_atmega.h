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

/**
 * @file 
 * ATmega I2C Manager API. This defines asynchronous and synchronous I2C Managers
 * for ATmega architecture.
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
#include "i2c_handler_common.h"

// Prevent direct inclusion (must be done through i2c_handler.h)
#ifndef I2C_HANDLER_HH
#error "i2c_handler_atmega.h shall not be directly included! Include 'i2c_handler.h' instead."
#endif
// Prevent inclusion for ATtiny architecture
#ifndef TWCR
#error "i2c_handler_atmega.h cannot be included in an ATtiny program!"
#endif

/**
 * This macro indicates if truly asynchronous I2C management is available for a platform:
 * - for ATmega architecture, it is set to `1`
 * - for ATtiny architecture, it is set to `0`
 */
#define I2C_TRUE_ASYNC 1

/**
 * Register the necessary ISR (Interrupt Service Routine) for an asynchronous
 * I2C Manager to work properly.
 * 
 * @param MANAGER one of many asynchronous I2C managers defined in this header
 */
#define REGISTER_I2C_ISR(MANAGER)                                   \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change<MANAGER>();                        \
}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an asynchronous
 * I2C Manager to work properly, along with a callback function that will be called
 * everytime an I2C transaction progresses (one command executed, whole transaction
 * executed, error).
 * 
 * @param MANAGER one of many asynchronous I2C managers defined in this header
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered; it should follow this prototype: 
 * `void f(I2CCallback, typename MANAGER::ABSTRACT_FUTURE&)`
 * 
 * @sa i2c::I2CCallback
 */
#define REGISTER_I2C_ISR_FUNCTION(MANAGER, CALLBACK)                \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change_function<MANAGER, CALLBACK>();     \
}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an asynchronous
 * I2C Manager to work properly, along with a callback method that will be called
 * everytime an I2C transaction progresses (one command executed, whole transaction
 * executed, error).
 * 
 * @param MANAGER one of many asynchronous I2C managers defined in this header
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function); it should
 * follow this prototype: `void f(I2CCallback, typename MANAGER::ABSTRACT_FUTURE&)`
 * 
 * @sa i2c::I2CCallback
 */
#define REGISTER_I2C_ISR_METHOD(MANAGER, HANDLER, CALLBACK)             \
ISR(TWI_vect)                                                           \
{                                                                       \
	i2c::isr_handler::i2c_change_method<MANAGER, HANDLER, CALLBACK>();  \
}

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_I2C_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_I2C_ISR_HANDLERS_FRIEND		\
	friend struct i2c::isr_handler;			\
	DECL_TWI_FRIENDS

namespace i2c
{
	/**
	 * I2C Manager policy to use in case of an error during I2C transaction.
	 * @warning available only on ATmega MCU.
	 * @sa I2CAsyncManager
	 */
	enum class I2CErrorPolicy : uint8_t
	{
		/**
		 * Do nothing at all in case of an error; useful only with a synchronous
		 * I2C Manager.
		 */
		DO_NOTHING,

		/**
		 * In case of an error during I2C transaction, then all I2CCommand currently
		 * in queue will be removed.
		 * @warning this means that an error with device A can trigger a removal
		 * of pending commands for device B.
		 */
		CLEAR_ALL_COMMANDS,

		/**
		 * In case of an error during I2C transaction, then all pending I2CCommand
		 * of the current transaction will be removed.
		 */
		CLEAR_TRANSACTION_COMMANDS
	};

	/**
	 * Type passed to I2C ISR registered callbacks (asynchronous I2C Manager only)
	 * when an asynchronous I2C transaction is executed. 
	 */
	enum class I2CCallback : uint8_t
	{
		/** An I2C command is being processed (intermediate step). */
		NONE = 0,
		/** An I2C command has just been finished executed. */
		END_COMMAND,
		/** The last I2C command in a transaction has just been finished executing. */
		END_TRANSACTION,
		/** An error has occurred during I2C transaction execution. */
		ERROR
	};

	/// @cond notdocumented
	template<I2CErrorPolicy POLICY = I2CErrorPolicy::DO_NOTHING> struct I2CErrorPolicySupport
	{
		I2CErrorPolicySupport() = default;
		template<typename T>
		void handle_error(UNUSED const I2CCommand<T>& current, UNUSED containers::Queue<I2CCommand<T>>& commands)
		{
			// Intentionally empty: do nothing in this policy
		}
	};
	template<> struct I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	{
		I2CErrorPolicySupport() = default;
		template<typename T>
		void handle_error(UNUSED const I2CCommand<T>& current, containers::Queue<I2CCommand<T>>& commands)
		{
			commands.clear_();
		}
	};
	template<> struct I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS>
	{
		I2CErrorPolicySupport() = default;
		template<typename T>
		void handle_error(const I2CCommand<T>& current, containers::Queue<I2CCommand<T>>& commands)
		{
			// Clear command belonging to the same transaction (i.e. same future)
			const auto future = current.future();
			I2CCommand<T> command;
			while (commands.peek_(command))
			{
				if (command.future() != future)
					break;
				commands.pull_(command);
			}
		}
	};
	/// @endcond

	//==============
	// Sync Handler 
	//==============
	/// @cond notdocumented
	template<I2CMode MODE_, bool HAS_STATUS_ = false, typename STATUS_HOOK_ = I2C_STATUS_HOOK>
	class ATmegaI2CSyncHandler
	{
	private:
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using STATUS = I2CStatusSupport<HAS_STATUS_, STATUS_HOOK_>;

	public:
		explicit ATmegaI2CSyncHandler(STATUS_HOOK_ status_hook = nullptr) : status_hook_{status_hook} {}

		void begin_()
		{
			// 1. set SDA/SCL pullups
			I2C_TRAIT::PORT |= I2C_TRAIT::SCL_SDA_MASK;
			// 2. set I2C frequency
			TWBR_ = MODE_TRAIT::FREQUENCY;
			TWSR_ = 0;
			// 3. Enable TWI
			TWCR_ = bits::BV8(TWEN);
		}

		void end_()
		{
			// 1. Disable TWI
			TWCR_ = 0;
			// 2. remove SDA/SCL pullups
			I2C_TRAIT::PORT &= bits::COMPL(I2C_TRAIT::SCL_SDA_MASK);
		}

		// Low-level methods to handle the bus in an asynchronous way
		bool exec_start_()
		{
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTA);
			return wait_twint(Status::START_TRANSMITTED);
		}

		bool exec_repeat_start_()
		{
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTA);
			return wait_twint(Status::REPEAT_START_TRANSMITTED);
		}

		bool exec_send_slar_(uint8_t target)
		{
			send_byte(target | 0x01U);
			return wait_twint(Status::SLA_R_TRANSMITTED_ACK);
		}
		
		bool exec_send_slaw_(uint8_t target)
		{
			send_byte(target);
			return wait_twint(Status::SLA_W_TRANSMITTED_ACK);
		}

		bool exec_send_data_(uint8_t data)
		{
			send_byte(data);
			return wait_twint(Status::DATA_TRANSMITTED_ACK);
		}

		bool exec_receive_data_(bool last_byte, uint8_t& data)
		{
			const uint8_t twcr = (last_byte ? bits::BV8(TWEN, TWINT) : bits::BV8(TWEN, TWINT, TWEA));
			const Status expected =  (last_byte ? Status::DATA_RECEIVED_NACK : Status::DATA_RECEIVED_ACK);
			TWCR_ = twcr;
			if (wait_twint(expected))
			{
				data = TWDR_;
				return true;
			}
			return false;
		}

		void exec_stop_()
		{
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
		}
	
	private:
		static constexpr const REG8 TWBR_{TWBR};
		static constexpr const REG8 TWSR_{TWSR};
		static constexpr const REG8 TWCR_{TWCR};
		static constexpr const REG8 TWDR_{TWDR};

		void send_byte(uint8_t data)
		{
			TWDR_ = data;
			TWCR_ = bits::BV8(TWEN, TWINT);
		}

		bool wait_twint(Status expected_status)
		{
			TWCR_.loop_until_bit_set(TWINT);
			const Status status = Status(TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7));
			status_hook_.call_hook(expected_status, status);
			return (status == expected_status);
		}

		STATUS status_hook_;
	};
	/// @endcond

	/**
	 * Abstract synchronous I2C Manager for ATmega architecture.
	 * You should never need to subclass AbstractI2CSyncATmegaManager yourself.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam HAS_STATUS_ tells this I2C Manager to call a status hook at each 
	 * step of an I2C transaction; using `false` will generate smaller code.
	 * @tparam STATUS_HOOK_ the type of the hook to be called when `HAS_STATUS_` is 
	 * `true`. This can be a simple function pointer (of type `I2C_STATUS_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * @tparam HAS_DEBUG_ tells this I2C Manager to call a debugging hook at each 
	 * step of an I2C transaction; this is useful for debugging support for a new 
	 * I2C device; using `false` will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called when `HAS_DEBUG_` is 
	 * `true`. This can be a simple function pointer (of type `I2C_DEBUG_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * 
	 * @sa I2CMode
	 * @sa I2C_STATUS_HOOK
	 * @sa I2C_DEBUG_HOOK
	 * @sa i2c::debug
	 * @sa i2c::status
	 */
	template<I2CMode MODE_, bool HAS_STATUS_, typename STATUS_HOOK_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CSyncATmegaManager
		: public AbstractI2CSyncManager<ATmegaI2CSyncHandler<MODE_, HAS_STATUS_, STATUS_HOOK_>, 
			MODE_, STATUS_HOOK_, HAS_DEBUG_, DEBUG_HOOK_>
	{
	private:
		using PARENT = AbstractI2CSyncManager<ATmegaI2CSyncHandler<MODE_, HAS_STATUS_, STATUS_HOOK_>, 
			MODE_, STATUS_HOOK_, HAS_DEBUG_, DEBUG_HOOK_>;

	public:
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

	protected:
		/// @cond notdocumented
		explicit AbstractI2CSyncATmegaManager(
			STATUS_HOOK_ status_hook = nullptr, DEBUG_HOOK_ debug_hook = nullptr)
			:	PARENT{status_hook, debug_hook} {}
		/// @endcond

		template<typename> friend class I2CDevice;
	};

	//===============
	// Async Manager 
	//===============
	/**
	 * Abstract asynchronous I2C Manager.
	 * It is specifically subclassed for ATmega architecture.
	 * You should never need to subclass AbstractI2CAsyncManager yourself.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * @tparam HAS_STATUS_ tells this I2C Manager to call a status hook at each 
	 * step of an I2C transaction; using `false` will generate smaller code.
	 * @tparam STATUS_HOOK_ the type of the hook to be called when `HAS_STATUS_` is 
	 * `true`. This can be a simple function pointer (of type `I2C_STATUS_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * @tparam HAS_DEBUG_ tells this I2C Manager to call a debugging hook at each 
	 * step of an I2C transaction; this is useful for debugging support for a new 
	 * I2C device; using `false` will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called when `IS_DEBUG` is 
	 * `true`. This can be a simple function pointer (of type `I2C_DEBUG_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * 
	 * @sa I2CMode
	 * @sa I2C_STATUS_HOOK
	 * @sa I2C_DEBUG_HOOK
	 * @sa i2c::debug
	 * @sa i2c::status
	 */
	template<I2CMode MODE_, I2CErrorPolicy POLICY_,
		bool HAS_STATUS_, typename STATUS_HOOK_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CAsyncManager
	{
	private:
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using STATUS = I2CStatusSupport<HAS_STATUS_, STATUS_HOOK_>;
		using DEBUG = I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>;
		using POLICY = I2CErrorPolicySupport<POLICY_>;

	public:
		//TODO DOC
		using ABSTRACT_FUTURE = future::AbstractFuture;
		//TODO DOC
		template<typename OUT, typename IN> using FUTURE = future::Future<OUT, IN>;

		/**
		 * The type of I2CCommand to use in the buffer passed to the constructor 
		 * of this AbstractI2CAsyncManager.
		 */
		using I2CCOMMAND = I2CCommand<ABSTRACT_FUTURE>;

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
			I2C_TRAIT::PORT |= I2C_TRAIT::SCL_SDA_MASK;
			// 2. set I2C frequency
			TWBR_ = MODE_TRAIT::FREQUENCY;
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
			I2C_TRAIT::PORT &= bits::COMPL(I2C_TRAIT::SCL_SDA_MASK);
		}

	protected:
		/// @cond notdocumented
		template<uint8_t SIZE>
		explicit AbstractI2CAsyncManager(
			I2CCOMMAND (&buffer)[SIZE], 
			STATUS_HOOK_ status_hook = nullptr,
			DEBUG_HOOK_ debug_hook = nullptr)
			:	commands_{buffer}, 
				status_hook_{status_hook}, debug_hook_{debug_hook} {}
		/// @endcond

	private:
		bool ensure_num_commands_(uint8_t num_commands) const
		{
			return commands_.free_() >= num_commands;
		}

		bool push_command_(
			I2CLightCommand command, uint8_t target, ABSTRACT_FUTURE& future)
		{
			return commands_.push_(I2CCOMMAND{command, target, future});
		}

		void last_command_pushed_()
		{
			// Check if need to initiate transmission (i.e no current command is executed)
			if (command_.type().is_none())
			{
				// Dequeue first pending command and start TWI operation
				dequeue_command_(true);
			}
		}

		static constexpr const REG8 TWBR_{TWBR};
		static constexpr const REG8 TWSR_{TWSR};
		static constexpr const REG8 TWCR_{TWCR};
		static constexpr const REG8 TWDR_{TWDR};

		// States of execution of an I2C command through ISR calls
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

		ABSTRACT_FUTURE& current_future() const
		{
			return command_.future();
		}

		void send_byte(uint8_t data)
		{
			TWDR_ = data;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		}

		// Dequeue the next command in the queue and process it immediately
		void dequeue_command_(bool first)
		{
			if (!commands_.pull_(command_))
			{
				command_ = I2CCOMMAND{};
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
				return (command_.type().is_write() ? State::SLAW : State::SLAR);

				case State::SLAR:
				case State::RECV:
				if (command_.byte_count() > 1)
					return State::RECV;
				else
					return State::RECV_LAST;

				case State::RECV_LAST:
				return State::STOP;

				case State::SLAW:
				return State::SEND;
				
				case State::SEND:
				if (command_.byte_count() >= 1)
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
			debug_hook_.call_hook(DebugStatus::START);
			expected_status_ = Status::START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_repeat_start_()
		{
			debug_hook_.call_hook(DebugStatus::REPEAT_START);
			expected_status_ = Status::REPEAT_START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_send_slar_()
		{
			debug_hook_.call_hook(DebugStatus::SLAR, command_.target());
			// Read device address from queue
			expected_status_ = Status::SLA_R_TRANSMITTED_ACK;
			send_byte(command_.target() | 0x01U);
		}
		void exec_send_slaw_()
		{
			debug_hook_.call_hook(DebugStatus::SLAW, command_.target());
			// Read device address from queue
			expected_status_ = Status::SLA_W_TRANSMITTED_ACK;
			send_byte(command_.target());
		}
		void exec_send_data_()
		{
			// Determine next data byte
			uint8_t data = 0;
			ABSTRACT_FUTURE& future = current_future();
			bool ok = future.get_storage_value_(data);
			debug_hook_.call_hook(DebugStatus::SEND, data);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (ok)
				command_.decrement_byte_count();
			else
				future.set_future_error_(errors::EILSEQ);
			debug_hook_.call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			expected_status_ = Status::DATA_TRANSMITTED_ACK;
			send_byte(data);
		}
		void exec_receive_data_()
		{
			// Is this the last byte to receive?
			if (command_.byte_count() == 1)
			{
				debug_hook_.call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				expected_status_ = Status::DATA_RECEIVED_NACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			}
			else
			{
				debug_hook_.call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				expected_status_ = Status::DATA_RECEIVED_ACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			}
		}
		void exec_stop_(bool error = false)
		{
			debug_hook_.call_hook(DebugStatus::STOP);
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
			if (!error)
				expected_status_ = Status::OK;
			command_ = I2CCOMMAND{};
			current_ = State::NONE;
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(MODE_TRAIT::DELAY_AFTER_STOP);
		}

		bool is_end_transaction() const
		{
			return command_.type().is_end();
		}

		bool handle_no_error(ABSTRACT_FUTURE& future, Status status)
		{
			if (check_no_error(future, status)) return true;
			policy_.handle_error(command_, commands_);
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			dequeue_command_(true);
			return false;
		}

		I2CCallback i2c_change()
		{
			// Check status Vs. expected status
			const Status status = Status(TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7));
			ABSTRACT_FUTURE& future = current_future();
			if (!handle_no_error(future, status))
				return I2CCallback::ERROR;
			
			// Handle TWI interrupt when data received
			if ((current_ == State::RECV) || (current_ == State::RECV_LAST))
			{
				const uint8_t data = TWDR_;
				bool ok = future.set_future_value_(data);
				// This should only happen in case there are 2 concurrent providers for this future
				if (ok)
					command_.decrement_byte_count();
				else
					future.set_future_error_(errors::EILSEQ);
				debug_hook_.call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
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
				if (command_.type().is_finish())
					future.set_future_finish_();
				result = (is_end_transaction() ? I2CCallback::END_TRANSACTION : I2CCallback::END_COMMAND);
				// Check if we need to STOP (no more pending commands in queue)
				if (commands_.empty_())
					exec_stop_();
				// Check if we need to STOP or REPEAT START (current command requires STOP)
				else if (command_.type().is_stop())
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

		bool check_no_error(ABSTRACT_FUTURE& future, Status status)
		{
			status_hook_.call_hook(expected_status_, status);
			if (status == expected_status_) return true;
			// Handle special case of last transmitted byte possibly not acknowledged by device
			if (	(expected_status_ == Status::DATA_TRANSMITTED_ACK)
				&&	(status == Status::DATA_TRANSMITTED_NACK)
				&&	(command_.byte_count() == 0))
				return true;

			// The future must be marked as error
			if (future.status() != future::FutureStatus::ERROR)
				future.set_future_error_(errors::EPROTO);
			return false;
		}

		// Status of current command processing
		I2CCOMMAND command_;

		// Latest I2C status
		Status expected_status_ = Status::OK;

		// Status of current command processing
		State current_ = State::NONE;

		// Queue of commands to execute
		containers::Queue<I2CCOMMAND> commands_;

		POLICY policy_{};
		STATUS status_hook_;
		DEBUG debug_hook_;

		template<typename> friend class I2CDevice;
		friend struct isr_handler;
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture.
	 * This class offers no support for dynamic proxies, nor any debug facility.
	 * @warning You need to register the proper ISR for this class to work properly.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * 
	 * @sa i2c::I2CMode
	 * @sa i2c::I2CErrorPolicy
	 * @sa REGISTER_I2C_ISR()
	 * @sa REGISTER_I2C_ISR_FUNCTION()
	 * @sa REGISTER_I2C_ISR_METHOD()
	 */
	template<I2CMode MODE_, I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	class I2CAsyncManager : 
		public AbstractI2CAsyncManager<MODE_, POLICY_, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncManager(typename PARENT::I2CCOMMAND (&buffer)[SIZE]) : PARENT{buffer}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture with debug facility.
	 * This class offers no support for dynamic proxies.
	 * @warning You need to register the proper ISR for this class to work properly.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * @tparam DEBUG_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_DEBUG_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 * @sa i2c::I2CErrorPolicy
	 * @sa REGISTER_I2C_ISR()
	 * @sa REGISTER_I2C_ISR_FUNCTION()
	 * @sa REGISTER_I2C_ISR_METHOD()
	 */
	template<
		I2CMode MODE_, 
		I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS, 
		typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CAsyncDebugManager : 
		public AbstractI2CAsyncManager<MODE_, POLICY_, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param debug_hook the debug hook function or functor that is called during
		 * I2C transaction execution.
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncDebugManager(
			typename PARENT::I2CCOMMAND (&buffer)[SIZE], DEBUG_HOOK_ debug_hook) 
			: PARENT{buffer, nullptr, debug_hook}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture with status notification facility.
	 * This class offers no support for dynamic proxies.
	 * @warning You need to register the proper ISR for this class to work properly.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_STATUS_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 * @sa i2c::I2CErrorPolicy
	 * @sa REGISTER_I2C_ISR()
	 * @sa REGISTER_I2C_ISR_FUNCTION()
	 * @sa REGISTER_I2C_ISR_METHOD()
	 */
	template<
		I2CMode MODE_, 
		I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS, 
		typename STATUS_HOOK_ = I2C_STATUS_HOOK>
	class I2CAsyncStatusManager : 
		public AbstractI2CAsyncManager<MODE_, POLICY_, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param status_hook the status hook function or functor that is called during
		 * I2C transaction execution.
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncStatusManager(
			typename PARENT::I2CCOMMAND (&buffer)[SIZE], STATUS_HOOK_ status_hook) 
			: PARENT{buffer, status_hook}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture with debug and status
	 * notification facilities.
	 * This class offers no support for dynamic proxies.
	 * @warning You need to register the proper ISR for this class to work properly.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_STATUS_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_DEBUG_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 * @sa i2c::I2CErrorPolicy
	 * @sa REGISTER_I2C_ISR()
	 * @sa REGISTER_I2C_ISR_FUNCTION()
	 * @sa REGISTER_I2C_ISR_METHOD()
	 */
	template<
		I2CMode MODE_, 
		I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS, 
		typename STATUS_HOOK_ = I2C_STATUS_HOOK,
		typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CAsyncStatusDebugManager : 
		public AbstractI2CAsyncManager<MODE_, POLICY_, true, STATUS_HOOK_, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, true, STATUS_HOOK_, true, DEBUG_HOOK_>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param status_hook the status hook function or functor that is called during
		 * I2C transaction execution.
		 * @param debug_hook the debug hook function or functor that is called during
		 * I2C transaction execution.
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncStatusDebugManager(
			typename PARENT::I2CCOMMAND (&buffer)[SIZE], STATUS_HOOK_ status_hook, DEBUG_HOOK_ debug_hook) 
			: PARENT{buffer, status_hook, debug_hook}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Synchronous I2C Manager for ATmega architecture.
	 * This class offers no support for dynamic proxies, nor any debug facility.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_>
	class I2CSyncManager : 
		public AbstractI2CSyncATmegaManager<MODE_, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATmegaManager<MODE_, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>;
	public:
		I2CSyncManager() : PARENT{} {}
	};

	/**
	 * Synchronous I2C Manager for ATmega architecture wit status notification
	 * facility.
	 * This class offers no support for dynamic proxies.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_STATUS_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_, typename STATUS_HOOK_ = I2C_STATUS_HOOK>
	class I2CSyncStatusManager : 
		public AbstractI2CSyncATmegaManager<MODE_, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATmegaManager<MODE_, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>;
	public:
		explicit I2CSyncStatusManager(STATUS_HOOK_ status_hook) : PARENT{status_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATmega architecture with debug facility.
	 * This class offers no support for dynamic proxies.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam DEBUG_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_DEBUG_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CSyncDebugManager : 
		public AbstractI2CSyncATmegaManager<MODE_, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATmegaManager<MODE_, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncDebugManager(DEBUG_HOOK_ debug_hook) : PARENT{nullptr, debug_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATmega architecture with status notification
	 * and debug facility.
	 * This class offers no support for dynamic proxies.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_STATUS_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_DEBUG_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_, typename STATUS_HOOK_ = I2C_STATUS_HOOK, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CSyncStatusDebugManager : 
		public AbstractI2CSyncATmegaManager<MODE_,  true, STATUS_HOOK_, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATmegaManager<MODE_, true, STATUS_HOOK_, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncStatusDebugManager(STATUS_HOOK_ status_hook, DEBUG_HOOK_ debug_hook) 
		: PARENT{status_hook, debug_hook} {}
	};

	/// @cond notdocumented
	// Specific traits for I2C Manager
	// Async managers first
	template<I2CMode MODE_, I2CErrorPolicy POLICY_>
	struct I2CManager_trait<I2CAsyncManager<MODE_, POLICY_>>
		:	I2CManager_trait_impl<true, false, false, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CAsyncDebugManager<MODE_, POLICY_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<true, false, true, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_, typename STATUS_HOOK_>
	struct I2CManager_trait<I2CAsyncStatusManager<MODE_, POLICY_, STATUS_HOOK_>>
		:	I2CManager_trait_impl<true, true, false, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_, typename STATUS_HOOK_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CAsyncStatusDebugManager<MODE_, POLICY_, STATUS_HOOK_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<true, true, true, MODE_> {};
	/// @endcond

	/// @cond notdocumented
	struct isr_handler
	{
		template<typename MANAGER>
		static void i2c_change()
		{
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2C Manager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2C Manager");
			interrupt::HandlerHolder<MANAGER>::handler()->i2c_change();
		}

		template<typename MANAGER, void (*CALLBACK_)(I2CCallback, typename MANAGER::ABSTRACT_FUTURE&)>
		static void i2c_change_function()
		{
			using interrupt::HandlerHolder;
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2C Manager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2C Manager");
			typename MANAGER::ABSTRACT_FUTURE& future = HandlerHolder<MANAGER>::handler()->current_future();
			I2CCallback callback =  HandlerHolder<MANAGER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE)
			{
				CALLBACK_(callback, future);
			}
		}

		template<typename MANAGER, typename HANDLER_, 
			void (HANDLER_::*CALLBACK_)(I2CCallback, typename MANAGER::ABSTRACT_FUTURE&)>
		static void i2c_change_method()
		{
			using interrupt::HandlerHolder;
			using interrupt::CallbackHandler;
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2C Manager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2C Manager");
			typename MANAGER::ABSTRACT_FUTURE& future = HandlerHolder<MANAGER>::handler()->current_future();
			I2CCallback callback =  HandlerHolder<MANAGER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE)
			{
				using HANDLER = 
					CallbackHandler<void (HANDLER_::*)(I2CCallback, typename MANAGER::ABSTRACT_FUTURE&), CALLBACK_>;
				HANDLER::call(callback, future);
			}
		}
	};
	/// @endcond
}

#endif /* I2C_HANDLER_ATMEGA_HH */
/// @endcond
