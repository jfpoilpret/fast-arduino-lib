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

#ifndef I2C_HANDLER_HH
#define I2C_HANDLER_HH

#include <util/delay_basic.h>
#include "utilities.h"
#include "boards/board_traits.h"
#include "i2c.h"

//TODO Refactor further to only put the interface in class, and put functions impl afterwards (will that support INLINE?)
namespace i2c
{
	template<I2CMode MODE> class I2CManager;
	
#if defined(TWCR)
	// NOTE we use prescaler = 1 everywhere
	template<I2CMode MODE>
	class I2CHandler
	{
	private:
		static constexpr const uint32_t STANDARD_FREQUENCY = 100000UL;
		static constexpr const uint32_t FAST_FREQUENCY = 400000UL;

		static constexpr uint8_t calculate_TWBR(uint32_t frequency)
		{
			return (F_CPU / frequency - 16UL) / 2;
		}

		static constexpr const uint8_t TWBR_VALUE = 
			calculate_TWBR(MODE == I2CMode::Standard ? STANDARD_FREQUENCY : FAST_FREQUENCY);

		using TRAIT = board_traits::TWI_trait;

		I2CHandler(I2C_STATUS_HOOK hook): _status{}, _hook{hook} {}

		void begin() INLINE
		{
			// 1. set SDA/SCL pullups
			TRAIT::PORT |= TRAIT::SCL_SDA_MASK;
			// 2. set I2C frequency
			TWBR = TWBR_VALUE;
			TWSR = 0;
			// 3. Enable TWI
			TWCR = _BV(TWEN);
		}
		void end() INLINE
		{
			// 1. Disable TWI
			TWCR = 0;
			// 2. remove SDA/SCL pullups
			TRAIT::PORT &= ~TRAIT::SCL_SDA_MASK;
		}

	public:
		uint8_t status() const
		{
			return _status;
		}

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

		uint8_t _status;
		const I2C_STATUS_HOOK _hook;

		friend class I2CManager<MODE>;
	};
#else
	template<I2CMode MODE = I2CMode::Standard>
	class I2CHandler
	{
	private:
		using TRAIT = board_traits::TWI_trait;

		I2CHandler(I2C_STATUS_HOOK hook): _status{}, _hook{hook}
		{
			// set SDA/SCL default directions
			TRAIT::DDR &= ~_BV(TRAIT::BIT_SDA);
			TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
			TRAIT::DDR |= _BV(TRAIT::BIT_SCL);
			TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
		}

		inline void SCL_HIGH() INLINE
		{
			TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
			TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
		}
		inline void SCL_LOW() INLINE
		{
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
		}
		inline void SDA_HIGH() INLINE
		{
			TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
		}
		inline void SDA_LOW() INLINE
		{
			TRAIT::PORT &= ~_BV(TRAIT::BIT_SDA);
		}

		inline void SDA_INPUT() INLINE
		{
			TRAIT::DDR &= ~_BV(TRAIT::BIT_SDA);
			TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
		}
		inline void SDA_OUTPUT() INLINE
		{
			TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
		}

		void begin() INLINE
		{
			// 1. Force 1 to data
			USIDR = 0xFF;
			// 2. Enable TWI
			// Set USI I2C mode, enable software clock strobe (USITC)
			USICR = _BV(USIWM1) | _BV(USICS1) | _BV(USICLK);
			// Clear all interrupt flags
			USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
			// 3. Set SDA as output
			SDA_OUTPUT();
		}
		void end() INLINE
		{
			// Disable TWI
			USICR = 0;
			//TODO should we set SDA back to INPUT?
		}

	public:
		uint8_t status() const
		{
			return _status;
		}

		// low-level methods to handle the bus, used by friend class I2CDevice
		bool start() INLINE
		{
			return _start(Status::START_TRANSMITTED);
		}
		bool repeat_start() INLINE
		{
			return _start(Status::REPEAT_START_TRANSMITTED);
		}

		bool send_slar(uint8_t address) INLINE
		{
			return send_byte(address | 0x01, Status::SLA_R_TRANSMITTED_ACK, Status::SLA_R_TRANSMITTED_NACK);
		}
		bool send_slaw(uint8_t address) INLINE
		{
			return send_byte(address, Status::SLA_W_TRANSMITTED_ACK, Status::SLA_W_TRANSMITTED_NACK);
		}

