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

/**
 * @file 
 * I2C API common definitions.
 */
#ifndef I2C_HH
#define I2C_HH

#include <stdint.h>

//NOTE only Master operation is supported for the moment
/**
 * Define API to define and manage I2C devices.
 * I2C is available to all MCU supported by FastArduino, even in ATtiny MCU, for which
 * I2C is implemented with *Universal Serial Interface* (USI).
 * @note Current implementation supports both synchronous and asynchronous operation.
 * However, asynchronous operation is only supported on ATmega MCU.
 * 
 * The following snippet shows how to use an I2C device, the DS1307 Real Time Clock:
 * @code
 * int main()
 * {
 *     i2c::I2CSyncManager<i2c::I2CMode::STANDARD> manager;
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
	 * Transmission status codes.
	 * Transmission status is returned by all `i2c::I2CDevice` read and write methods.
	 * This status is also transmitted to an optional hook function for debug purposes.
	 * 
	 * All codes are defined and directly mapped from ATmega328 datasheet 
	 * (section 22. "2-wire Serial interface", tables 22-2 and 22-3).
	 * 
	 * You will probably never need to use these codes in your program.
	 * 
	 * @sa I2CSyncStatusManager
	 * @sa I2CAsyncStatusManager
	 * @sa i2c::status
	 */
	namespace Status
	{
		/** Code indicating the last called method executed as expected without any issue. */
		constexpr const uint8_t OK = 0x00;
		/** [Transmitter/Receiver modes] A START condition has been transmitted. */
		constexpr const uint8_t START_TRANSMITTED = 0x08;
		/** [Transmitter/Receiver modes] A repeated START condition has been transmitted. */
		constexpr const uint8_t REPEAT_START_TRANSMITTED = 0x10;
		/** [Transmitter mode] SLA+W has been transmitted; ACK has been received. */
		constexpr const uint8_t SLA_W_TRANSMITTED_ACK = 0x18;
		/** [Transmitter mode] SLA+W has been transmitted; NOT ACK has been received. */
		constexpr const uint8_t SLA_W_TRANSMITTED_NACK = 0x20;
		/** [Transmitter mode] Data byte has been transmitted; ACK has been received. */
		constexpr const uint8_t DATA_TRANSMITTED_ACK = 0x28;
		/** [Transmitter mode] Data byte has been transmitted; NOT ACK has been received. */
		constexpr const uint8_t DATA_TRANSMITTED_NACK = 0x30;
		/**
		 * [Transmitter mode] Abitration lost in SLA+W or data bytes. 
		 * [Receiver mode] Abitration lost in SLA+R or NOT ACK bit. 
		 */
		constexpr const uint8_t ARBITRATION_LOST = 0x38;
		/** [Receiver mode] SLA+R has been transmitted; ACK has been received. */
		constexpr const uint8_t SLA_R_TRANSMITTED_ACK = 0x40;
		/** [Receiver mode] SLA+R has been transmitted; NOT ACK has been received. */
		constexpr const uint8_t SLA_R_TRANSMITTED_NACK = 0x48;
		/** [Receiver mode] Data byte has been transmitted; ACK has been returned. */
		constexpr const uint8_t DATA_RECEIVED_ACK = 0x50;
		/** [Receiver mode] Data byte has been transmitted; NOT ACK has been returned. */
		constexpr const uint8_t DATA_RECEIVED_NACK = 0x58;

		/** [Any mode] Problem occurring with Future handling during I2C transmission. */
		constexpr const uint8_t FUTURE_ERROR = 0x80;
	}

	/**
	 * I2C available transmission modes. This defines the maximum bus transmission 
	 * frequency.
	 * 
	 * @sa I2CSyncManager
	 * @sa I2CAsyncManager
	 */
	enum class I2CMode : uint8_t
	{
		/** I2C Standard mode, less than 100KHz. */
		STANDARD,
		/** I2C Fast mode, less than 400KHz. */
		FAST
	};
};

#endif /* I2C_HH */
/// @endcond
