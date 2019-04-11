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
 * I2C API common definitions.
 */
#ifndef I2C_HH
#define I2C_HH

//TODO add support for asynchronous operation?
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
// Should we then provide two distinct I2CManager classes?
//TODO register ISR macro?

//NOTE only Master operation is supported for the moment
/**
 * Define API to define and manage I2C devices.
 * I2C is available to all MCU supported by FastArduino, even in ATtiny MCU, for which
 * I2C is implemented with *Universal Serial Interface* (USI).
 * @note Current implementation supports only synchronous operation. Future FastArduino 
 * versions may also support asynchronous (non-blocking) operation.
 * 
 * The following snippet shows how to use an I2C device, the DS1307 Real Time Clock:
 * @code
 * int main()
 * {
 *     i2c::I2CManager<i2c::I2CMode::Standard> manager;
 *     manager.begin();
 *     devices::rtc::DS1307 rtc{manager};
 *     devices::rtc::tm now;
 *     rtc.get_datetime(now);
 *     ...
 *     manager.end();
 * }
 * @endcode
 */
namespace i2c
{
	/**
	 * Hook function type, useful for debugging new I2C devices implementation.
	 * @sa i2c::I2CManager
	 */
	using I2C_STATUS_HOOK = void (*)(uint8_t expected_status, uint8_t actual_status);

	/**
	 * Transmission status codes.
	 * Transmission status is returned by all `i2c::I2CDevice` read and write methods.
	 * This status is also transmitted to an optional hook function for debug purposes.
	 * @sa i2c::I2CManager::status()
	 */
	namespace Status
	{
		constexpr const uint8_t OK = 0x00;
		constexpr const uint8_t START_TRANSMITTED = 0x08;
		constexpr const uint8_t REPEAT_START_TRANSMITTED = 0x10;
		constexpr const uint8_t SLA_W_TRANSMITTED_ACK = 0x18;
		constexpr const uint8_t SLA_W_TRANSMITTED_NACK = 0x20;
		constexpr const uint8_t DATA_TRANSMITTED_ACK = 0x28;
		constexpr const uint8_t DATA_TRANSMITTED_NACK = 0x30;
		constexpr const uint8_t ARBITRATION_LOST = 0x38;

		constexpr const uint8_t SLA_R_TRANSMITTED_ACK = 0x40;
		constexpr const uint8_t SLA_R_TRANSMITTED_NACK = 0x48;
		constexpr const uint8_t DATA_RECEIVED_ACK = 0x50;
		constexpr const uint8_t DATA_RECEIVED_NACK = 0x58;
	}

	/**
	 * I2C available transmission modes. This defines the maximum bus transmission 
	 * frequency.
	 * @sa i2c::I2CManager
	 * @sa i2c::I2CManager
	 */
	enum class I2CMode : uint8_t
	{
		/** I2C Standard mode, less than 100KHz. */
		Standard,
		/** I2C Fast mode, less than 400KHz. */
		Fast
	};
};

#endif /* I2C_HH */
/// @endcond
