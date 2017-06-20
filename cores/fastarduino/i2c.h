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

#include <util/delay_basic.h>
#include "boards/board_traits.h"

//TODO Use enum class for frequency selection: 100KHz or 400KHz)
//TODO Add debug facility (needed for implementing new devices)
// - special I2CManager?
// - integrate in current I2CManager but careful about size and speed impact!
//TODO ERRORS? constants in errors.h or in i2c namespace or dedicated enum class?


//TODO add support for ATtiny (use USI)
//TODO add support for asynchronous operation?
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
// Should we then provide two distinct I2CManager classes?
//TODO register ISR macro?

//NOTE only Master operation is supported for the moment
namespace i2c
{
	using I2C_STATUS_HOOK = void (*)(uint8_t expected_status, uint8_t actual_status);
	
	namespace Status
	{
		constexpr const uint8_t OK							= 0x00;
		constexpr const uint8_t START_TRANSMITTED			= 0x08;
		constexpr const uint8_t REPEAT_START_TRANSMITTED	= 0x10;
		constexpr const uint8_t SLA_W_TRANSMITTED_ACK		= 0x18;
		constexpr const uint8_t SLA_W_TRANSMITTED_NACK		= 0x20;
		constexpr const uint8_t DATA_TRANSMITTED_ACK		= 0x28;
		constexpr const uint8_t DATA_TRANSMITTED_NACK		= 0x30;
		constexpr const uint8_t ARBITRATION_LOST			= 0x38;
		
		constexpr const uint8_t SLA_R_TRANSMITTED_ACK		= 0x40;
		constexpr const uint8_t SLA_R_TRANSMITTED_NACK		= 0x48;
		constexpr const uint8_t DATA_RECEIVED_ACK			= 0x50;
		constexpr const uint8_t DATA_RECEIVED_NACK			= 0x58;
		
	}
	
	enum class I2CMode: uint8_t
	{
		Standard,
		Fast
	};

#if defined(TWCR)
	// NOTE we use prescaler = 1 everywhere
	class I2CManager
	{
		using TRAIT = board_traits::TWI_trait;
		
	public:
		static constexpr const uint32_t DEFAULT_FREQUENCY = 100000UL;
		
		I2CManager(I2C_STATUS_HOOK hook = 0): _status{}, _hook{hook} {}
		
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
		uint8_t status() const
		{
			return _status;
		}
		
	private:
		// low-level methods to handle the bus, used by friend class I2CDevice
		bool start() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
			return wait_twint(Status::START_TRANSMITTED);
		}
		bool repeat_start() INLINE
		{
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
			return wait_twint(Status::REPEAT_START_TRANSMITTED);
		}
		bool send_slar(uint8_t address) INLINE
		{
			TWDR = address | 0x01;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(Status::SLA_R_TRANSMITTED_ACK);
		}
		bool send_slaw(uint8_t address) INLINE
		{
			TWDR = address;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(Status::SLA_W_TRANSMITTED_ACK);
		}
		bool send_data(uint8_t data) INLINE
		{
			TWDR = data;
			TWCR = _BV(TWEN) | _BV(TWINT);
			return wait_twint(Status::DATA_TRANSMITTED_ACK);
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
				ok = wait_twint(Status::DATA_RECEIVED_NACK);
			}
			else
			{
				TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
				ok = wait_twint(Status::DATA_RECEIVED_ACK);
			}
			if (ok) data = TWDR;
			return ok;
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
			if (_hook) _hook(expected_status, _status);
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
		const I2C_STATUS_HOOK _hook;
		
		friend class I2CDevice;
	};
