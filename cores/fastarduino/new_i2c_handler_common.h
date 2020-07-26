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
 * Common I2C Manager API. This is automatically included by other header files,
 * you should never include it directly in your programs.
 */
#ifndef I2C_HANDLER_COMMON_HH
#define I2C_HANDLER_COMMON_HH

#include <stdint.h>
#include "boards/board_traits.h"
#include "bits.h"
#include "i2c.h"
#include "streams.h"
#include "future.h"
#include "lifecycle.h"
#include "queue.h"
#include "utilities.h"

namespace i2c
{
	/**
	 * I2CManager policy to use in case of an error during I2C transaction.
	 * @warning available only on ATmega MCU.
	 * @sa I2CManager
	 */
	enum class I2CErrorPolicy : uint8_t
	{
		/**
		 * Do nothing at all in case of an error; useful only with a synchronous
		 * I2CManager.
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
	 * List of debug states that are reported by the I2CManager in debug mode.
	 * @sa I2CManager
	 */
	enum class DebugStatus : uint8_t
	{
		/** A start condition has just been sent. */
		START = 0,
		/** A repeat start condition has just been sent. */
		REPEAT_START,
		/** A slave address has just been sent for writing. */
		SLAW,
		/** A slave address has just been sent for reading. */
		SLAR,
		/** A byte has just be sent to the slave. */
		SEND,
		/** A byte is being received from the slave. */
		RECV,
		/** The last byte is being received from the slave. */
		RECV_LAST,
		/** A stop condition has just been sent. */
		STOP,

		/** The latest sent byte has been acknowledged by the slave. */
		SEND_OK,
		/** The latest sent byte has not been acknowledged by the slave. */
		SEND_ERROR,
		/** I2CManager has acknowledged the latest received byte from the slave. */
		RECV_OK,
		/** I2CManager has not acknowledged the latest received byte from the slave. */
		RECV_ERROR
	};

	/**
	 * The default debugging hook type.
	 * @warning Do not use this (function pointer) for your hooks! This will 
	 * increase code size and ISR delay. Rather use functors as defined in
	 * `i2c_debug.h`.
	 * @sa i2c::debug::I2CDebugRecorder
	 * @sa i2c::debug::I2CLiveDebugger
	 */
	using I2C_DEBUG_HOOK = void (*)(DebugStatus status, uint8_t data);

	/// @cond notdocumented
	// Type of commands in queue
	class I2CCommandType
	{
	public:
		constexpr I2CCommandType() = default;
		constexpr I2CCommandType(const I2CCommandType&) = default;
		constexpr I2CCommandType(uint8_t value) : value_{value} {}
		constexpr I2CCommandType(bool write, bool stop, bool finish, bool end)
			:	value_{value(write, stop, finish, end)} {}
		I2CCommandType& operator=(const I2CCommandType&) = default;

		bool is_none() const
		{
			return value_ == NONE;
		}
		bool is_write() const
		{
			return value_ & WRITE;
		}
		bool is_stop() const
		{
			return value_ & STOP;
		}
		bool is_finish() const
		{
			return value_ & FINISH;
		}
		bool is_end() const
		{
			return value_ & END;
		}
		void add_flags(uint8_t value)
		{
			value_ |= value;
		}

		static constexpr uint8_t flags(bool stop, bool finish, bool end)
		{
			return (stop ? STOP : 0) | (finish ? FINISH : 0) | (end ? END : 0);
		}

	private:
		static constexpr const uint8_t NONE = 0;
		static constexpr const uint8_t NOT_NONE = bits::BV8(0);
		static constexpr const uint8_t WRITE = bits::BV8(1);
		static constexpr const uint8_t STOP = bits::BV8(2);
		static constexpr const uint8_t FINISH = bits::BV8(3);
		static constexpr const uint8_t END = bits::BV8(4);

		static constexpr uint8_t value(bool write, bool stop, bool finish, bool end)
		{
			return NOT_NONE | (write ? WRITE : 0) | (stop ? STOP : 0) | (finish ? FINISH : 0) | (end ? END : 0);
		}

		uint8_t value_ = NONE;

		friend bool operator==(const I2CCommandType&, const I2CCommandType&);
		friend bool operator!=(const I2CCommandType&, const I2CCommandType&);
	};

