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
 * I2C Device API.
 */
#ifndef I2C_DEVICE_HH
#define I2C_DEVICE_HH

#include "initializer_list.h"
#include "iterator.h"
#include "errors.h"
#include "i2c.h"
#include "future.h"
#include "lifecycle.h"
#include "i2c_handler.h"
#include "utilities.h"

namespace i2c
{
	/// @cond notdocumented
	// Forward declaration
	template<typename MANAGER> class I2CFutureHelper;
	/// @endcond

	// Trick to support MODE as I2CDevice constructor template argument (deducible)
	/// @cond notdocumented
	template<I2CMode MODE> struct Mode {};
	/// @endcond

	/** Constant determining that best supported I2C mode for an I2CDevice is STANDARD (100kHz). */
	static constexpr Mode I2C_STANDARD = Mode<I2CMode::STANDARD>{};
	/** Constant determining that best supported I2C mode for an I2CDevice is FAST (400kHz). */
	static constexpr Mode I2C_FAST = Mode<I2CMode::FAST>{};
	
	// Internal (private) utility class to start/end synchronization or do nothing at all
	/// @cond notdocumented
	template<bool DISABLE = true> class DisableInterrupts
	{
		DisableInterrupts()
		{
			cli();
		}
		~DisableInterrupts()
		{
			SREG = sreg_;
		}

		const uint8_t sreg_ = SREG;

		template<typename> friend class I2CDevice;
	};
	template<> class DisableInterrupts<false>
	{
		DisableInterrupts() = default;

		template<typename> friend class I2CDevice;
	};
	/// @endcond

	/**
	 * Base class for all I2C devices.
	 * 
	 * @tparam MANAGER_ the type of I2C Manager used to handle I2C communication
	 * 
	 * @sa i2c::I2CSyncManager
	 * @sa i2c::I2CAsyncManager
	 */
	template<typename MANAGER_>
	class I2CDevice
	{
	public:
		/** the type of I2C Manager that can handle this device. */
		using MANAGER = MANAGER_;

	private:
		using MANAGER_TRAIT = I2CManager_trait<MANAGER>;
		// Ensure MANAGER is an accepted I2C Manager type
		static_assert(
			MANAGER_TRAIT::IS_I2CMANAGER, "MANAGER_ must be a valid I2C Manager type");

	protected:
		/**
		 * The actual type used for all proxies; may be `lifecycle::LightProxy` 
		 * or `lifecycle::DirectProxy`. This is defined by MANAGER type, whether
		 * it uses a lifecycle::AbstractLifeCycleManager or not.
		 */
		template<typename T> using PROXY = typename MANAGER::template PROXY<T>;

		/**
		 * The abstract base class of all futures to be defined for a device.
		 * This may be `future::AbstractFuture` or `future::AbstractFakeFuture`;
		 * this is defined by MANAGER type, whether it is asynchronous or synchronous.
		 */
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;

		/**
		 * The template base class of all futures to be defined for a device.
		 * This may be `future::Future` or `future::FakeFuture`; this is defined
		 * by MANAGER type, whether it is asynchronous or synchronous.
		 */
		template<typename OUT, typename IN> using FUTURE = typename MANAGER::template FUTURE<OUT, IN>;

		/**
		 * Create a new I2C device. This constructor must be called by a subclass
		 * implementing an actua I2C device.
		 * 
		 * @param manager the I2C Manager that is in charge of I2C bus
		 * @param device the 8-bits device address on the I2C bus; it is constructed 
		 * from the actual 7-bits address, after left-shifting 1 bit. This can be 
		 * changed dynamically later for devices that support address changes.
		 * @param mode  the best I2C mode for this device; this determines the 
		 * I2C Manager types that can manage this device.
		 * @param auto_stop if `true`, then any chain of commands (started with 
		 * `launch_commands()`) will end with a STOP condition generated on the I2C bus;
		 * if `false`, then STOP condition will generated only when
		 * requested in `read()` or `write()` calls)
		 * 
		 * @sa set_device()
		 */
		template<I2CMode MODE>
		I2CDevice(MANAGER& manager, uint8_t device, UNUSED Mode<MODE> mode, bool auto_stop) 
		: device_{device}, handler_{manager}, auto_stop_flags_{I2CCommandType::flags(auto_stop, true, true)}
		{
			// Ensure that MANAGER I2C mode is compliant with the best mode for this device
			static_assert((MODE == I2CMode::FAST) || (MODE == MANAGER_TRAIT::MODE),
				"MANAGER_ I2CMode must be compliant with this device best mode");
		}

