//   Copyright 2016-2018 Jean-Francois Poilpret
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

// NOTE: ATtiny implementation provides no pullup, hence you must ensure your 
// I2C bus has pullups where needed
namespace i2c
{
	template<I2CMode MODE_> class I2CManager;

	template<I2CMode MODE_> class I2CHandler
	{
	private:
		using TRAIT = board_traits::TWI_trait;

		inline I2CHandler(I2C_STATUS_HOOK hook) INLINE;

		inline void begin() INLINE;
		inline void end() INLINE;

	public:
		static constexpr const I2CMode MODE = MODE_;
		inline uint8_t status() const INLINE
		{
			return status_;
		}

		// Low-level methods to handle the bus, used by friend class I2CDevice
		inline bool start() INLINE;
		inline bool repeat_start() INLINE;
		inline bool send_slar(uint8_t address) INLINE;
		inline bool send_slaw(uint8_t address) INLINE;
		inline bool send_data(uint8_t data) INLINE;
		inline bool receive_data(uint8_t& data, bool last_byte = false) INLINE;
		inline void stop() INLINE;

	private:
// Implementation part is specific to AVR architecture: ATmega (TWCR) or ATtiny (USICR)
#if defined(TWCR)
		bool wait_twint(uint8_t expected_status);

		static constexpr const uint32_t STANDARD_FREQUENCY = (F_CPU / 100000UL - 16UL) / 2;
		static constexpr const uint32_t FAST_FREQUENCY = (F_CPU / 400000UL - 16UL) / 2;
		static constexpr const uint8_t TWBR_VALUE = (MODE == I2CMode::Standard ? STANDARD_FREQUENCY : FAST_FREQUENCY);
#else
		inline void SCL_HIGH() INLINE;
		inline void SCL_LOW() INLINE;
		inline void SDA_HIGH() INLINE;
		inline void SDA_LOW() INLINE;
		inline void SDA_INPUT() INLINE;
		inline void SDA_OUTPUT() INLINE;

		bool send_start(uint8_t good_status);
		bool send_byte(uint8_t data, uint8_t ACK, uint8_t NACK);
		uint8_t transfer(uint8_t USISR_count);
		bool callback_hook(bool ok, uint8_t good_status, uint8_t bad_status);

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		// Timing constants for current mode (as per I2C specifications)
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.0 : 0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.7 : 1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.0 : 0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.7 : 0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.0 : 0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(MODE == I2CMode::Standard ? 4.7 : 1.3);
#endif

		uint8_t status_;
		const I2C_STATUS_HOOK hook_;

		friend class I2CManager<MODE>;
	};

// IMPLEMENTATION
//================

#if defined(TWCR)

	// ATmega implementation
	//----------------------
	template<I2CMode MODE> I2CHandler<MODE>::I2CHandler(I2C_STATUS_HOOK hook) : status_{}, hook_{hook}
	{
	}