	streams::ostream& operator<<(streams::ostream& out, const I2CCommandType& t);
	bool operator==(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ == b.value_);
	}
	bool operator!=(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ != b.value_);
	}
	/// @endcond

	/**
	 * Light atomic I2C command as prepared by an I2C device.
	 * Each command embeds:
	 * - the command type (read, write...), 
	 * - the count of bytes to be read or  written,
	 * 
	 * @warning You should never need to use this API by yourself. This is 
	 * internally used by FastArduino I2CManager to handle I2C transactions.
	 * 
	 * @sa I2CCommand
	 */
	class I2CLightCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CLightCommand() = default;
		constexpr I2CLightCommand(const I2CLightCommand&) = default;
		constexpr I2CLightCommand(I2CCommandType type, uint8_t byte_count) : type_{type}, byte_count_{byte_count} {}

		I2CCommandType type() const
		{
			return type_;
		}
		I2CCommandType& type()
		{
			return type_;
		}
		uint8_t byte_count() const
		{
			return byte_count_;
		}
		void decrement_byte_count()
		{
			--byte_count_;
		}
		void update_byte_count(uint8_t read_count, uint8_t write_count)
		{
			if (!byte_count_)
				byte_count_ = (type_.is_write() ? write_count : read_count);
		}
		/// @endcond

	private:
		// Type of this command
		I2CCommandType type_ = I2CCommandType{};
		// The number of remaining bytes to be read or write
		uint8_t byte_count_ = 0;
	};

	/**
	 * Atomic I2C command as used internally by an asynchronous I2C Manager.
	 * You shall use it when you define a buffer of commands for an asynchronous
	 * I2C Manager constructor.
	 * 
	 * Each command embeds:
	 * - the command type (read, write...), 
	 * - the count of bytes to be read or  written,
	 * - the address of target slave device
	 * - a proxy to the future holding inputs and results of the I2C transaction 
	 * 
	 * @warning You should never need to use this API by yourself. This is 
	 * internally used by FastArduino I2CManager to handle I2C transactions.
	 * 
	 * @sa lifecycle::LightProxy
	 * @sa future::AbstractFuture
	 */
	class I2CCommand : public I2CLightCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CCommand() = default;
		constexpr I2CCommand(const I2CCommand&) = default;
		constexpr I2CCommand(
			const I2CLightCommand& that, uint8_t target, lifecycle::LightProxy<future::AbstractFuture> future)
			:	I2CLightCommand{that}, target_{target}, future_{future} {}
		constexpr I2CCommand& operator=(const I2CCommand&) = default;

		uint8_t target() const
		{
			return target_;
		}
		lifecycle::LightProxy<future::AbstractFuture> future() const
		{
			return future_;
		}

		void set_target(uint8_t target, lifecycle::LightProxy<future::AbstractFuture> future)
		{
			target_ = target;
			future_ = future;
		}
		/// @endcond

	private:
		// Address of the target device (on 8 bits, already left-shifted)
		uint8_t target_ = 0;
		// A proxy to the future to be used for this command
		lifecycle::LightProxy<future::AbstractFuture> future_;
	};

	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, const I2CCommand& c);
	/// @endcond

	/// @cond notdocumented
	// Generic support for I2C debugging
	template<bool IS_DEBUG_ = false, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>  class I2CDebugSupport
	{
	protected:
		I2CDebugSupport(UNUSED DEBUG_HOOK_ hook = nullptr) {}
		void call_hook(UNUSED DebugStatus status, UNUSED uint8_t data = 0) {}
	};
	// template<> template<typename DEBUG_HOOK_> class I2CDebugSupport<true, DEBUG_HOOK_>
	template<typename DEBUG_HOOK_> class I2CDebugSupport<true, DEBUG_HOOK_>
	{
	protected:
		I2CDebugSupport(DEBUG_HOOK_ hook) : hook_{hook} {}
		void call_hook(DebugStatus status, uint8_t data = 0)
		{
			hook_(status, data);
		}
	private:
		DEBUG_HOOK_ hook_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for LifeCycle resolution
	template<bool HAS_LIFECYCLE_ = false> class I2CLifeCycleSupport
	{
	protected:
		I2CLifeCycleSupport(UNUSED lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr) {}
		template<typename T> T& resolve(lifecycle::LightProxy<T> proxy) const
		{
			return *proxy.destination();
		}
	};
	template<> class I2CLifeCycleSupport<true>
	{
	protected:
		I2CLifeCycleSupport(lifecycle::AbstractLifeCycleManager* lifecycle_manager)
			:	lifecycle_manager_{*lifecycle_manager} {}
		template<typename T> T& resolve(lifecycle::LightProxy<T> proxy) const
		{
			return *proxy(lifecycle_manager_);
		}
	private:
		lifecycle::AbstractLifeCycleManager& lifecycle_manager_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for LifeCycle resolution
	template<I2CErrorPolicy POLICY = I2CErrorPolicy::DO_NOTHING> class I2CErrorPolicySupport
	{
	protected:
		I2CErrorPolicySupport() {}
		void handle_error(UNUSED const I2CCommand& current, UNUSED containers::Queue<I2CCommand>& commands) {}
	};
	template<> class I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	{
	protected:
		I2CErrorPolicySupport() {}
		void handle_error(UNUSED const I2CCommand& current, containers::Queue<I2CCommand>& commands)
		{
			commands.clear_();
		}
	};
	template<> class I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS>
	{
	protected:
		I2CErrorPolicySupport() {}
		void handle_error(const I2CCommand& current, containers::Queue<I2CCommand>& commands)
		{
			// Clear command belonging to the same transaction (i.e. same future)
			const auto future = current.future();
			I2CCommand command;
			while (commands.peek_(command))
			{
				if (command.future() != future)
					break;
				commands.pull_(command);
			}
		}
	};
	/// @endcond

	/**
	 * Abstract I2C Manager.
	 * It is specifically subclassed for ATmega Vs. ATtiny architectures.
	 * You should never need to subclass AbstractBaseI2CManager yourself.
	 * 
	 * For the time being, the MCU must always act as the only master on the bus.
	 * Using MCU as a slave will be supported in a later version of FastArduino.
	 */
	class AbstractBaseI2CManager
	{
	public:
		/**
		 * Return latest transmission status.
		 * Possible statuses are defined in namespace `i2c::Status`.
		 * If latest operation was OK, then `i2c::Status::OK` (`0`) is returned.
		 * Any non zero value indicates an error.
		 * @sa i2c::Status
		 */
		uint8_t status() const
		{
			return status_;
		}

	protected:
		/// @cond notdocumented
		AbstractBaseI2CManager() = default;
		// Latest I2C status
		uint8_t status_ = 0;
		/// @endcond
	};

	/// @cond notdocumented
	// Specific traits for I2CManager
	template<typename T> struct I2CManager_trait
	{
		static constexpr bool IS_I2CMANAGER = false;
		static constexpr bool IS_ASYNC = false;
		static constexpr bool HAS_LIFECYCLE = false;
		static constexpr bool IS_DEBUG = false;
		static constexpr I2CMode MODE = I2CMode::STANDARD;
	};

	template<bool IS_ASYNC_, bool HAS_LIFECYCLE_, bool IS_DEBUG_, I2CMode MODE_>
	struct I2CManager_trait_impl
	{
		static constexpr bool IS_I2CMANAGER = true;
		static constexpr bool IS_ASYNC = IS_ASYNC_;
		static constexpr bool HAS_LIFECYCLE = HAS_LIFECYCLE_;
		static constexpr bool IS_DEBUG = IS_DEBUG_;
		static constexpr I2CMode MODE = MODE_;
	};

	// Specific traits for I2CMode
	template<I2CMode MODE_ = I2CMode::STANDARD> struct I2CMode_trait
	{
		static constexpr I2CMode MODE = MODE_;
		static constexpr uint32_t RATE = 100'000UL;
		static constexpr uint32_t FREQUENCY = (F_CPU / RATE - 16UL) / 2;
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(4.0 + 4.7);
	};
	template<> struct I2CMode_trait<I2CMode::FAST>
	{
		static constexpr I2CMode MODE = I2CMode::FAST;
		static constexpr uint32_t RATE = 400'000UL;
		static constexpr uint32_t FREQUENCY = (F_CPU / RATE - 16UL) / 2;
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(0.6 + 1.3);
	};
	/// @endcond
}

#endif /* I2C_HANDLER_COMMON_HH */
/// @endcond
