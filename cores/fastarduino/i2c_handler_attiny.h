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
 * ATtiny I2C Manager API. This defines the synchronous I2C Manager for ATtiny 
 * architecture.
 */
#ifndef I2C_HANDLER_ATTINY_HH
#define I2C_HANDLER_ATTINY_HH

#include <util/delay_basic.h>

#include "i2c.h"
#include "future.h"
#include "bits.h"
#include "utilities.h"
#include "i2c_handler_common.h"

// Prevent inclusion for ATmega architecture
#ifdef TWCR
#error "i2c_handler_attiny.h cannot be included in an ATmega program!"
#endif

/**
 * This macro indicates if truly asynchronous I2C management is available for a platform:
 * - for ATmega architecture, it is set to `1`
 * - for ATtiny architecture, it is set to `0`
 */
#define I2C_TRUE_ASYNC 0

namespace i2c
{
	//==============
	// Sync Handler 
	//==============
	/// @cond notdocumented
	template<I2CMode MODE_, bool HAS_STATUS_ = false, typename STATUS_HOOK_ = I2C_STATUS_HOOK>
	class ATtinyI2CSyncHandler
	{
	private:
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using STATUS = I2CStatusSupport<HAS_STATUS_, STATUS_HOOK_>;

	public:
		ATtinyI2CSyncHandler(STATUS_HOOK_ status_hook = nullptr) : status_hook_{status_hook}
		{
			// set SDA/SCL default directions
			I2C_TRAIT::PORT |= I2C_TRAIT::SCL_SDA_MASK;
			I2C_TRAIT::DDR |= I2C_TRAIT::SCL_SDA_MASK;
		}

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
			// Set SDA back to INPUT
			SDA_INPUT();
		}

		// Low-level methods to handle the bus in an asynchronous way
		bool exec_start_()
		{
			return notify_status(send_start_(),
				Status::START_TRANSMITTED, Status::ARBITRATION_LOST);
		}

		bool exec_repeat_start_()
		{
			return notify_status(send_start_(),
				Status::REPEAT_START_TRANSMITTED, Status::ARBITRATION_LOST);
		}

		bool exec_send_slar_(uint8_t target)
		{
			return notify_status(send_byte_impl(target | 0x01U), 
				Status::SLA_R_TRANSMITTED_ACK, Status::SLA_R_TRANSMITTED_NACK);
		}

		bool exec_send_slaw_(uint8_t target)
		{
			return notify_status(send_byte_impl(target), 
				Status::SLA_W_TRANSMITTED_ACK, Status::SLA_W_TRANSMITTED_NACK);
		}

		bool exec_send_data_(uint8_t data)
		{
			return notify_status(send_byte_impl(data),
				Status::DATA_TRANSMITTED_ACK, Status::DATA_TRANSMITTED_NACK);
		}

		bool exec_receive_data_(bool last_byte, uint8_t& data)
		{
			SDA_INPUT();
			data = transfer(USISR_DATA);
			// Send ACK (or NACK if last byte)
			USIDR_ = (last_byte ? UINT8_MAX : 0x00);
			transfer(USISR_ACK);
			//TODO return ((transfer(USISR_ACK) & 0x01U) == 0);
			return notify_status(true, Status::DATA_RECEIVED_ACK, Status::DATA_RECEIVED_NACK);
		}

		void exec_stop_()
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

	private:
		static constexpr const REG8 USIDR_{USIDR};
		static constexpr const REG8 USISR_{USISR};
		static constexpr const REG8 USICR_{USICR};

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		bool notify_status(bool as_expected, uint8_t good, uint8_t bad)
		{
			status_hook_.call_hook(good, (as_expected ? good : bad));
			return as_expected;
		}  

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

		bool send_start_()
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

