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
	 * List of debug states that are reported by the I2C Manager in debug mode.
	 * 
	 * @sa I2CSyncDebugManager
	 * @sa I2CAsyncDebugManager
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
		/** I2C Manager has acknowledged the latest received byte from the slave. */
		RECV_OK,
		/** I2C Manager has not acknowledged the latest received byte from the slave. */
		RECV_ERROR
	};

	/**
	 * The default debugging hook type.
	 * @warning Do not use this (function pointer) for your hooks! This will 
	 * increase code size and ISR delay. Rather use functors as defined in
	 * `i2c_debug.h`.
	 * 
	 * @sa I2CSyncDebugManager
	 * @sa I2CAsyncDebugManager
	 * @sa i2c::debug::I2CDebugRecorder
	 * @sa i2c::debug::I2CDebugLiveLogger
	 */
	using I2C_DEBUG_HOOK = void (*)(DebugStatus status, uint8_t data);

	/**
	 * The default status observer hook type.
	 * @warning Do not use this (function pointer) for your hooks! This will 
	 * increase code size and ISR delay. Rather use functors as defined in
	 * `i2c_status.h`.
	 * 
	 * @sa I2CSyncStatusManager
	 * @sa I2CAsyncStatusManager
	 * @sa i2c::debug::I2CDebugRecorder
	 * @sa i2c::debug::I2CDebugLiveLogger
	 */
	using I2C_STATUS_HOOK = void (*)(Status expected, Status actual);

	/// @cond notdocumented
	// Type of commands in queue
	class I2CCommandType
	{
	public:
		constexpr I2CCommandType() = default;
		constexpr I2CCommandType(const I2CCommandType&) = default;
		explicit constexpr I2CCommandType(uint8_t value) : value_{value} {}
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
	bool operator==(const I2CCommandType& a, const I2CCommandType& b);
	bool operator!=(const I2CCommandType& a, const I2CCommandType& b);
	/// @endcond

	/**
	 * Light atomic I2C command as prepared by an I2C device.
	 * Each command embeds:
	 * - the command type (read, write...), 
	 * - the count of bytes to be read or  written,
	 * 
	 * @warning You should never need to use this API by yourself. This is 
	 * internally used by FastArduino I2C Manager to handle I2C transactions.
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
	 * internally used by FastArduino I2C Manager to handle I2C transactions.
	 */
	template<typename T> class I2CCommand : public I2CLightCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CCommand() = default;
		constexpr I2CCommand(const I2CCommand&) = default;
		constexpr I2CCommand(
			const I2CLightCommand& that, uint8_t target, T future)
			:	I2CLightCommand{that}, target_{target}, future_{future} {}
		constexpr I2CCommand& operator=(const I2CCommand&) = default;

		uint8_t target() const
		{
			return target_;
		}
		T future() const
		{
			return future_;
		}

		void set_target(uint8_t target, T future)
		{
			target_ = target;
			future_ = future;
		}
		/// @endcond

	private:
		// Address of the target device (on 8 bits, already left-shifted)
		uint8_t target_ = 0;
		// A proxy to the future to be used for this command
		T future_{};
	};

	/// @cond notdocumented
	template<typename T>
	streams::ostream& operator<<(streams::ostream& out, const I2CCommand<T>& c)
	{
		out	<< '{' << c.type() << ',' 
			<< streams::hex << c.target() << '}' << streams::flush;
		return out;
	}
	/// @endcond

	/// @cond notdocumented
	// Generic support for I2C debugging
	template<bool IS_DEBUG_ = false, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>  struct I2CDebugSupport
	{
		explicit I2CDebugSupport(UNUSED DEBUG_HOOK_ hook = nullptr) {}
		void call_hook(UNUSED DebugStatus status, UNUSED uint8_t data = 0)
		{
			// Intentionally left empty
		}
	};
	template<typename DEBUG_HOOK_> struct I2CDebugSupport<true, DEBUG_HOOK_>
	{
		explicit I2CDebugSupport(DEBUG_HOOK_ hook) : hook_{hook} {}
		void call_hook(DebugStatus status, uint8_t data = 0)
		{
			hook_(status, data);
		}
	private:
		DEBUG_HOOK_ hook_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for I2C status hook
	template<bool IS_STATUS_ = false, typename STATUS_HOOK_ = I2C_STATUS_HOOK>  struct I2CStatusSupport
	{
		explicit I2CStatusSupport(UNUSED STATUS_HOOK_ hook = nullptr) {}
		void call_hook(UNUSED Status expected, UNUSED Status actual)
		{
			// Intentionally left empty
		}
	};
	template<typename STATUS_HOOK_> struct I2CStatusSupport<true, STATUS_HOOK_>
	{
		explicit I2CStatusSupport(STATUS_HOOK_ hook) : hook_{hook} {}
		void call_hook(Status expected, Status actual)
		{
			hook_(expected, actual);
		}
	private:
		STATUS_HOOK_ hook_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for LifeCycle resolution
	template<bool HAS_LIFECYCLE_ = false> struct I2CLifeCycleSupport
	{
		template<typename T> using PROXY = lifecycle::DirectProxy<T>;
		template<typename T> static PROXY<T> make_proxy(const T& dest)
		{
			return lifecycle::make_direct_proxy(dest);
		}
		explicit I2CLifeCycleSupport(UNUSED lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr) {}
		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return *proxy();
		}
	};
	template<> struct I2CLifeCycleSupport<true>
	{
		template<typename T> using PROXY = lifecycle::LightProxy<T>;
		template<typename T> static PROXY<T> make_proxy(const T& dest)
		{
			return lifecycle::make_light_proxy(dest);
		}
		explicit I2CLifeCycleSupport(lifecycle::AbstractLifeCycleManager* lifecycle_manager)
			:	lifecycle_manager_{*lifecycle_manager} {}
		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return *proxy(lifecycle_manager_);
		}
	private:
		lifecycle::AbstractLifeCycleManager& lifecycle_manager_;
	};
	/// @endcond

	/// @cond notdocumented
	// Specific traits for I2CMode
	template<I2CMode MODE_ = I2CMode::STANDARD> struct I2CMode_trait
	{
		static constexpr I2CMode MODE = MODE_;
		static constexpr uint32_t RATE = 100'000UL;
		static constexpr uint32_t FREQUENCY = ((F_CPU / RATE) - 16UL) / 2;
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
		static constexpr uint32_t FREQUENCY = ((F_CPU / RATE) - 16UL) / 2;
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(0.6 + 1.3);
	};
	/// @endcond

	/**
	 * Abstract synchronous I2C Manager for all MCU architectures.
	 * A specifi abstract subclass is defined for each MCU architecture 
	 * (ATmega, ATtiny).
	 * You should never need to subclass AbstractI2CSyncManager yourself.
	 * 
	 * @tparam ARCH_HANDLER_ the type of an actual class handling I2C control
	 * on actual target architecture
	 * @tparam MODE_ the I2C mode for this manager
	 * @tparam HAS_LC_ tells if this I2C Manager must be able to handle 
	 * proxies to Future that can move around and must be controlled by a 
	 * LifeCycleManager; using `false` will generate smaller code.
	 * @tparam STATUS_HOOK_ the type of the hook to be called. This can be a 
	 * simple function pointer (of type `I2C_STATUS_HOOK`) or a Functor class 
	 * (or Functor class reference). Using a Functor class will generate smaller
	 * code.
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
	 * 
	 */
	// Abstract generic class for synchronous I2C management
	template<typename ARCH_HANDLER_, I2CMode MODE_, bool HAS_LC_, 
		typename STATUS_HOOK_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CSyncManager
	{
	protected:
		using ARCH_HANDLER = ARCH_HANDLER_;
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using DEBUG = I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>;
		using LC = I2CLifeCycleSupport<HAS_LC_>;
		using ABSTRACT_FUTURE = future::AbstractFakeFuture;
		template<typename T> using PROXY = typename LC::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = future::FakeFuture<OUT, IN>;

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
			handler_.begin_();
		}

		/**
		 * Disable MCU I2C transmission.
		 * This method is NOT synchronized.
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			handler_.end_();
		}

	protected:
		/// @cond notdocumented
		explicit AbstractI2CSyncManager(
			lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr, 
			STATUS_HOOK_ status_hook = nullptr,
			DEBUG_HOOK_ debug_hook = nullptr)
			:	handler_{status_hook}, lc_{lifecycle_manager}, debug_hook_{debug_hook} {}
		/// @endcond

	private:
		bool ensure_num_commands_(UNUSED uint8_t num_commands) const
		{
			return true;
		}

		bool push_command_(
			I2CLightCommand command, uint8_t target, PROXY<ABSTRACT_FUTURE> proxy)
		{
			// Check command is not empty
			const I2CCommandType type = command.type();
			if (type.is_none()) return true;
			if (clear_commands_) return false;
			ABSTRACT_FUTURE& future = lc_.resolve(proxy);
			// Execute command immediately, from start to optional stop
			bool ok = (no_stop_ ? exec_repeat_start_() : exec_start_());
			stopped_already_ = false;
			if (!ok)
				return handle_error(future);

			if (type.is_write())
			{
				// Send device address
				if (!exec_send_slaw_(target))
					return handle_error(future);
				// Send content
				while (command.byte_count() > 0)
					// In case of a NACK on data writing, we check if it is not last byte
					if ((!exec_send_data_(command, future)) && (command.byte_count() > 0))
						return handle_error(future);
			}
			else
			{
				// Send device address
				if (!exec_send_slar_(target))
					return handle_error(future);
				// Receive content
				while (command.byte_count() > 0)
					if (!exec_receive_data_(command, future))
						return handle_error(future);
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
			{
				exec_stop_();
				no_stop_ = false;
			}
			clear_commands_ = false;
			stopped_already_ = false;
		}

		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return lc_.resolve(proxy);
		}
		
		// Low-level methods to handle the bus in an asynchronous way
		bool exec_start_()
		{
			debug_hook_.call_hook(DebugStatus::START);
			return handler_.exec_start_();
		}

		bool exec_repeat_start_()
		{
			debug_hook_.call_hook(DebugStatus::REPEAT_START);
			return handler_.exec_repeat_start_();
		}

		bool exec_send_slar_(uint8_t target)
		{
			debug_hook_.call_hook(DebugStatus::SLAR, target);
			return handler_.exec_send_slar_(target);
		}
		
		bool exec_send_slaw_(uint8_t target)
		{
			debug_hook_.call_hook(DebugStatus::SLAW, target);
			return handler_.exec_send_slaw_(target);
		}

		bool exec_send_data_(I2CLightCommand& command, ABSTRACT_FUTURE& future)
		{
			// Determine next data byte
			uint8_t data = 0;
			bool ok = future.get_storage_value_(data);
			debug_hook_.call_hook(DebugStatus::SEND, data);
			debug_hook_.call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (!ok)
			{
				future.set_future_error_(errors::EILSEQ);
				return false;
			}
			command.decrement_byte_count();
			return handler_.exec_send_data_(data);
		}

		bool exec_receive_data_(I2CLightCommand& command, ABSTRACT_FUTURE& future)
		{
			// Is this the last byte to receive?
			const bool last_byte = (command.byte_count() == 1);
			const DebugStatus debug = (last_byte ? DebugStatus::RECV_LAST : DebugStatus::RECV);
			debug_hook_.call_hook(debug);

			uint8_t data;
			if (handler_.exec_receive_data_(last_byte, data))
			{
				const bool ok = future.set_future_value_(data);
				debug_hook_.call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
				// This should only happen in case there are 2 concurrent providers for this future
				if (ok)
				{
					command.decrement_byte_count();
				}
				else
				{
					future.set_future_error_(errors::EILSEQ);
				}
				return ok;
			}
			return false;
		}

		void exec_stop_()
		{
			debug_hook_.call_hook(DebugStatus::STOP);
			handler_.exec_stop_();
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(MODE_TRAIT::DELAY_AFTER_STOP);
			stopped_already_ = true;
		}

		// This method is called when an error has occurred
		bool handle_error(ABSTRACT_FUTURE& future)
		{
			if (future.status() != future::FutureStatus::ERROR)
				// The future must be marked as error
				future.set_future_error_(errors::EPROTO);

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

		ARCH_HANDLER handler_;
		LC lc_;
		DEBUG debug_hook_;

		template<typename> friend class I2CDevice;
	};

	/// @cond notdocumented
	// Specific traits for I2C Manager
	template<typename T> struct I2CManager_trait
	{
		static constexpr bool IS_I2CMANAGER = false;
		static constexpr bool IS_ASYNC = false;
		static constexpr bool HAS_LIFECYCLE = false;
		static constexpr bool IS_DEBUG = false;
		static constexpr bool IS_STATUS = false;
		static constexpr I2CMode MODE = I2CMode::STANDARD;
	};

	template<bool IS_ASYNC_, bool HAS_LIFECYCLE_, bool IS_STATUS_, bool IS_DEBUG_, I2CMode MODE_>
	struct I2CManager_trait_impl
	{
		static constexpr bool IS_I2CMANAGER = true;
		static constexpr bool IS_ASYNC = IS_ASYNC_;
		static constexpr bool HAS_LIFECYCLE = HAS_LIFECYCLE_;
		static constexpr bool IS_DEBUG = IS_DEBUG_;
		static constexpr bool IS_STATUS = IS_STATUS_;
		static constexpr I2CMode MODE = MODE_;
	};
	/// @endcond
}

#endif /* I2C_HANDLER_COMMON_HH */
/// @endcond
