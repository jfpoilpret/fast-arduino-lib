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

#ifndef I2C_DEVICE_HH
#define I2C_DEVICE_HH

#include <fastarduino/array.h>
#include <fastarduino/initializer_list.h>
#include <fastarduino/errors.h>
#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/utilities.h>

#include "i2c_handler.h"

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

namespace i2c
{
	enum class I2CFinish : uint8_t
	{
		NONE = 0,
		FORCE_STOP = 0x01,
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

	template<I2CMode MODE>
	class AbstractDevice
	{
		public:
		AbstractDevice(I2CHandler<MODE>& handler, uint8_t device)
		: device_{device}, handler_{handler} {}

		AbstractDevice(const AbstractDevice<MODE>&) = delete;
		AbstractDevice<MODE>& operator=(const AbstractDevice<MODE>&) = delete;

		protected:
		void set_device(uint8_t device)
		{
			device_ = device;
		}

		I2CCommand read(I2CFinish finish = I2CFinish::NONE)
		{
			return I2CCommand::read(
				device_, uint8_t(finish & I2CFinish::FORCE_STOP), 0, uint8_t(finish & I2CFinish::FUTURE_FINISH));
		}
		I2CCommand write(I2CFinish finish = I2CFinish::NONE)
		{
			return I2CCommand::write(
				device_, uint8_t(finish & I2CFinish::FORCE_STOP), 0, uint8_t(finish & I2CFinish::FUTURE_FINISH));
		}
		int launch_commands(future::AbstractFuture& future, std::initializer_list<I2CCommand> commands)
		{
			const uint8_t num_commands = commands.size();
			if (num_commands == 0) return errors::EINVAL;
			auto& manager = future::AbstractFutureManager::instance();
			OUTER_SYNCHRONIZED
			{
				INNER_SYNCHRONIZED
				{
					// pre-conditions (must be synchronized)
					if (!handler_.ensure_num_commands_(num_commands)) return errors::EAGAIN;
					if (manager.available_futures_() == 0) return errors::EAGAIN;
					manager.register_future_(future);
				}
				int error = 0;
				for (I2CCommand command : commands)
				{
					command.future_id = future.id();
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

	private:
		uint8_t device_ = 0;
		I2CHandler<MODE>& handler_;
	};
}

#undef SYNCHRONIZED

#endif /* I2C_DEVICE_HH */
/// @endcond
