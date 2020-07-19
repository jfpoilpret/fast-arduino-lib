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
 * I2C Device API.
 */
#ifndef I2C_DEVICE_HH
#define I2C_DEVICE_HH

#include "initializer_list.h"
#include "errors.h"
#include "i2c.h"
#include "future.h"
#include "lifecycle.h"
#include "new_i2c_handler.h"

/// @cond notdocumented
#if defined(TWCR)
// Truly asynchronous mode (ATmega only)
// The whole code block in launch_commands() must be synchronized
#define OUTER_SYNCHRONIZED synchronized
#define INNER_SYNCHRONIZED
#else
// Fake asynchronous mode (ATtiny)
// Only a few method calls in launch_commands() shall be synchronized
#define INNER_SYNCHRONIZED synchronized
#define OUTER_SYNCHRONIZED
#endif
/// @endcond

namespace i2c
{
	/**
	 * Action(s) to perform at the end of an I2C read or write command.
	 * Values can be or'ed together to combine several actions.
	 * 
	 * @sa I2CDevice::read()
	 * @sa I2CDevice::write()
	 * @sa I2CCommand
	 * @sa I2CCommandType
	 */
	enum class I2CFinish : uint8_t
	{
		/** Perform no specific action ate the end of a read or write I2C command. */
		NONE = 0,
		/**
		 * Force an I2C STOP condition at the end of a read or write command, 
		 * instead of the default REPEAT START condition.
		 * This may be useful for some devices that do not support REEAT START well.
		 */
		FORCE_STOP = 0x01,
		/**
		 * Force finishing the Future associated with the current read or write 
		 * I2C command. This is useful for I2C transaction with only write commands,
		 * hence associated to a Future with no output.
		 * @warning
		 * In general, you would never use this value in your own implementation 
		 * for a device, because FastArduino I2C support already ensures this is
		 * done at the end of a complete I2C transaction (with mutiple read and 
		 * write commands).
		 * This could be useful in the exceptional case where your I2C transaction
		 * is made of many commands, but you would like to finish the associated 
		 * Future **before** the last command is executed, which should be an
		 * extraordinarily rare situation.
		 */
		FUTURE_FINISH = 0x02
	};
	
	I2CFinish operator|(I2CFinish f1, I2CFinish f2)
	{
		return I2CFinish(uint8_t(f1) | uint8_t(f2));
	}
	I2CFinish operator&(I2CFinish f1, I2CFinish f2)
	{
		return I2CFinish(uint8_t(f1) & uint8_t(f2));
	}

	/**
	 * Base class for all I2C devices.
	 * 
	 * @tparam MODE_ the I2C mode for this device; this determines the `I2CManager` type
	 * that can manage this device.
	 * @sa i2c::I2CMode
	 * @sa i2c::I2CManager
	 */
	template<I2CMode MODE_>
	class I2CDevice
	{
	public:
		/** the I2C mode for this device. */
		static constexpr const I2CMode MODE = MODE_;
		/** the type of `I2CManager` that can handle this device. */
		using MANAGER = I2CManager<MODE>;

	protected:
		/**
		 * Create a new I2C device. This constructor must be called by a subclass
		 * implementing an actua I2C device.
		 * @param manager the I2C Manager that is in charge of I2C bus
		 * @param device the 8-bits device address on the I2C bus; it is constructed 
		 * from the actual 7-bits address, after left-shifting 1 bit. This can be 
		 * changed dynamically later for devices that support address changes.
		 * 
		 * @sa set_device()
		 */
		I2CDevice(MANAGER& manager, uint8_t device)
		: device_{device}, handler_{manager} {}

		I2CDevice(const I2CDevice<MODE>&) = delete;
		I2CDevice<MODE>& operator=(const I2CDevice<MODE>&) = delete;

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
		 * Build a read I2CCommand that can be later pushed to the I2CManager for
		 * proper handling.
		 * Calling this method has no effect on the bus until the returned command 
		 * is actually pushed to the I2CManager through a `launch_commands()` call.
		 * 
		 * @param read_count the number of bytes to read from the device to fill
		 * the output value in the Future associated with the I2C transaction; 
		 * if `0`, the whole output value should be filled by this command.
		 * @param finish specify the behavior to adopt after this command has been
		 * fully handled through the I2C bus; it allows to ensure a STOP condition
		 * will be sent on the bus (independently of the existence of further commands),
		 * or force the end of the Future associated to the chain of I2C commands.
		 * 
		 * @sa launch_commands()
		 * @sa write()
		 */
		I2CCommand read(uint8_t read_count = 0, I2CFinish finish = I2CFinish::NONE) const
		{
			return I2CCommand::read(device_, uint8_t(finish & I2CFinish::FORCE_STOP), 
				uint8_t(finish & I2CFinish::FUTURE_FINISH), read_count);
		}