#else
	//TODO Externalize that utility method (to another header?) and rename so it does not produce compile errors!
	static constexpr uint8_t calculate_delay1_count(float time_us)
	{
		return uint8_t(F_CPU / 1000000UL / 3.0 * time_us + 1);
	}

	//TODO make private methods as public from a private class -> cleaner
	// NOTE we use prescaler = 1 everywhere
	class I2CManager
	{
		using TRAIT = board_traits::TWI_trait;
		
	public:
		//TODO hook is never called currently
		I2CManager(I2C_STATUS_HOOK hook = 0): _status{}, _hook{hook} {}
		
		void begin(I2CMode mode = I2CMode::Standard) INLINE
		{
			// 0. Initialize timing constants
			_mode = mode;
			// 1. set SDA/SCL as outputs and HIGH (master)
			TRAIT::DDR |= TRAIT::PULLUP_MASK;
			TRAIT::PORT |= TRAIT::PULLUP_MASK;
			// 2. Force 1 to data
			USIDR = 0xFF;
			// 3. Enable TWI
			// Set USI I2C mode, enable software clock strobe (USITC)
			USICR = _BV(USIWM1) | _BV(USICS1) | _BV(USICLK);
			// Clear all interrupt flags
			USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
		}
		void end() INLINE
		{
			// 1. Disable TWI
			USICR = 0;
			// 2. remove SDA/SCL pullups
			//FIXME WHAT SHALL WE DO HERE REALLY?
			TRAIT::DDR &= ~TRAIT::PULLUP_MASK;
			TRAIT::PORT &= ~TRAIT::PULLUP_MASK;
		}
		uint8_t status() const
		{
			return _status;
		}
		
	private:
		// low-level methods to handle the bus, used by friend class I2CDevice
		bool start() INLINE
		{
			// Ensure SCL is HIGH
			TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
			TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
			// Wait for Tsu-sta
			_delay_loop_1(T_SU_STA());
			// Now we can generate start condition
			// First ensure SDA and SCL are set as outputs
			TRAIT::DDR |= TRAIT::PULLUP_MASK;
			// Force SDA low for Thd-sta
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SDA);
			_delay_loop_1(T_HD_STA());
			// Pull SCL low
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
			_delay_loop_1(T_LOW());
			// Release SDA (force high)
			TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
			//TODO how to check START transmission? USISIF flag
			_status = Status::OK;
			return true;
		}
		bool repeat_start() INLINE
		{
			return start();
		}
		//TODO refactor if possible (need size optimization)
		bool send_slar(uint8_t address) INLINE
		{
			// Set SCL low
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
			// Transfer address byte
			USIDR = address | 0x01;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			TRAIT::DDR &= ~ _BV(TRAIT::BIT_SDA);
			if (transfer(USISR_ACK) & 0x01)
			{
				// NACK received
				TRAIT::PORT |= TRAIT::PULLUP_MASK;
				// store state
				_status = Status::SLA_R_TRANSMITTED_NACK;
				return false;
			}
			// Set SDA as output again
			TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
			_status = Status::OK;
			return true;
		}
		bool send_slaw(uint8_t address) INLINE
		{
			// Set SCL low
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
			// Transfer address byte
			USIDR = address;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			TRAIT::DDR &= ~ _BV(TRAIT::BIT_SDA);
			if (transfer(USISR_ACK) & 0x01)
			{
				// NACK received
				TRAIT::PORT |= TRAIT::PULLUP_MASK;
				// store state
				_status = Status::SLA_W_TRANSMITTED_NACK;
				return false;
			}
			// Set SDA as output again
			TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
			_status = Status::OK;
			return true;
		}
		bool send_data(uint8_t data) INLINE
		{
			// Set SCL low
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
			// Transfer address byte
			USIDR = data;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			TRAIT::DDR &= ~ _BV(TRAIT::BIT_SDA);
			if (transfer(USISR_ACK) & 0x01)
			{
				// NACK received
				TRAIT::PORT |= TRAIT::PULLUP_MASK;
				// store state
				_status = Status::DATA_TRANSMITTED_NACK;
				return false;
			}
			// Set SDA as output again
			TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
			_status = Status::OK;
			return true;
		}
		bool receive_data(uint8_t& data, bool last_byte = false) INLINE
		{
			// First set SDA as input
			TRAIT::DDR &= ~ _BV(TRAIT::BIT_SDA);
			data = transfer(USISR_DATA);
			// Set SDA as output again
			TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
			// Send ACK (or NACK if last byte)
			USIDR = (last_byte ? 0xFF : 0x00);
			transfer(USISR_ACK);
			_status = Status::OK;
			return true;
		}
		void stop() INLINE
		{
			// Pull SDA low
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SDA);
			// Release SCL
			TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
			TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
			_delay_loop_1(T_SU_STO());
			// Release SDA
			TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
			_delay_loop_1(T_BUF());
		}
		
	private:
		uint8_t transfer(uint8_t USISR_count)
		{
			// Init counter (8 bits or 1 bit for acknowledge)
			USISR = USISR_count;
			do
			{
				_delay_loop_1(T_LOW());
				// clock strobe (SCL raising edge)
				USICR |= _BV(USITC);
				TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
				_delay_loop_1(T_HIGH());
				// clock strobe (SCL falling edge)
				USICR |= _BV(USITC);
			}
			while (bit_is_clear(USISR, USIOIF));
			_delay_loop_1(T_LOW());
			return USIDR;
		}
		