		I2CDevice(const I2CDevice&) = delete;
		I2CDevice& operator=(const I2CDevice&) = delete;

		/**
		 * Change the I2C address of this device.
		 * 
		 * @param device the 8-bits device address on the I2C bus; it is constructed 
		 * from the actual 7-bits address, after left-shifting 1 bit. This can be 
		 * changed dynamically later for devices that support address changes.
		 */
		void set_device(uint8_t device)
		{
			device_ = device;
		}

		/**
		 * Build a read I2CLightCommand that can be later pushed to the I2C Manager for
		 * proper handling.
		 * Calling this method has no effect on the bus until the returned command 
		 * is actually pushed to the I2C Manager through a `launch_commands()` call.
		 * 
		 * @param read_count the number of bytes to read from the device to fill
		 * the output value in the Future associated with the I2C transaction; 
		 * if `0`, the whole output value should be filled by this command.
		 * @param finish_future force finishing the Future associated with the created
		 * read I2C command; in general, you would never use this value in your own
		 * implementation for a device, because FastArduino I2C support already 
		 * ensures this is done at the end of a complete I2C transaction (with 
		 * mutiple read and write commands); this could be useful in the exceptional 
		 * case where your I2C transaction is made of many commands, but you would 
		 * like to finish the associated Future **before** the last command is 
		 * executed, which should be an extraordinarily rare situation.
		 * @param stop force a STOP condition on the I2C bus at the end of this command
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 */
		static constexpr I2CLightCommand read(uint8_t read_count = 0, bool finish_future = false, bool stop = false)
		{
			const I2CCommandType type{false, stop, finish_future, false};
			return I2CLightCommand{type, read_count};
		}

		/**
		 * Build a write I2CLightCommand that can be later pushed to the I2C Manager for
		 * proper handling.
		 * Calling this method has no effect on the bus until the returned command 
		 * is actually pushed to the I2C Manager through a `launch_commands()` call.
		 * 
		 * @param write_count the number of bytes to get from the storage value
		 * in the Future associated with the I2C transaction, in order to write 
		 * them to the device; if `0`, the whole storage value should be used by 
		 * this command.
		 * @param finish_future force finishing the Future associated with the created
		 * write I2C command; in general, you would never use this value in your own
		 * implementation for a device, because FastArduino I2C support already 
		 * ensures this is done at the end of a complete I2C transaction (with 
		 * mutiple read and write commands); this could be useful in the exceptional 
		 * case where your I2C transaction is made of many commands, but you would 
		 * like to finish the associated Future **before** the last command is 
		 * executed, which should be an extraordinarily rare situation.
		 * @param stop force a STOP condition on the I2C bus at the end of this command
		 * 
		 * @sa launch_commands()
		 * @sa read()
		 */
		static constexpr I2CLightCommand write(uint8_t write_count = 0, bool finish_future = false, bool stop = false)
		{
			const I2CCommandType type{true, stop, finish_future, false};
			return I2CLightCommand{type, write_count};
		}

