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

#define I2C_TRUE_ASYNC 1

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

/**
 * Register the necessary ISR (Interrupt Service Routine) for an asynchronous
 * I2CManager to work properly.
 */
#define REGISTER_I2C_ISR(MANAGER)                                   \
ISR(TWI_vect)                                                       \
{                                                                   \
	i2c::isr_handler::i2c_change<MANAGER>();                        \
}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an asynchronous
 * I2CManager to work properly, along with a callback method that will be called
 * everytime an I2C transaction progresses (one command executed, whole transaction
 * executed, error).
 * 
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
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
 * I2CManager to work properly, along with a callback method that will be called
 * everytime an I2C transaction progresses (one command executed, whole transaction
 * executed, error).
 * 
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 * 
 * @sa i2c::I2CCallback
 */
#define REGISTER_I2C_ISR_METHOD(MANAGER, HANDLER, CALLBACK)             \
ISR(TWI_vect)                                                           \
{                                                                       \
	i2c::isr_handler::i2c_change_method<MANAGER, HANDLER, CALLBACK>();  \
}

namespace i2c
{
	//TODO Maybe add an extra layer AbstractATmegaI2CManager with all common methods for sync and ascyn?
	/**
	 * Abstract asynchronous I2C Manager.
	 * It is specifically subclassed for ATmega architecture.
	 * You should never need to subclass AbstractI2CAsyncManager yourself.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam POLICY_ the policy to use in case of an error during I2C transaction
	 * @tparam HAS_LC_ tells if this I2CManager must be able to handle 
	 * proxies to Future that can move around and must be controlled by a 
	 * LifeCycleManager; using `false` will generate smaller code.
	 * @tparam HAS_DEBUG_ tells this I2CManager to call a debugging hook at each 
	 * step of an I2C transaction; this is useful for debugging support for a new 
	 * I2C device; using `false` will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called when `IS_DEBUG` is 
	 * `true`. This can be a simple function pointer (of type `I2C_DEBUG_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * 
	 * @sa I2CMode
	 * @sa I2C_DEBUG_HOOK
	 */
	template<I2CMode MODE_, I2CErrorPolicy POLICY_, bool HAS_LC_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CAsyncManager :
		public AbstractI2CManager,
		public I2CErrorPolicySupport<POLICY_>, 
		public I2CLifeCycleSupport<HAS_LC_>, 
		public I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>
	{
	private:
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using PARENT = AbstractI2CManager;
		using DEBUG = I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>;
		using POLICY = I2CErrorPolicySupport<POLICY_>;
		using LC = I2CLifeCycleSupport<HAS_LC_>;

	public:
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
			I2CCommand (&buffer)[SIZE], 
			lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr,
			DEBUG_HOOK_ hook = nullptr)
			:	AbstractI2CManager{},
				I2CErrorPolicySupport<POLICY_>{},
				I2CLifeCycleSupport<HAS_LC_>{lifecycle_manager},
				I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>{hook},
				commands_(buffer) {}
		/// @endcond

	private:
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

		future::AbstractFuture& current_future() const
		{
			return LC::resolve(command_.future());
		}

		bool ensure_num_commands_(uint8_t num_commands)
		{
			return commands_.free_() >= num_commands;
		}

		void send_byte(uint8_t data)
		{
			TWDR_ = data;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
		}

