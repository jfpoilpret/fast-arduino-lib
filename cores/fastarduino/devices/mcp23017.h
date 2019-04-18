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

#ifndef MCP23017_H
#define MCP23017_H

#include <math.h>
#include "../i2c_device.h"
#include "../utilities.h"

namespace devices
{
	//TODO select way to work with this device: 8 or 16 bits
	template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast> class MCP23017 : public i2c::I2CDevice<MODE_>
	{
	public:
		static constexpr const i2c::I2CMode MODE = MODE_;

		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		//TODO device_ to be calculated
		MCP23017(MANAGER& manager, uint8_t address)
			: i2c::I2CDevice<MODE>(manager), device_{address}
		{
			//TODO initialize bank (0 or 1?)
		}

		// API to define for configuration
		bool begin()
		{
		}

		inline bool end() INLINE
		{
		}

		inline Status status() INLINE
		{
			Status status;
			read_register(STATUS_REG, (uint8_t&) status);
			return status;
		}

	private:
		static constexpr const uint8_t DEVICE_ADDRESS = 0x1E << 1;

		static constexpr const uint8_t CONFIG_REG_A = 0;
		static constexpr const uint8_t CONFIG_REG_B = 1;
		static constexpr const uint8_t MODE_REG = 2;
		static constexpr const uint8_t OUTPUT_REG_1 = 3;
		static constexpr const uint8_t STATUS_REG = 9;
		static constexpr const uint8_t IDENT_REG_A = 10;
		static constexpr const uint8_t IDENT_REG_B = 11;
		static constexpr const uint8_t IDENT_REG_C = 12;

		bool write_register(uint8_t address, uint8_t value)
		{
			return (this->write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&& this->write(DEVICE_ADDRESS, value, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK);
		}

		bool read_register(uint8_t address, uint8_t& value)
		{
			return (this->write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&& this->read(DEVICE_ADDRESS, value, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK);
		}

		const uint8_t device_;
	};
}

#endif /* MCP23017_H */
