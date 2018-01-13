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

#ifndef I2C_HH
#define I2C_HH

//TODO add support for asynchronous operation?
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
// Should we then provide two distinct I2CManager classes?
//TODO register ISR macro?

//NOTE only Master operation is supported for the moment
namespace i2c
{
	// Hook function type, useful for debugging new I2C devices implementation
	using I2C_STATUS_HOOK = void (*)(uint8_t expected_status, uint8_t actual_status);

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

	enum class I2CMode : uint8_t
	{
		Standard,
		Fast
	};
};

#endif /* I2C_HH */