//		bool wait_twint(uint8_t expected_status)
//		{
//			
//			loop_until_bit_is_set(TWCR, TWINT);
//			_status = TWSR & 0xF8;
//			if (_hook) _hook(expected_status, _status);
//			if (_status == expected_status)
//			{
//				_status = 0;
//				return true;
//			}
//			else
//				return false;
//		}

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);
		
		// Timing constants for Standard mode (as per I2C specifications)
		static constexpr const uint8_t STD_T_HD_STA = calculate_delay1_count(4.0);
		static constexpr const uint8_t STD_T_LOW = calculate_delay1_count(4.7);
		static constexpr const uint8_t STD_T_HIGH = calculate_delay1_count(4.0);
		static constexpr const uint8_t STD_T_SU_STA = calculate_delay1_count(4.7);
		static constexpr const uint8_t STD_T_SU_STO = calculate_delay1_count(4.0);
		static constexpr const uint8_t STD_T_BUF = calculate_delay1_count(4.7);
		
		// Timing constants for Fast mode (as per I2C specifications)
		static constexpr const uint8_t FAST_T_HD_STA = calculate_delay1_count(0.6);
		static constexpr const uint8_t FAST_T_LOW = calculate_delay1_count(1.3);
		static constexpr const uint8_t FAST_T_HIGH = calculate_delay1_count(0.6);
		static constexpr const uint8_t FAST_T_SU_STA = calculate_delay1_count(0.6);
		static constexpr const uint8_t FAST_T_SU_STO = calculate_delay1_count(0.6);
		static constexpr const uint8_t FAST_T_BUF = calculate_delay1_count(1.3);
		
		uint8_t T_HD_STA() const
		{
			return (_mode == I2CMode::Standard ? STD_T_HD_STA : FAST_T_HD_STA);
		}
		uint8_t T_LOW() const
		{
			return (_mode == I2CMode::Standard ? STD_T_LOW : FAST_T_LOW);
		}
		uint8_t T_HIGH() const
		{
			return (_mode == I2CMode::Standard ? STD_T_HIGH : FAST_T_HIGH);
		}
		uint8_t T_SU_STA() const
		{
			return (_mode == I2CMode::Standard ? STD_T_SU_STA : FAST_T_SU_STA);
		}
		uint8_t T_SU_STO() const
		{
			return (_mode == I2CMode::Standard ? STD_T_SU_STO : FAST_T_SU_STO);
		}
		uint8_t T_BUF() const
		{
			return (_mode == I2CMode::Standard ? STD_T_BUF : FAST_T_BUF);
		}
		
		uint8_t _status;
		const I2C_STATUS_HOOK _hook;
		I2CMode _mode;
		
		friend class I2CDevice;
	};
#endif
	
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
			return _manager.status();
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
			return _manager.status();
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
			return _manager.status();
		}
		
	private:
		I2CManager& _manager;
	};
};

#endif /* I2C_HH */

