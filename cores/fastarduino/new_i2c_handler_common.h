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

	// Used by TWI ISR to potentially call a registered callback
	//TODO move to ATmega?
	//TODO DOC
	enum class I2CCallback : uint8_t
	{
		NONE = 0,
		END_COMMAND,
		END_TRANSACTION,
		ERROR
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
	 * @sa i2c::debug::I2CAsyncDebugger
	 * @sa i2c::debug::I2CSyncDebugger
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

	//TODO check if need to be externalized to cpp file
	streams::ostream& operator<<(streams::ostream& out, const I2CCommandType& t)
	{
		if (t.is_none()) return out << F("NONE") << streams::flush;
		out << (t.is_write() ? F("WRITE") : F("READ"));
		if (t.is_stop())
			out << F("[STOP]");
		if (t.is_finish())
			out << F("[FINISH]");
		if (t.is_end())
			out << F("[END]");
		return out << streams::flush;
	}
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
	 * Atomic I2C command as prepared by an I2C device.
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
	class I2CCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CCommand() = default;
		constexpr I2CCommand(const I2CCommand&) = default;
		constexpr I2CCommand(I2CCommandType type, uint8_t byte_count) : type_{type}, byte_count_{byte_count} {}
		constexpr I2CCommand& operator=(const I2CCommand&) = default;

		I2CCommandType type() const
		{
			return type_;
		}
		I2CCommandType& type()
		{
			return type_;
		}
		uint8_t target() const
		{
			return target_;
		}
		uint8_t byte_count() const
		{
			return byte_count_;
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
		void decrement_byte_count()
		{
			--byte_count_;
		}
		//TODO remove later
		void set_byte_count(uint8_t byte_count)
		{
			byte_count_ = byte_count;
		}
		/// @endcond

	private:
		// Type of this command
		I2CCommandType type_ = I2CCommandType{};
		// The number of remaining bytes to be read or write
		uint8_t byte_count_ = 0;
		// Address of the target device (on 8 bits, already left-shifted)
		uint8_t target_ = 0;
		// A proxy to the future to be used for this command
		lifecycle::LightProxy<future::AbstractFuture> future_;

		friend bool operator==(const I2CCommand&, const I2CCommand&);
		friend bool operator!=(const I2CCommand&, const I2CCommand&);
	};

	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, const I2CCommand& c)
	{
		out	<< '{' << c.type() << ',' 
			<< streams::hex << c.target() << '}' << streams::flush;
		return out;
	}
	/// @endcond

	//TODO refactor to have a common class with everything common (non template)
	//TODO then define a generic template (subclass) with 4 specializations for HAS_LIFECYCLE and IS_DEBUG
	//TODO here MODE_ template arg seems unused?
	/**
	 * Abstract I2C Manager.
	 * It is specifically subclassed for ATmega Vs. ATtiny architectures.
	 * You should never need to subclass AbstractI2CManager yourself.
	 * 
	 * For the time being, the MCU must always act as the only master on the bus.
	 * Using MCU as a slave will be supported in a later version of FastArduino.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam HAS_LIFECYCLE_ tells if this I2CManager must be able to handle 
	 * proxies to Future that can mvoe around and must eb cotnrolled by a 
	 * LifeCycleManager; using `false` will generate smaller code.
	 * @tparam IS_DEBUG_ tells this I2CManager to call a debugging hook at each 
	 * step of an I2C transaction; this is useful for debugging support for a new 
	 * I2C device; using `false` will generate smaller code.
	 * @tparam DEBUG_HOOK_ the type of the hook to be called when `IS_DEBUG` is 
	 * `true`. This can be a simple function pointer (of type `I2C_DEBUG_HOOK`)
	 * or a Functor class (or Functor class reference). Using a Functor class will
	 * generate smaller code.
	 * 
	 * @sa I2CMode
	 * @sa I2CManager
	 * @sa I2C_DEBUG_HOOK
	 */
	template<I2CMode MODE_, bool HAS_LIFECYCLE_ = false, bool IS_DEBUG_ = false, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>
	class AbstractI2CManager
	{
	public:
		/** The I2C mode for this manager. */
		static constexpr const I2CMode MODE = MODE_;

		//TODO DOC
		static constexpr const bool HAS_LIFECYCLE = HAS_LIFECYCLE_;
		static constexpr const bool IS_DEBUG = IS_DEBUG_;
		using DEBUG_HOOK = DEBUG_HOOK_;

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
		AbstractI2CManager(const AbstractI2CManager<MODE_>&) = delete;
		AbstractI2CManager<MODE_>& operator=(const AbstractI2CManager<MODE_>&) = delete;

		AbstractI2CManager(I2CErrorPolicy error_policy)
			:	lifecycle_manager_{nullptr}, error_policy_{error_policy}, hook_{nullptr}
		{
			static_assert(!HAS_LIFECYCLE, "HAS_LIFECYCLE_ must be false with this constructor");
			static_assert(!IS_DEBUG, "IS_DEBUG_ must be false with this constructor");
		}

		AbstractI2CManager(
			lifecycle::AbstractLifeCycleManager* lifecycle_manager, I2CErrorPolicy error_policy)
			:	lifecycle_manager_{lifecycle_manager}, error_policy_{error_policy}, hook_{nullptr}
		{
			static_assert(HAS_LIFECYCLE, "HAS_LIFECYCLE_ must be true with this constructor");
			static_assert(!IS_DEBUG, "IS_DEBUG_ must be false with this constructor");
		}

		AbstractI2CManager(I2CErrorPolicy error_policy, DEBUG_HOOK hook)
			:	lifecycle_manager_{nullptr}, error_policy_{error_policy}, hook_{hook}
		{
			static_assert(!HAS_LIFECYCLE, "HAS_LIFECYCLE_ must be false with this constructor");
			static_assert(IS_DEBUG, "IS_DEBUG_ must be true with this constructor");
		}

		AbstractI2CManager(
			lifecycle::AbstractLifeCycleManager* lifecycle_manager, I2CErrorPolicy error_policy, DEBUG_HOOK hook)
			:	lifecycle_manager_{lifecycle_manager}, error_policy_{error_policy}, hook_{hook}
		{
			static_assert(HAS_LIFECYCLE, "HAS_LIFECYCLE_ must be true with this constructor");
			static_assert(IS_DEBUG, "IS_DEBUG_ must be true with this constructor");
		}

		using TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;

		bool check_no_error()
		{
			if (status_ == expected_status_) return true;
			// Handle special case of last transmitted byte possibly not acknowledged by device
			if (	(expected_status_ == Status::DATA_TRANSMITTED_ACK)
				&&	(status_ == Status::DATA_TRANSMITTED_NACK)
				&&	(command_.byte_count() == 0))
				return true;

			// When status is FUTURE_ERROR then future has already been marked accordingly
			if (status_ != Status::FUTURE_ERROR)
				// The future must be marked as error
				current_future().set_future_error_(errors::EPROTO);
			return false;
		}

		template<typename T> T& resolve(lifecycle::LightProxy<T> proxy) const
		{
			if (HAS_LIFECYCLE)
				return *proxy(lifecycle_manager_);
			else
				return *proxy.destination();
		}

		future::AbstractFuture& current_future() const
		{
			return resolve(command_.future());
		}

		void call_hook(DebugStatus status, uint8_t data = 0)
		{
			if (IS_DEBUG)
				hook_(status, data);
		}

		static constexpr const uint32_t STANDARD_FREQUENCY = (F_CPU / 100'000UL - 16UL) / 2;
		static constexpr const uint32_t FAST_FREQUENCY = (F_CPU / 400'000UL - 16UL) / 2;

		lifecycle::AbstractLifeCycleManager* lifecycle_manager_;
		const I2CErrorPolicy error_policy_;
		DEBUG_HOOK hook_;

		// Status of current command processing
		I2CCommand command_;
		uint8_t expected_status_ = 0;

		// Latest I2C status
		uint8_t status_ = 0;
	};

	/// @cond notdocumented
	// Specific traits for I2CManager
	template<typename T> struct I2CManager_trait
	{
		static constexpr bool IS_I2CMANAGER = false;
		static constexpr bool IS_DEBUG = false;
		static constexpr bool HAS_LIFECYCLE = false;
	};

	template<bool HAS_LIFECYCLE_, bool IS_DEBUG_> struct I2CManager_trait_impl
	{
		static constexpr bool IS_I2CMANAGER = true;
		static constexpr bool IS_DEBUG = IS_DEBUG_;
		static constexpr bool HAS_LIFECYCLE = HAS_LIFECYCLE_;
	};
	/// @endcond
}

#endif /* I2C_HANDLER_COMMON_HH */
/// @endcond
