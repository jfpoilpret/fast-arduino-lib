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

#ifndef I2C_HH
#define I2C_HH

#include "boards/board_traits.h"

//TODO define traits for other MCU
//TODO add support for ATtiny (use USI)

//TODO ERRORS? constants in errors.h or dedicated enum class?

//TODO add support for asynchronous operation?
//TODO register ISR macro?
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
// Should we then provide two distinct I2CManager classes?

//NOTE only Master operation is supported for the moment
namespace i2c
{
	// NOTE we use prescaler = 1 everywhere
	class I2CManager
	{
		using TRAIT = board_traits::TWI_trait;
		
	public:
		static constexpr const uint32_t DEFAULT_FREQUENCY = 100000UL;
		
		I2CManager(): _status{} {}
		
		void begin(uint32_t frequency = DEFAULT_FREQUENCY) INLINE
		{
			// 1. set SDA/SCL pullups
			TRAIT::PORT |= TRAIT::PULLUP_MASK;
			// 2. set I2C frequency
			TWBR = calculate_TWBR(frequency);
			TWSR = 0;
			// 3. Enable TWI
			TWCR = _BV(TWEN);
		}
		void end() INLINE
		{
			// 1. Disable TWI
			TWCR = 0;
			// 2. remove SDA/SCL pullups
			TRAIT::PORT &= ~TRAIT::PULLUP_MASK;
		}
		uint8_t error() const
		{
			return _status;
		}
		
	private:
		// low-level methods to handle the bus, used by friend class I2CDevice
		bool start() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
			return wait_twint(0x08);
		}
		bool repeat_start() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
			return wait_twint(0x10);
		}
		bool send_slar(uint8_t address) INLINE
		{
			TWDR = address | 0x01;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(0x40);
		}
		bool send_slaw(uint8_t address) INLINE
		{
			TWDR = address;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(0x18);
		}
		bool send_data(uint8_t data) INLINE
		{
			TWDR = data;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(0x28);
		}
		bool receive_data(uint8_t& data, bool last_byte = false) INLINE
		{
			//FIXME it seems TWEA is to acknowledge next received byte (not the previous one!)
			// Then a problem occurs for the last byte we want to get, which should have NACK instead!
			// Send ACK for previous data (including SLA-R)
			bool ok;
			if (last_byte)
			{
				TWCR = _BV(TWEN) | _BV(TWINT);
				ok = wait_twint(0x58);
			}
			else
			{
				TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
				ok = wait_twint(0x50);
			}
			if (ok) data = TWDR;
			return ok;
		}
//		bool stop_receive() INLINE
//		{
//			// Just send NACK to notify slave to stop sending data
//			TWCR = _BV(TWEN) | _BV(TWINT);
//			return wait_twint(0x58);
//		}
		void stop() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
		}
		
	private:
		bool wait_twint(uint8_t expected_status)
		{
			loop_until_bit_is_set(TWCR, TWINT);
			_status = TWSR & 0xF8;
			if (_status == expected_status)
			{
				_status = 0;
				return true;
			}
			else
				return false;
		}

		static constexpr uint8_t calculate_TWBR(uint32_t frequency)
		{
			return (F_CPU / frequency - 16UL) / 2;
		}
		
		uint8_t _status;
		
		friend class I2CDevice;
	};
	
	enum class BusConditions: uint8_t
	{
		NO_START_NO_STOP = 0x00,
		START_NO_STOP = 0x01,
		REPEAT_START_NO_STOP = 0x03,
		START_STOP = 0x05,
		REPEAT_START_STOP = 0x07,
		NO_START_STOP = 0x04
	};
	
	class I2CDevice
	{
	protected:
		I2CDevice(I2CManager& manager):_manager{manager} {}
		
		int read(uint8_t address, uint8_t* data, uint8_t size, BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) && _manager.send_slar(address);
			while (ok && --size)
				ok = _manager.receive_data(*data++);
			if (uint8_t(conditions) & 0x04)
			{
				ok = ok && _manager.receive_data(*data++, true);
				_manager.stop();
			}
			return _manager.error();
		}
		template<typename T> int read(uint8_t address, T& data, BusConditions conditions = BusConditions::START_STOP)
		{
			return read(address, (uint8_t*) &data, sizeof(T), conditions);
		}
		
		int write(uint8_t address, const uint8_t* data, uint8_t size, BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) && _manager.send_slaw(address);
			while (ok && size--)
				ok = _manager.send_data(*data++);
			if (uint8_t(conditions) & 0x04)
				_manager.stop();
			return _manager.error();
		}
		template<typename T> int write(uint8_t address, const T& data, BusConditions conditions = BusConditions::START_STOP)
		{
			return write(address, (const uint8_t*) &data, sizeof(T), conditions);
		}
		// Specialization for uint8_t data
		int write(uint8_t address, uint8_t data, BusConditions conditions = BusConditions::START_STOP)
		{
			bool ok = true;
			if (uint8_t(conditions) & 0x01)
				ok = (uint8_t(conditions) & 0x02 ? _manager.repeat_start() : _manager.start()) && _manager.send_slaw(address);
			ok = ok && _manager.send_data(data);
			if (uint8_t(conditions) & 0x04)
				_manager.stop();
			return _manager.error();
		}
		
	private:
		I2CManager& _manager;
	};
};

#endif /* I2C_HH */

