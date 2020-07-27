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
 * ATtiny I2C Manager API. This defines the synchronous I2CManager for ATtiny 
 * architecture.
 */
#ifndef I2C_HANDLER_ATTINY_HH
#define I2C_HANDLER_ATTINY_HH

#include <util/delay_basic.h>

#include "i2c.h"
#include "future.h"
#include "bits.h"
#include "utilities.h"
#include "new_i2c_handler_common.h"

#define I2C_TRUE_ASYNC 0

namespace i2c
{
	/**
	 * Abstract synchronous I2C Manager for ATtiny architecture.
	 * You should never need to subclass AbstractI2CSyncManager yourself.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
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
	template<I2CMode MODE_, bool HAS_LC_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CSyncManager :
		public AbstractBaseI2CManager,
		public I2CLifeCycleSupport<HAS_LC_>,
		public I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>
	{
	private:
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using PARENT = AbstractBaseI2CManager;
		using DEBUG = I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>;
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

		/**
		 * Disable MCU I2C transmission.
		 * This method is NOT synchronized.
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			// Disable TWI
			USICR_ = 0;
			// Set SDA back to INPUT
			SDA_INPUT();
		}

	protected:
		/// @cond notdocumented
		explicit AbstractI2CSyncManager(
			lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr, DEBUG_HOOK_ hook = nullptr)
			:	PARENT{}, LC{lifecycle_manager}, DEBUG{hook}
		{
			// set SDA/SCL default directions
			I2C_TRAIT::PORT |= I2C_TRAIT::SCL_SDA_MASK;
			I2C_TRAIT::DDR |= I2C_TRAIT::SCL_SDA_MASK;
		}
		/// @endcond

	private:
		bool ensure_num_commands_(UNUSED uint8_t num_commands)
		{
			return true;
		}

		bool push_command_(
			I2CLightCommand command, uint8_t target, lifecycle::LightProxy<future::AbstractFuture> proxy)
		{
			// Check command is not empty
			const I2CCommandType type = command.type();
			if (type.is_none()) return true;
			if (clear_commands_) return false;
			future::AbstractFuture& future = LC::resolve(proxy);
			// Execute command immediately, from start to optional stop
			if (!exec_start_())
				return handle_error(future, Status::ARBITRATION_LOST);

			if (type.is_write())
			{
				// Send device address
				if (!exec_send_slaw_(target))
					return handle_error(future, Status::SLA_W_TRANSMITTED_NACK);
				// Send content
				while (command.byte_count() > 0)
					// In case of a NACK on data writing, we check if it is not last byte
					if ((!exec_send_data_(command, future)) && (command.byte_count() > 0))
						return handle_error(future, Status::DATA_TRANSMITTED_NACK);
				status_ = Status::DATA_TRANSMITTED_ACK;
			}
			else
			{
				// Send device address
				if (!exec_send_slar_(target))
					return handle_error(future, Status::SLA_R_TRANSMITTED_NACK);
				// Receive content
				while (command.byte_count() > 0)
					if (!exec_receive_data_(command, future))
						return handle_error(future, Status::DATA_RECEIVED_NACK);
				status_ = Status::DATA_RECEIVED_ACK;
			}

			// Check if we must force finish the future
			if (type.is_finish())
				future.set_future_finish_();
			// Check if we must force a STOP
			if (type.is_stop())
				exec_stop_();
			// Ensure STOP is generated or not depending on latest command executed
			no_stop_ = !type.is_stop();
			return true;
		}

		void last_command_pushed_()
		{
			// Check if previously executed command already did a STOP (and needed one)
			if ((!no_stop_) && (!stopped_already_) && (!clear_commands_))
				exec_stop_();
			no_stop_ = false;
			clear_commands_ = false;
			stopped_already_ = false;
		}

		static constexpr const REG8 USIDR_{USIDR};
		static constexpr const REG8 USISR_{USISR};
		static constexpr const REG8 USICR_{USICR};

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		void SCL_HIGH()
		{
			I2C_TRAIT::PORT |= bits::BV8(I2C_TRAIT::BIT_SCL);
			I2C_TRAIT::PIN.loop_until_bit_set(I2C_TRAIT::BIT_SCL);
		}

		void SCL_LOW()
		{
			I2C_TRAIT::PORT &= bits::CBV8(I2C_TRAIT::BIT_SCL);
		}

		void SDA_HIGH()
		{
			I2C_TRAIT::PORT |= bits::BV8(I2C_TRAIT::BIT_SDA);
		}

		void SDA_LOW()
		{
			I2C_TRAIT::PORT &= bits::CBV8(I2C_TRAIT::BIT_SDA);
		}

		void SDA_INPUT()
		{
			I2C_TRAIT::DDR &= bits::CBV8(I2C_TRAIT::BIT_SDA);
		}

		void SDA_OUTPUT()
		{
			I2C_TRAIT::DDR |= bits::BV8(I2C_TRAIT::BIT_SDA);
		}

		bool start_impl_()
		{
			// Ensure SCL is HIGH
			SCL_HIGH();
			// Wait for Tsu-sta
			_delay_loop_1(MODE_TRAIT::T_SU_STA);
			// Now we can generate start condition
			// Force SDA low for Thd-sta
			SDA_LOW();
			_delay_loop_1(MODE_TRAIT::T_HD_STA);
			// Pull SCL low
			SCL_LOW();
			// Release SDA (force high)
			SDA_HIGH();
			// Check START transmission with USISIF flag
			stopped_already_ = false;
			return USISR_ & bits::BV8(USISIF);
		}

		bool send_byte_impl(uint8_t data)
		{
			// Set SCL low
			SCL_LOW();
			// Transfer address byte
			USIDR_ = data;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			SDA_INPUT();
			return ((transfer(USISR_ACK) & 0x01U) == 0);
		}

		uint8_t receive_impl(bool last_byte)
		{
			SDA_INPUT();
			uint8_t data = transfer(USISR_DATA);
			// Send ACK (or NACK if last byte)
			USIDR_ = (last_byte ? UINT8_MAX : 0x00);
			transfer(USISR_ACK);
			return data;
		}

		void stop_impl()
		{
			// Pull SDA low
			SDA_LOW();
			// Release SCL
			SCL_HIGH();
			_delay_loop_1(MODE_TRAIT::T_SU_STO);
			// Release SDA
			SDA_HIGH();
			_delay_loop_1(MODE_TRAIT::T_BUF);
		}

		uint8_t transfer(uint8_t USISR_count)
		{
			//Rework according to AVR310
			// Init counter (8 bits or 1 bit for acknowledge)
			USISR_ = USISR_count;
			do
			{
				_delay_loop_1(MODE_TRAIT::T_LOW);
				// clock strobe (SCL raising edge)
				USICR_ = bits::BV8(USIWM1, USICS1, USICLK, USITC);
				I2C_TRAIT::PIN.loop_until_bit_set(I2C_TRAIT::BIT_SCL);
				_delay_loop_1(MODE_TRAIT::T_HIGH);
				// clock strobe (SCL falling edge)
				USICR_ = bits::BV8(USIWM1, USICS1, USICLK, USITC);
			}
			while ((USISR_ & bits::BV8(USIOIF)) == 0);
			_delay_loop_1(MODE_TRAIT::T_LOW);
			// Read data
			uint8_t data = USIDR_;
			USIDR_ = UINT8_MAX;
			// Release SDA
			SDA_OUTPUT();
			return data;
		}

		// Low-level methods to handle the bus in an asynchronous way
		bool exec_start_()
		{
			DEBUG::call_hook(DebugStatus::START);
			return start_impl_();
		}

		bool exec_send_slar_(uint8_t target)
		{
			DEBUG::call_hook(DebugStatus::SLAR, target);
			return send_byte_impl(target | 0x01U);
		}

		bool exec_send_slaw_(uint8_t target)
		{
			DEBUG::call_hook(DebugStatus::SLAW, target);
			return send_byte_impl(target);
		}

		bool exec_send_data_(I2CLightCommand& command, future::AbstractFuture& future)
		{
			// Determine next data byte
			uint8_t data = 0;
			bool ok = future.get_storage_value_(data);
			DEBUG::call_hook(DebugStatus::SEND, data);
			DEBUG::call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (ok)
			{
				command.decrement_byte_count();
				return send_byte_impl(data);
			}
			else
			{
				future.set_future_error_(errors::EILSEQ);
				status_ = Status::FUTURE_ERROR;
				return false;
			}
		}

		bool exec_receive_data_(I2CLightCommand& command, future::AbstractFuture& future)
		{
			// Is this the last byte to receive?
			uint8_t data;
			if (command.byte_count() == 1)
			{
				DEBUG::call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				data = receive_impl(true);
			}
			else
			{
				DEBUG::call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				data = receive_impl(false);
			}
			// Fill future
			bool ok = future.set_future_value_(data);
			DEBUG::call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
			// This should only happen in case there are 2 concurrent providers for this future
			if (ok)
			{
				command.decrement_byte_count();
			}
			else
			{
				future.set_future_error_(errors::EILSEQ);
				status_ = Status::FUTURE_ERROR;
			}
			return ok;
		}

		void exec_stop_()
		{
			DEBUG::call_hook(DebugStatus::STOP);
			stop_impl();
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(MODE_TRAIT::DELAY_AFTER_STOP);
			stopped_already_ = true;
		}

		// This method is called when an error has occurred
		bool handle_error(future::AbstractFuture& future, uint8_t status)
		{
			// When status is FUTURE_ERROR then future has already been marked accordingly
			if (status != Status::FUTURE_ERROR)
				// The future must be marked as error
				future.set_future_error_(errors::EPROTO);
			// Update status
			status_ = status;

			// Clear command belonging to the same transaction (i.e. same future)
			// ie forbid any new command until last command (add new flag for that)
			clear_commands_ = true;
			// In case of an error, immediately send a STOP condition
			exec_stop_();
			return false;
		}

		// Flags for storing I2C transaction operation state
		bool no_stop_ = false;
		bool clear_commands_ = false;
		bool stopped_already_ = false;

		template<I2CMode, typename> friend class I2CDevice;
	};

#if 0
	/**
	 * General I2C Manager for ATtiny architecture, on which it handles all I2C
	 * commands in a synchronous way (contrarily to ATmega implementation which
	 * is asynchronous).
	 * It is used by all I2C devices for transmission.
	 * 
	 * TODO mention it works the same (API) as ATmega implementation
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @sa i2c::I2CMode
	 */
	template<
		I2CMode MODE_ = I2CMode::STANDARD, 
		bool HAS_LIFECYCLE_ = false, 
		bool IS_DEBUG_ = false, 
		typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CManager : public AbstractBaseI2CManager<MODE_, HAS_LIFECYCLE_, IS_DEBUG_, DEBUG_HOOK_>
	{
	public:
		/**
		 * Create an I2C Manager for ATtiny MCUs, with an optional hook function
		 * for debugging.
		 * 
		 * @param error_policy the policy used to handle queued command when an 
		 * error occurs
		 */
		explicit I2CManager(I2CErrorPolicy error_policy = I2CErrorPolicy::CLEAR_ALL_COMMANDS)
			:	PARENT{error_policy}
		{
			// set SDA/SCL default directions
			I2C_TRAIT::PORT |= bits::BV8(I2C_TRAIT::BIT_SDA, I2C_TRAIT::BIT_SCL);
			I2C_TRAIT::DDR |= bits::BV8(I2C_TRAIT::BIT_SDA, I2C_TRAIT::BIT_SCL);
		}

		/**
		 * Create an I2C Manager for ATtiny MCUs, with an optional hook function
		 * for debugging.
		 * 
		 * @param error_policy the policy used to handle queued command when an 
		 * error occurs
		 * @param hook an optional hook function that will be called back after
		 * each transmission operation.
		 */
		explicit I2CManager(DEBUG_HOOK_ hook, I2CErrorPolicy error_policy = I2CErrorPolicy::CLEAR_ALL_COMMANDS)
			:	PARENT{error_policy, hook}
		{
			// set SDA/SCL default directions
			I2C_TRAIT::PORT |= bits::BV8(I2C_TRAIT::BIT_SDA, I2C_TRAIT::BIT_SCL);
			I2C_TRAIT::DDR |= bits::BV8(I2C_TRAIT::BIT_SDA, I2C_TRAIT::BIT_SCL);
		}
	};
#endif

