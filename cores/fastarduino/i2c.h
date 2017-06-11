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
//TODO should methods return the error or just a bool and then an error() method would return the exact issue

//TODO add support for asynchronous operation?
//TODO register ISR macro?
//NOTE only Master operation is supported for the moment
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
// Should we then provide two distinct I2CManager classes?

namespace i2c
{
	//TODO all static or singleton or whatever?
	// NOTE we use prescaler = 1 everywhere
	class I2CManager
	{
		using TRAIT = board_traits::TWI_trait;
		
	public:
		static constexpr const uint32_t DEFAULT_FREQUENCY = 100000UL;
		
		I2CManager(): _status{}, _error{} {}
		
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
		int error() const
		{
			return _error ? _status : 0;
		}
		
		//TODO low-level methods to handle the bus, make private?
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
		bool receive_data(uint8_t& data) INLINE
		{
			// Send ACK for previous data (including SLA-R)
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
			bool ok = wait_twint(0x50);
			if (ok) data = TWDR;
			return ok;
		}
		bool stop_receive() INLINE
		{
			// Just send NACK to notify slave to stop sending data
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(0x58);
		}
		void stop() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
		}
		
	private:
		bool wait_twint(uint8_t expected_status)
		{
			loop_until_bit_is_set(TWCR, TWINT);
			_status = TWSR & 0xF8;
			_error = (_status != expected_status);
			return !_error;
		}

		static constexpr uint8_t calculate_TWBR(uint32_t frequency)
		{
			return (F_CPU / frequency - 16UL) / 2;
		}
		
		int _status;
		//TODO remove _error as _status should be enough (forced to zero if no error)
		bool _error;
	};
	
	class I2CDevice
	{
	protected:
		//TODO public only for tests without subclasses
	public:
		I2CDevice(I2CManager& manager):_manager{manager}, _stopped{true} {}
		
		//TODO improve API to allow better split with several reads/writes without start/sla/stop
		
		//TODO shouldn't these methods be all protected (ie used by actual devices with "business" methods)?
		int read(uint8_t address, uint8_t* data, uint8_t size, bool dont_stop = false)
		{
			bool ok =		(_stopped ? _manager.start() : _manager.repeat_start())
						&&	_manager.send_slar(address);
			while (ok && size--)
				ok = _manager.receive_data(*data++);
			ok = ok && _manager.stop_receive();
			if (!dont_stop)
				_manager.stop();
			_stopped = !dont_stop;
			return _manager.error();
		}
		int write(uint8_t address, const uint8_t* data, uint8_t size, bool dont_stop = false)
		{
			bool ok =		(_stopped ? _manager.start() : _manager.repeat_start())
						&&	_manager.send_slaw(address);
			while (ok && size--)
				ok = _manager.send_data(*data++);
			if (!dont_stop)
				_manager.stop();
			_stopped = !dont_stop;
			return _manager.error();
		}
		
		template<typename T> int write(uint8_t address, const T& data, bool dont_stop = false)
		{
			return write(address, (const uint8_t*) &data, sizeof(T), dont_stop);
		}
		template<typename T> int read(uint8_t address, T& data, bool dont_stop = false)
		{
			return read(address, (uint8_t*) &data, sizeof(T), dont_stop);
		}
		
	private:
		I2CManager& _manager;
		bool _stopped;
	};
};

#endif /* I2C_HH */