		STATUS status_hook_;
	};
	/// @endcond

	/**
	 * Abstract synchronous I2C Manager for ATtiny architecture.
	 * You should never need to subclass AbstractI2CSyncManager yourself.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam HAS_LC_ tells if this I2C Manager must be able to handle 
	 * proxies to Future that can move around and must be controlled by a 
	 * LifeCycleManager; using `false` will generate smaller code.
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
	 */
	template<I2CMode MODE_, bool HAS_LC_, 
		bool HAS_STATUS_, typename STATUS_HOOK_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CSyncATtinyManager
		: public AbstractI2CSyncManager<ATtinyI2CSyncHandler<MODE_, HAS_STATUS_, STATUS_HOOK_>, 
			MODE_, HAS_LC_, STATUS_HOOK_, HAS_DEBUG_, DEBUG_HOOK_>
	{
	private:
		using PARENT = AbstractI2CSyncManager<ATtinyI2CSyncHandler<MODE_, HAS_STATUS_, STATUS_HOOK_>, 
			MODE_, HAS_LC_, STATUS_HOOK_, HAS_DEBUG_, DEBUG_HOOK_>;
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

	protected:
		/// @cond notdocumented
		explicit AbstractI2CSyncATtinyManager(
			lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr, 
				STATUS_HOOK_ status_hook = nullptr, DEBUG_HOOK_ debug_hook = nullptr)
			:	PARENT{lifecycle_manager, status_hook, debug_hook} {}
		/// @endcond

		template<typename> friend class I2CDevice;
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture.
	 * This class offers no support for dynamic proxies, nor any debug facility.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_>
	class I2CSyncManager : 
		public AbstractI2CSyncATtinyManager<MODE_, false, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, false, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>;
	public:
		I2CSyncManager() : PARENT{} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with status notification facility.
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
		public AbstractI2CSyncATtinyManager<MODE_, false, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, false, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>;
	public:
		I2CSyncStatusManager(STATUS_HOOK_ status_hook) : PARENT{nullptr, status_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with debug facility.
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
		public AbstractI2CSyncATtinyManager<MODE_, false, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, false, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncDebugManager(DEBUG_HOOK_ debug_hook) : PARENT{nullptr, nullptr, debug_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with status notification
	 * and debug facilities.
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
		public AbstractI2CSyncATtinyManager<MODE_, false, true, STATUS_HOOK_, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, false, true, STATUS_HOOK_, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncStatusDebugManager(STATUS_HOOK_ status_hook, DEBUG_HOOK_ debug_hook) 
		: PARENT{nullptr, status_hook, debug_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with support for dynamic proxies.
	 * This class offers no debug facility.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_>
	class I2CSyncLCManager : 
		public AbstractI2CSyncATtinyManager<MODE_, true, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, true, false, I2C_STATUS_HOOK, false, I2C_DEBUG_HOOK>;
	public:
		explicit I2CSyncLCManager(lifecycle::AbstractLifeCycleManager& lifecycle_manager)
			:	PARENT{&lifecycle_manager} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with status notification
	 * facility and support for dynamic proxies.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_STATUS_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_, typename STATUS_HOOK_>
	class I2CSyncLCStatusManager : 
		public AbstractI2CSyncATtinyManager<MODE_, true, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, true, true, STATUS_HOOK_, false, I2C_DEBUG_HOOK>;
	public:
		explicit I2CSyncLCStatusManager(
			lifecycle::AbstractLifeCycleManager& lifecycle_manager, STATUS_HOOK_ status_hook)
			:	PARENT{&lifecycle_manager, status_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with debug facility
	 * and support for dynamic proxies.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam DEBUG_HOOK_ the type of the hook to be called. This can be a simple 
	 * function pointer (of type `I2C_DEBUG_HOOK`) or a Functor class (or Functor 
	 * class reference). Using a Functor class will generate smaller code.
	 * 
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class I2CSyncLCDebugManager : 
		public AbstractI2CSyncATtinyManager<MODE_, true, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, true, false, I2C_STATUS_HOOK, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncLCDebugManager(
			lifecycle::AbstractLifeCycleManager& lifecycle_manager, DEBUG_HOOK_ debug_hook)
			:	PARENT{&lifecycle_manager, nullptr, debug_hook} {}
	};

	/**
	 * Synchronous I2C Manager for ATtiny architecture with status notification
	 * and debug facilities and support for dynamic proxies.
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
	class I2CSyncLCStatusDebugManager : 
		public AbstractI2CSyncATtinyManager<MODE_, true, true, STATUS_HOOK_, true, DEBUG_HOOK_>
	{
		using PARENT = AbstractI2CSyncATtinyManager<MODE_, true, true, STATUS_HOOK_, true, DEBUG_HOOK_>;
	public:
		explicit I2CSyncLCStatusDebugManager(
			lifecycle::AbstractLifeCycleManager& lifecycle_manager, STATUS_HOOK_ status_hook, DEBUG_HOOK_ debug_hook)
			:	PARENT{&lifecycle_manager, status_hook, debug_hook} {}
	};

	/// @cond notdocumented
	// Specific traits for I2C Manager
	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncManager<MODE_>>
		:	I2CManager_trait_impl<false, false, false, false, MODE_> {};

	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncLCManager<MODE_>>
		:	I2CManager_trait_impl<false, true, false, false, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_>
	struct I2CManager_trait<I2CSyncStatusManager<MODE_, STATUS_HOOK_>>
		:	I2CManager_trait_impl<false, false, true, false, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, false, false, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncStatusDebugManager<MODE_, STATUS_HOOK_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, false, true, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_>
	struct I2CManager_trait<I2CSyncLCStatusManager<MODE_, STATUS_HOOK_>>
		:	I2CManager_trait_impl<false, true, true, false, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncLCDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, true, false, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncLCStatusDebugManager<MODE_, STATUS_HOOK_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, true, true, true, MODE_> {};
	/// @endcond
}

#endif /* I2C_HANDLER_ATTINY_HH */
/// @endcond