		bool send_data(uint8_t data) INLINE
		{
			return send_byte(data, Status::DATA_TRANSMITTED_ACK, Status::DATA_TRANSMITTED_NACK);
		}
		bool receive_data(uint8_t& data, bool last_byte = false)
		{
			SDA_INPUT();
			data = transfer(USISR_DATA);
			// Send ACK (or NACK if last byte)
			USIDR = (last_byte ? 0xFF : 0x00);
			uint8_t good_status = (last_byte ? Status::DATA_RECEIVED_NACK : Status::DATA_RECEIVED_ACK);
			transfer(USISR_ACK);
			return callback_hook(true, good_status, good_status);
		}

		void stop() INLINE
		{
			// Pull SDA low
			SDA_LOW();
			// Release SCL
			SCL_HIGH();
			_delay_loop_1(T_SU_STO);
			// Release SDA
			SDA_HIGH();
			_delay_loop_1(T_BUF);
		}

	private:
		bool _start(uint8_t good_status)
		{
			// Ensure SCL is HIGH
			SCL_HIGH();
			// Wait for Tsu-sta
			_delay_loop_1(T_SU_STA);
			// Now we can generate start condition
			// Force SDA low for Thd-sta
			SDA_LOW();
			_delay_loop_1(T_HD_STA);
			// Pull SCL low
			SCL_LOW();
//			_delay_loop_1(T_LOW());
			// Release SDA (force high)
			SDA_HIGH();
			//TODO check START transmission with USISIF flag?
//			return callback_hook(USISR & _BV(USISIF), good_status, Status::ARBITRATION_LOST);
			return callback_hook(true, good_status, Status::ARBITRATION_LOST);
		}

		bool send_byte(uint8_t data, uint8_t ACK, uint8_t NACK)
		{
			// Set SCL low TODO is this line really needed for every byte transferred?
			SCL_LOW();
			// Transfer address byte
			USIDR = data;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			SDA_INPUT();
			return callback_hook((transfer(USISR_ACK) & 0x01) == 0, ACK, NACK);
		}

		uint8_t transfer(uint8_t USISR_count)
		{
			// Init counter (8 bits or 1 bit for acknowledge)
			USISR = USISR_count;
			do
			{
				_delay_loop_1(T_LOW);
				// clock strobe (SCL raising edge)
				USICR |= _BV(USITC);
				TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
				_delay_loop_1(T_HIGH);
				// clock strobe (SCL falling edge)
				USICR |= _BV(USITC);
			}
			while (bit_is_clear(USISR, USIOIF));
			_delay_loop_1(T_LOW);
			// Read data
			uint8_t data = USIDR;
			USIDR = 0xFF;
			// Release SDA
			SDA_OUTPUT();
			return data;
		}

		bool callback_hook(bool ok, uint8_t good_status, uint8_t bad_status)
		{
			if (_hook) _hook(good_status, (ok ? good_status : bad_status));
			_status = (ok ? Status::OK : bad_status);
			return ok;
		}

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		static constexpr uint8_t calculate_count(float standard, float fast)
		{
			return utils::calculate_delay1_count(MODE == I2CMode::Standard ? standard : fast);
		}

		// Timing constants for current mode (as per I2C specifications)
		static constexpr const uint8_t T_HD_STA = calculate_count(4.0, 0.6);
		static constexpr const uint8_t T_LOW = calculate_count(4.7, 1.3);
		static constexpr const uint8_t T_HIGH = calculate_count(4.0, 0.6);
		static constexpr const uint8_t T_SU_STA = calculate_count(4.7, 0.6);
		static constexpr const uint8_t T_SU_STO = calculate_count(4.0, 0.6);
		static constexpr const uint8_t T_BUF = calculate_count(4.7, 1.3);

		uint8_t _status;
		const I2C_STATUS_HOOK _hook;
		friend class I2CManager<MODE>;
	};
#endif
}

#endif /* I2C_HANDLER_HH */