	template<I2CMode MODE> void I2CHandler<MODE>::begin()
	{
		// 1. set SDA/SCL pullups
		TRAIT::PORT |= TRAIT::SCL_SDA_MASK;
		// 2. set I2C frequency
		TWBR = TWBR_VALUE;
		TWSR = 0;
		// 3. Enable TWI
		TWCR = _BV(TWEN);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::end()
	{
		// 1. Disable TWI
		TWCR = 0;
		// 2. remove SDA/SCL pullups
		TRAIT::PORT &= ~TRAIT::SCL_SDA_MASK;
	}

	template<I2CMode MODE> bool I2CHandler<MODE>::start()
	{
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
		return wait_twint(Status::START_TRANSMITTED);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::repeat_start()
	{
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
		return wait_twint(Status::REPEAT_START_TRANSMITTED);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_slar(uint8_t address)
	{
		TWDR = address | 0x01;
		TWCR = _BV(TWEN) | _BV(TWINT);
		return wait_twint(Status::SLA_R_TRANSMITTED_ACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_slaw(uint8_t address)
	{
		TWDR = address;
		TWCR = _BV(TWEN) | _BV(TWINT);
		return wait_twint(Status::SLA_W_TRANSMITTED_ACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_data(uint8_t data)
	{
		TWDR = data;
		TWCR = _BV(TWEN) | _BV(TWINT);
		return wait_twint(Status::DATA_TRANSMITTED_ACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::receive_data(uint8_t& data, bool last_byte)
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
	template<I2CMode MODE> void I2CHandler<MODE>::stop()
	{
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
	}

	template<I2CMode MODE> bool I2CHandler<MODE>::wait_twint(uint8_t expected_status)
	{
		loop_until_bit_is_set(TWCR, TWINT);
		status_ = TWSR & 0xF8;
		if (hook_) hook_(expected_status, status_);
		if (status_ == expected_status)
		{
			status_ = 0;
			return true;
		}
		else
			return false;
	}

#else

	// ATtiny implementation
	//----------------------
	template<I2CMode MODE> I2CHandler<MODE>::I2CHandler(I2C_STATUS_HOOK hook) : status_{}, hook_{hook}
	{
		// set SDA/SCL default directions
		TRAIT::DDR &= ~_BV(TRAIT::BIT_SDA);
		// TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
		TRAIT::DDR |= _BV(TRAIT::BIT_SCL);
		TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
	}

	template<I2CMode MODE> void I2CHandler<MODE>::SCL_HIGH()
	{
		TRAIT::PORT |= _BV(TRAIT::BIT_SCL);
		TRAIT::PIN.loop_until_bit_set(TRAIT::BIT_SCL);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::SCL_LOW()
	{
		TRAIT::PORT &= ~_BV(TRAIT::BIT_SCL);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::SDA_HIGH()
	{
		TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::SDA_LOW()
	{
		TRAIT::PORT &= ~_BV(TRAIT::BIT_SDA);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::SDA_INPUT()
	{
		TRAIT::DDR &= ~_BV(TRAIT::BIT_SDA);
		// TRAIT::PORT |= _BV(TRAIT::BIT_SDA);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::SDA_OUTPUT()
	{
		TRAIT::DDR |= _BV(TRAIT::BIT_SDA);
	}

	template<I2CMode MODE> void I2CHandler<MODE>::begin()
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
	template<I2CMode MODE> void I2CHandler<MODE>::end()
	{
		// Disable TWI
		USICR = 0;
		//TODO should we set SDA back to INPUT?
	}

	template<I2CMode MODE> bool I2CHandler<MODE>::start()
	{
		return send_start(Status::START_TRANSMITTED);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::repeat_start()
	{
		return send_start(Status::REPEAT_START_TRANSMITTED);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_slar(uint8_t address)
	{
		return send_byte(address | 0x01, Status::SLA_R_TRANSMITTED_ACK, Status::SLA_R_TRANSMITTED_NACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_slaw(uint8_t address)
	{
		return send_byte(address, Status::SLA_W_TRANSMITTED_ACK, Status::SLA_W_TRANSMITTED_NACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::send_data(uint8_t data)
	{
		return send_byte(data, Status::DATA_TRANSMITTED_ACK, Status::DATA_TRANSMITTED_NACK);
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::receive_data(uint8_t& data, bool last_byte)
	{
		SDA_INPUT();
		data = transfer(USISR_DATA);
		// Send ACK (or NACK if last byte)
		USIDR = (last_byte ? 0xFF : 0x00);
		uint8_t good_status = (last_byte ? Status::DATA_RECEIVED_NACK : Status::DATA_RECEIVED_ACK);
		transfer(USISR_ACK);
		return callback_hook(true, good_status, good_status);
	}
	template<I2CMode MODE> void I2CHandler<MODE>::stop()
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

	template<I2CMode MODE> bool I2CHandler<MODE>::send_start(uint8_t good_status)
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
	template<I2CMode MODE> bool I2CHandler<MODE>::send_byte(uint8_t data, uint8_t ACK, uint8_t NACK)
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
	template<I2CMode MODE> uint8_t I2CHandler<MODE>::transfer(uint8_t USISR_count)
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
		} while (bit_is_clear(USISR, USIOIF));
		_delay_loop_1(T_LOW);
		// Read data
		uint8_t data = USIDR;
		USIDR = 0xFF;
		// Release SDA
		SDA_OUTPUT();
		return data;
	}
	template<I2CMode MODE> bool I2CHandler<MODE>::callback_hook(bool ok, uint8_t good_status, uint8_t bad_status)
	{
		if (hook_) hook_(good_status, (ok ? good_status : bad_status));
		status_ = (ok ? Status::OK : bad_status);
		return ok;
	}

#endif
}

#endif /* I2C_HANDLER_HH */