	//TODO DOC
	template<I2CMode MODE_>
	class I2CSyncManager : public AbstractI2CSyncManager<MODE_, false, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncManager<MODE_, false, false, I2C_DEBUG_HOOK>;
	public:
		I2CSyncManager() : PARENT{} {}
	};

	//TODO DOC
	template<I2CMode MODE_, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CSyncDebugManager : public AbstractI2CSyncManager<MODE_, false, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncManager<MODE_, false, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncDebugManager(DEBUG_HOOK_ hook) : PARENT{nullptr, hook} {}
	};

	//TODO DOC
	template<I2CMode MODE_>
	class I2CSyncLCManager : public AbstractI2CSyncManager<MODE_, true, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncManager<MODE_, true, false, I2C_DEBUG_HOOK>;
	public:
		explicit I2CSyncLCManager(lifecycle::AbstractLifeCycleManager& lifecycle_manager)
			:	PARENT{&lifecycle_manager} {}
	};

	//TODO DOC
	template<I2CMode MODE_, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CSyncLCDebugManager : public AbstractI2CSyncManager<MODE_, true, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncManager<MODE_, true, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncLCDebugManager(lifecycle::AbstractLifeCycleManager& lifecycle_manager)
			:	PARENT{&lifecycle_manager} {}
	};

	/// @cond notdocumented
	// Specific traits for I2CManager
	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncManager<MODE_>>
		:	I2CManager_trait_impl<false, false, false, MODE_> {};

	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncLCManager<MODE_>>
		:	I2CManager_trait_impl<false, true, false, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, false, true, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncLCDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, true, true, MODE_> {};
	/// @endcond
}

#endif /* I2C_HANDLER_ATTINY_HH */
/// @endcond
