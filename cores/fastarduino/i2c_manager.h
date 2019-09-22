//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * I2C Manager API.
 */
#ifndef I2C_MANAGER_HH
#define I2C_MANAGER_HH

#include "i2c.h"
#include "i2c_handler.h"

namespace i2c
{
	/**
	 * General I2C Manager.
	 * It is used by all I2C devices for transmission.
	 * For the time being, the MCU must always act as the only master on the bus.
	 * Using MCU as a slave will be supported in a later version of FastArduino.
	 * 
	 * @tparam MODE_ the I2C mode for this manager
	 * @sa i2c::I2CMode
	 */
	template<I2CMode MODE_ = I2CMode::STANDARD> class I2CManager
	{
	public:
		I2CManager(const I2CManager<MODE_>&) = delete;
		I2CManager<MODE_>& operator=(const I2CManager<MODE_>&) = delete;
		
		/** The I2C mode for this manager. */
		static constexpr const I2CMode MODE = MODE_;

		/**
		 * Create an I2C Manager with an optional hook function for debugging.
		 * @param hook an optional hook function that will be called back after
		 * each transmission operation.
		 */
		explicit I2CManager(I2C_STATUS_HOOK hook = nullptr) : handler_{hook} {}

		/**
		 * Prepare and enable the MCU for I2C transmission.
		 * Preparation includes setup of I2C pins (SDA and SCL).
		 */
		void begin() INLINE
		{
			handler_.begin();
		}

		/**
		 * Disable MCU I2C transmission.
		 */
		void end() INLINE
		{
			handler_.end();
		}

		/**
		 * Return latest transmission status.
		 * Possible statuses are defined in namespace `i2c::Status`.
		 * If latest operation was OK, then `i2c::Status::OK` (`0`) is returned.
		 * Any non zero value indicates an error.
		 * @sa i2c::Status
		 */
		uint8_t status() const
		{
			return handler_.status();
		}

	private:
		using HANDLER = I2CHandler<MODE>;

		HANDLER& handler() INLINE
		{
			return handler_;
		}

		HANDLER handler_;
		template<I2CMode M> friend class I2CDevice;
	};
};

#endif /* I2C_MANAGER_HH */
/// @endcond