		/**
		 * Build a write I2CCommand that can be later pushed to the I2CManager for
		 * proper handling.
		 * Calling this method has no effect on the bus until the returned command 
		 * is actually pushed to the I2CManager through a `launch_commands()` call.
		 * 
		 * @param write_count the number of bytes to get from the storage value
		 * in the Future associated with the I2C transaction, in order to write 
		 * them to the device; if `0`, the whole storage value should be used by 
		 * this command.
		 * @param finish specify the behavior to adopt after this command has been
		 * fully handled through the I2C bus; it allows to ensure a STOP condition
		 * will be sent on the bus (independently of the existence of further commands),
		 * or force the end of the Future associated to the chain of I2C commands.
		 * 
		 * @sa launch_commands()
		 * @sa read()
		 */
		I2CCommand write(uint8_t write_count = 0, I2CFinish finish = I2CFinish::NONE) const
		{
			return I2CCommand::write(device_, uint8_t(finish & I2CFinish::FORCE_STOP), 
				uint8_t(finish & I2CFinish::FUTURE_FINISH), write_count);
		}

		/**
		 * Launch execution (asynchronously for ATmega, synchronously for ATtiny)
		 * of a chain of I2CCommand items (constructed with `read()` and `write()`
		 * methods).
		 * 
		 * On ATmega, the method returns immediately and one has to use @p future
		 * status to know when all @p commands have been executed.
		 * 
		 * On ATtiny, this command is blocking and returns only ocne all @p commands
		 * have been executed on the I2C bus.
		 * 
		 * I2C commands execution is based on a Future that is used to:
		 * - provide data to write commands
		 * - store data returned by read commands
		 * 
		 * @param future the Future conatining all write data and ready to store 
		 * all read data; it is shared by all @p commands
		 * @param commands the list of I2CCommand to be executed, one after another
		 * 
		 * @retval 0 when the method did not encounter any error
		 * @return an error code if something bad happended; for ATmega, this 
		 * typically happens when the queue of I2CCommand is full, or when 
		 * @p future could not be registered with the FutureManager; for ATtiny,
		 * since all execution is synchronous, any error on the I2C bus or the 
		 * target device will trigger an error here.
		 * 
		 * @sa read()
		 * @sa write()
		 * @sa errors
		 */
		int launch_commands(
			lifecycle::LightProxy<future::AbstractFuture> proxy, std::initializer_list<I2CCommand> commands)
		{
			//FIXME add checks for resolvability of proxy by handler_ (ie if dynamic, manager must not be null)
			const uint8_t num_commands = commands.size();
			if (num_commands == 0) return errors::EINVAL;
			OUTER_SYNCHRONIZED
			{
				uint8_t max_read = 0;
				uint8_t max_write = 0;
				INNER_SYNCHRONIZED
				{
					// pre-conditions (must be synchronized)
					if (!handler_.ensure_num_commands_(num_commands)) return errors::EAGAIN;
					future::AbstractFuture& future = resolve(proxy);
					max_read = future.get_future_value_size_();
					max_write = future.get_storage_value_size_();
				}
				// Limit total number of bytes read or written in a transaction to 255
				uint8_t total_read = 0;
				uint8_t total_write = 0;
				for (const I2CCommand& command : commands)
				{
					// Count number of bytes read and written
					if (command.type.write)
						total_write += (command.byte_count ? command.byte_count : max_write);
					else
						total_read += (command.byte_count ? command.byte_count : max_read);
				}
				// check sum of read commands byte_count matches future output size
				// check sum of write commands byte_count matches future input size
				if ((total_write != max_write) || (total_read != max_read)) return errors::EINVAL;

				// Now push each command to the I2CManager
				int error = 0;
				uint8_t index = 0;
				for (I2CCommand command : commands)
				{
					// update command.byte_count if 0
					command.future_ = proxy;
					if (!command.byte_count)
						command.byte_count = (command.type.write ? max_write : max_read);
					// force future finish for last command in transaction
					if (++index == num_commands)
						command.type.finish_future = true;
					// Note: on ATtiny, this method blocks until I2C command is finished!
					if (!handler_.push_command_(command))
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

		//TODO API DOC
		template<typename T> T& resolve(const lifecycle::LightProxy<T> proxy) const
		{
			return handler_.resolve(proxy);
		}

		//TODO maybe add 2 launch_commands with resp. AbstractFuture& and LifeCycle<AbstractFuture>?

	private:
		uint8_t device_ = 0;
		MANAGER& handler_;
	};
}

#undef INNER_SYNCHRONIZED
#undef OUTER_SYNCHRONIZED

#endif /* I2C_DEVICE_HH */
/// @endcond