		/**
		 * Launch execution (asynchronously or synchronously, depending on MANAGER)
		 * of a chain of I2CLightCommand items (constructed with `read()` and `write()`
		 * methods).
		 * 
		 * With an asynchronous MANAGER, the method returns immediately and one has
		 * to use @p future status to know when all @p commands have been executed.
		 * 
		 * With a synchronous MANAGER, this command is blocking and returns only
		 * once all @p commands have been executed on the I2C bus.
		 * 
		 * I2C commands execution is based on a Future that is used to:
		 * - provide data to write commands
		 * - store data returned by read commands
		 * 
		 * @param proxy a proxy (actual type defined by `PROXY` alias) to the Future
		 * containing all write data and ready to store all read data; it is shared
		 * by all @p commands ; passing a Future will be accepted as the compiler 
		 * will automatically construct the proper proxy for it.
		 * @param commands the list of I2CCommand to be executed, one after another;
		 * this list must be embedded within braces {}.
		 * 
		 * @retval 0 when the method did not encounter any error
		 * @retval errors::EINVAL if passed arguments are invalid e.g. if @p commands
		 * is empty, if @p proxy is dynamic but associated @p MANAGER does not
		 * have a lifecycle::LifeCycleManager, or if the total number of bytes read
		 * or written by all @p commands does not match @p proxy future input and
		 * output sizes.
		 * @retval errors::EAGAIN if the associated @p MANAGER has not enough space
		 * in its queue of commands; in such situation, you may retry the same call
		 * at a later time.
		 * @retval errors::EPROTO if an error occured during command execution
		 * 
		 * @sa read()
		 * @sa write()
		 * @sa errors
		 */
		int launch_commands(
			PROXY<ABSTRACT_FUTURE> proxy, utils::range<I2CLightCommand> commands)
		{
			uint8_t num_commands = commands.size();
			if (num_commands == 0) return errors::EINVAL;
			{
				// Truly asynchronous mode (ATmega only)
				// The whole code block in launch_commands() must be synchronized
				UNUSED auto outer_sync = DisableInterrupts<MANAGER_TRAIT::IS_ASYNC>{};
				uint8_t max_read;
				uint8_t max_write;
				{
					// Synchronous mode
					// Only the next few method calls shall be synchronized
					UNUSED auto inner_sync = DisableInterrupts<!MANAGER_TRAIT::IS_ASYNC>{};
					// pre-conditions (must be synchronized)
					if (!handler_.ensure_num_commands_(num_commands)) return errors::EAGAIN;
					ABSTRACT_FUTURE& future = resolve(proxy);
					max_read = future.get_future_value_size_();
					max_write = future.get_storage_value_size_();
				}

				// That check is normally usefull only in debug mode
				if (MANAGER_TRAIT::IS_DEBUG && (!check_commands(max_write, max_read, commands)))
						return errors::EINVAL;

				// Now push each command to the I2C Manager
				int error = 0;
				for (I2CLightCommand command : commands)
				{
					// update command.byte_count if 0
					command.update_byte_count(max_read, max_write);
					// force future finish for last command in transaction
					--num_commands;
					if (num_commands == 0)
						command.type().add_flags(auto_stop_flags_);
					// Note: on ATtiny, this method blocks until I2C command is finished!
					if (!handler_.push_command_(command, device_, proxy))
					{
						error = errors::EPROTO;
						break;
					}
				}
				// Notify handler that transaction is complete
				handler_.last_command_pushed_();
				return error;
			}
		}

		/**
		 * Helper method that asynchronously launches I2C commands for a simple
		 * Future performing one write followed by one read (typically for device
		 * register reading).
		 * @warning Asynchronous API!
		 * 
		 * @tparam F the type of @p future automatically deduced from @p future
		 * @param future a proxy to the Future to be updated by the launched I2C 
		 * commands
		 * @param stop force a STOP condition on the I2C bus at the end of the
		 * read command
		 * @return the same result codes as `launch_commands()`
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa read()
		 * @sa sync_read()
		 */
		template<typename F> int async_read(PROXY<F> future, bool stop = true)
		{
			return launch_commands(future, {write(), read(0, false, stop)});
		}

		/**
		 * Helper method that launches I2C commands for a simple Future performing
		 * one write followed by one read (typically for device register reading);
		 * the method blocks until the end of the I2C transaction.
		 * @warning Blocking API!
		 * 
		 * @tparam F the type of Future to be used for the I2C transaction; this
		 * template argument must be provided as it cannot be deduced from other
		 * arguments.
		 * @tparam T the type of expected @p result automatically deduced from 
		 * @p result
		 * @param result the result read from the I2C transaction
		 * @retval true if the action was performed successfully, i.e. @p result
		 * contains a correct value
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa read()
		 * @sa async_read()
		 */
		template<typename F, typename T = uint8_t> bool sync_read(T& result)
		{
			F future{};
			if (async_read<F>(future) != 0) return false;
			return future.get(result);
		}

		/**
		 * Helper method that asynchronously launches I2C commands for a simple
		 * Future performing only one write (typically for device register writing).
		 * @warning Asynchronous API!
		 * 
		 * @tparam F the type of @p future automatically deduced from @p future
		 * @param future a proxy to the Future to be updated by the launched I2C 
		 * commands
		 * @param stop force a STOP condition on the I2C bus at the end of the
		 * write command
		 * @return the same result codes as `launch_commands()`
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa sync_write()
		 */
		template<typename F> int async_write(PROXY<F> future, bool stop = true)
		{
			return launch_commands(future, {write(0, false, stop)});
		}

