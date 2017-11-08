//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef I2C_DEVICE_HH
#define I2C_DEVICE_HH

#include "i2c.h"
#include "i2c_manager.h"

namespace i2c
{
	enum class BusConditions : uint8_t
	{
		NO_START_NO_STOP = 0x00,
		START_NO_STOP = 0x01,
		REPEAT_START_NO_STOP = 0x03,
		START_STOP = 0x05,
		REPEAT_START_STOP = 0x07,
		NO_START_STOP = 0x04
	};

	template<I2CMode MODE = I2CMode::Standard> class I2CDevice
	{
	public:
		using MANAGER = I2CManager<MODE>;
		using HANDLER = typename MANAGER::HANDLER;

	protected:
		I2CDevice(MANAGER& manager) : _manager{manager.handler()}
		{
		}

		int read(uint8_t address, uint8_t* data, uint8_t size, BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) &&
					 _manager.send_slar(address);
			while (ok && --size) ok = _manager.receive_data(*data++);
			if (uint8_t(conditions) & 0x04)
			{
				ok = ok && _manager.receive_data(*data++, true);
				_manager.stop();
			}
			return _manager.status();
		}
		template<typename T> int read(uint8_t address, T& data, BusConditions conditions = BusConditions::START_STOP)
		{
			return read(address, (uint8_t*) &data, sizeof(T), conditions);
		}

		int write(uint8_t address, const uint8_t* data, uint8_t size,
				  BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) &&
					 _manager.send_slaw(address);
			while (ok && size--) ok = _manager.send_data(*data++);
			if (uint8_t(conditions) & 0x04) _manager.stop();
			return _manager.status();
		}
		template<typename T>
		int write(uint8_t address, const T& data, BusConditions conditions = BusConditions::START_STOP)
		{
			return write(address, (const uint8_t*) &data, sizeof(T), conditions);
		}
		// Specialization for uint8_t data
		int write(uint8_t address, uint8_t data, BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) &&
					 _manager.send_slaw(address);
			ok = ok && _manager.send_data(data);
			if (uint8_t(conditions) & 0x04) _manager.stop();
			return _manager.status();
		}

	private:
		HANDLER& _manager;
	};
};

#endif /* I2C_DEVICE_HH */