		bool push_command_(const I2CCommand& command)
		{
			return commands_.push_(command);
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

		// Dequeue the next command in the queue and process it immediately
		void dequeue_command_(bool first)
		{
			if (!commands_.pull_(command_))
			{
				command_ = I2CCommand{};
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
			DEBUG::call_hook(DebugStatus::START);
			expected_status_ = Status::START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_repeat_start_()
		{
			DEBUG::call_hook(DebugStatus::REPEAT_START);
			expected_status_ = Status::REPEAT_START_TRANSMITTED;
			TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWSTA);
		}
		void exec_send_slar_()
		{
			DEBUG::call_hook(DebugStatus::SLAR, command_.target());
			// Read device address from queue
			expected_status_ = Status::SLA_R_TRANSMITTED_ACK;
			send_byte(command_.target() | 0x01U);
		}
		void exec_send_slaw_()
		{
			DEBUG::call_hook(DebugStatus::SLAW, command_.target());
			// Read device address from queue
			expected_status_ = Status::SLA_W_TRANSMITTED_ACK;
			send_byte(command_.target());
		}
		void exec_send_data_()
		{
			// Determine next data byte
			uint8_t data = 0;
			future::AbstractFuture& future = current_future();
			bool ok = future.get_storage_value_(data);
			DEBUG::call_hook(DebugStatus::SEND, data);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (ok)
				command_.decrement_byte_count();
			else
				future.set_future_error_(errors::EILSEQ);
			DEBUG::call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			expected_status_ = Status::DATA_TRANSMITTED_ACK;
			send_byte(data);
		}
		void exec_receive_data_()
		{
			// Is this the last byte to receive?
			if (command_.byte_count() == 1)
			{
				DEBUG::call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				expected_status_ = Status::DATA_RECEIVED_NACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT);
			}
			else
			{
				DEBUG::call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				expected_status_ = Status::DATA_RECEIVED_ACK;
				TWCR_ = bits::BV8(TWEN, TWIE, TWINT, TWEA);
			}
		}
		void exec_stop_(bool error = false)
		{
			DEBUG::call_hook(DebugStatus::STOP);
			TWCR_ = bits::BV8(TWEN, TWINT, TWSTO);
			if (!error)
				expected_status_ = 0;
			command_ = I2CCommand{};
			current_ = State::NONE;
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(MODE_TRAIT::DELAY_AFTER_STOP);
		}

		bool is_end_transaction() const
		{
			return command_.type().is_end();
		}

		bool handle_no_error(future::AbstractFuture& future)
		{
			if (PARENT::check_no_error(future)) return true;
			POLICY::handle_error(command_, commands_);
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			dequeue_command_(true);
			return false;
		}

		I2CCallback i2c_change()
		{
			// Check status Vs. expected status
			status_ = TWSR_ & bits::BV8(TWS3, TWS4, TWS5, TWS6, TWS7);
			future::AbstractFuture& future = current_future();
			if (!handle_no_error(future))
				return I2CCallback::ERROR;
			
			// Handle TWI interrupt when data received
			if (current_ == State::RECV || current_ == State::RECV_LAST)
			{
				const uint8_t data = TWDR_;
				bool ok = future.set_future_value_(data);
				// This should only happen in case there are 2 concurrent providers for this future
				if (ok)
					command_.decrement_byte_count();
				else
					future.set_future_error_(errors::EILSEQ);
				DEBUG::call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
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

		// Queue of commands to execute
		containers::Queue<I2CCommand> commands_;

		// Status of current command processing
		State current_ = State::NONE;

		template<I2CMode, typename> friend class I2CDevice;
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
	 * @sa I2CAsyncDebugManager
	 * @sa I2CAsyncLCManager
	 */
	template<I2CMode MODE_, I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	class I2CAsyncManager : public AbstractI2CAsyncManager<MODE_, POLICY_, false, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, false, false, I2C_DEBUG_HOOK>;
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
		explicit I2CAsyncManager(I2CCommand (&buffer)[SIZE]) : PARENT{buffer}
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
	 * @sa I2CAsyncDebugManager
	 * @sa I2CAsyncLCManager
	 */
	template<
		I2CMode MODE_, 
		I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS, 
		typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CAsyncDebugManager : public AbstractI2CAsyncManager<MODE_, POLICY_, false, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, false, true, DEBUG_HOOK_>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param hook the debug hook function or functor that is called during
		 * I2C transaction execution.
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncDebugManager(I2CCommand (&buffer)[SIZE], DEBUG_HOOK_ hook) : PARENT{buffer, nullptr, hook}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture with support for dynalic proxies.
	 * This class offers no debug facility.
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
	 * @sa I2CAsyncDebugManager
	 * @sa I2CAsyncLCManager
	 */
	template<I2CMode MODE_, I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	class I2CAsyncLCManager : public AbstractI2CAsyncManager<MODE_, POLICY_, true, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, true, false, I2C_DEBUG_HOOK>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param lifecycle_manager the AbstractLifeCycleManager used to handle 
		 * the lifecycle of Future used by this I2CManager
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncLCManager(I2CCommand (&buffer)[SIZE], lifecycle::AbstractLifeCycleManager& lifecycle_manager)
			:	PARENT{buffer, &lifecycle_manager}
		{
			interrupt::register_handler(*this);
		}
	};

	/**
	 * Asynchronous I2C Manager for ATmega architecture with debug facility
	 * and support for dynamic proxies.
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
	 * @sa I2CAsyncDebugManager
	 * @sa I2CAsyncLCManager
	 */
	template<
		I2CMode MODE_, 
		I2CErrorPolicy POLICY_ = I2CErrorPolicy::CLEAR_ALL_COMMANDS, 
		typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CAsyncLCDebugManager : public AbstractI2CAsyncManager<MODE_, POLICY_, true, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CAsyncManager<MODE_, POLICY_, true, true, DEBUG_HOOK_>;
	public:
		/**
		 * Create an asynchronous I2C Manager for ATmega MCUs.
		 * 
		 * @tparam SIZE the size of I2CCommand buffer that will be queued for 
		 * asynchronous handling
		 * @param buffer a buffer of @p SIZE I2CCommand items, that will be used to
		 * queue I2C command for asynchronous handling
		 * @param lifecycle_manager the AbstractLifeCycleManager used to handle 
		 * the lifecycle of Future used by this I2CManager
		 * @param hook the debug hook function or functor that is called during
		 * I2C transaction execution
		 */
		template<uint8_t SIZE>
		explicit I2CAsyncLCDebugManager(
			I2CCommand (&buffer)[SIZE], lifecycle::AbstractLifeCycleManager& lifecycle_manager, DEBUG_HOOK_ hook)
			:	PARENT{buffer, &lifecycle_manager, hook}
		{
			interrupt::register_handler(*this);
		}
	};

	/// @cond notdocumented
	// Specific traits for I2CManager
	template<I2CMode MODE_, I2CErrorPolicy POLICY_>
	struct I2CManager_trait<I2CAsyncManager<MODE_, POLICY_>>
		:	I2CManager_trait_impl<true, false, false, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_>
	struct I2CManager_trait<I2CAsyncLCManager<MODE_, POLICY_>>
		:	I2CManager_trait_impl<true, true, false, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CAsyncDebugManager<MODE_, POLICY_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<true, false, true, MODE_> {};

	template<I2CMode MODE_, I2CErrorPolicy POLICY_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CAsyncLCDebugManager<MODE_, POLICY_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<true, true, true, MODE_> {};
	/// @endcond

	/// @cond notdocumented
	struct isr_handler
	{
		template<typename MANAGER>
		static void i2c_change()
		{
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2CManager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2CManager");
			interrupt::HandlerHolder<MANAGER>::handler()->i2c_change();
		}

		template<typename MANAGER, void (*CALLBACK_)(I2CCallback)>
		static void i2c_change_function()
		{
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2CManager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2CManager");
			I2CCallback callback =  interrupt::HandlerHolder<MANAGER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE) CALLBACK_(callback);
		}

		template<typename MANAGER, typename HANDLER_, void (HANDLER_::*CALLBACK_)(I2CCallback)>
		static void i2c_change_method()
		{
			static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2CManager");
			static_assert(I2CManager_trait<MANAGER>::IS_ASYNC, "MANAGER must be an asynchronous I2CManager");
			I2CCallback callback =  interrupt::HandlerHolder<MANAGER>::handler()->i2c_change();
			if (callback != I2CCallback::NONE)
				interrupt::CallbackHandler<void (HANDLER_::*)(I2CCallback), CALLBACK_>::call(callback);
		}
	};
	/// @endcond
}

#endif /* I2C_HANDLER_ATMEGA_HH */
/// @endcond