		/**
		 * Helper method that asynchronously launches I2C commands for a simple
		 * Future performing several register writes.
		 * @warning Asynchronous API!
		 * 
		 * @tparam F the type of @p future automatically deduced from @p future;
		 * this must be a `TWriteMultiRegisterFuture` specialization or a subclass.
		 * @param future a proxy to the Future to be updated by the launched I2C 
		 * commands
		 * @param stop force a STOP condition on the I2C bus at the end of each
		 * write command
		 * @return the same result codes as `launch_commands()`
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa TWriteMultiRegisterFuture()
		 */
		template<typename F> int async_multi_write(PROXY<F> future, bool stop = true)
		{
			constexpr uint8_t NUM_WRITES = F::NUM_WRITES;
			constexpr uint8_t WRITE_SIZE = F::WRITE_SIZE;
			I2CLightCommand writes[NUM_WRITES];
			prepare_multi_write_commands(writes, NUM_WRITES, WRITE_SIZE, stop);
			return launch_commands(future, utils::range(writes, NUM_WRITES));
		}

		/**
		 * Helper method that launches I2C commands for a simple Future performing
		 * only one write (typically for device register writing); the method blocks
		 * until the end of the I2C transaction.
		 * @warning Blocking API!
		 * 
		 * @tparam F the type of Future to be used for the I2C transaction; this
		 * template argument must be provided as it cannot be deduced from other
		 * arguments. This must have a constructor with one argument of type @p T.
		 * @tparam T the type of @p value automatically deduced from @p value
		 * @param value the value to write in the I2C transaction
		 * @retval true if the action was performed successfully, i.e. @p value
		 * was correctly transmitted to the device
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa async_write()
		 */
		template<typename F, typename T = uint8_t> bool sync_write(const T& value)
		{
			F future{value};
			if (async_write<F>(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Helper method that launches I2C commands for a simple Future performing
		 * only one write (typically for device register writing); the method blocks
		 * until the end of the I2C transaction.
		 * @warning Blocking API!
		 * 
		 * @tparam F the type of Future to be used for the I2C transaction; this
		 * template argument must be provided as it cannot be deduced from other
		 * arguments. This must have a default constructor.
		 * @retval true if the action was performed successfully, i.e. @p value
		 * was correctly transmitted to the device
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 * @sa async_write()
		 */
		template<typename F> bool sync_write()
		{
			F future{};
			if (async_write<F>(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Resolve @p proxy to an actual @p T (typically a `Future`).
		 * @tparam T the type pointed to by @p proxy
		 * @param proxy the proxy (actual type defined by `PROXY` alias) to a 
		 * Future to resolve
		 * @return a reference to the proxied Future
		 */
		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return handler_.resolve(proxy);
		}

		/**
		 * Create a PROXY from @p target.
		 * Depending on actual PROXY type, that may lead to a lifecycle::LightProxy
		 * or a lifecycle::DirectProxy.
		 * This can be used by device methods working in blocking mode.
		 */
		template<typename T> static PROXY<T> make_proxy(const T& target)
		{
			return MANAGER::LC::make_proxy(target);
		}

	private:
		static bool check_commands(
			uint8_t max_write, uint8_t max_read, const utils::range<I2CLightCommand>& commands)
		{
			// Limit total number of bytes read or written in a transaction to 255
			uint8_t total_read = 0;
			uint8_t total_write = 0;
			for (const I2CLightCommand& command : commands)
			{
				// Count number of bytes read and written
				if (command.type().is_write())
					total_write += (command.byte_count() ? command.byte_count() : max_write);
				else
					total_read += (command.byte_count() ? command.byte_count() : max_read);
			}
			// check sum of read commands byte_count matches future output size
			// check sum of write commands byte_count matches future input size
			return (total_write == max_write) && (total_read == max_read);
		}

		void prepare_multi_write_commands(I2CLightCommand* commands, uint8_t count, uint8_t write_size, bool stop)
		{
			I2CLightCommand command = write(write_size, false, stop);
			for (uint8_t i = 0; i < count; ++i)
				*commands++ = command;
		}

		uint8_t device_ = 0;
		MANAGER& handler_;
		const uint8_t auto_stop_flags_;
		friend class I2CFutureHelper<MANAGER>;
	};
}

#endif /* I2C_DEVICE_HH */
/// @endcond
