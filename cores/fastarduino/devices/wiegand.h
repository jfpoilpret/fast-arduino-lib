//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Common types used by 3D sensors (e.g. compass, gyroscope, accelerometer).
 */
#ifndef WIEGAND_H
#define WIEGAND_H

#include <stdint.h>
#include "../utilities.h"

namespace devices
{
	/**
	 * Defines utility classes to implement various protocols used in several devices, like Wiegand.
	 */
	namespace protocols
	{
	}
}

namespace devices::protocols
{
	//TODO DOCS
	//TODO review synchronized usage and optimize!
	//TODO improve performance, maybe by working on a byte-per-byte basis and with separate paroty data
	//TODO study Wiegand protocol extensions and improve this class as a template (on enum defining all Wiegand protocols)
	//TODO may need traits for generic use of extended protocols
	class Wiegand
	{
	public:
		using DATA_TYPE = uint32_t;
		static constexpr uint8_t DATA_BITS = 24;
		static constexpr DATA_TYPE DATA_MASK = 0x00FFFFFFUL;

		Wiegand() = default;

		void reset()
		{
			synchronized
			{
				buffer_ = 0UL;
				mask_ = INITIAL_MASK;
			}
		}

		bool available() const
		{
			synchronized return (mask_ == 0UL);
		}

		bool valid() const
		{
			uint32_t buffer = get_buffer();
			// 1. check parity on first 12 bits
			bool even_bit = (buffer & (1UL << PARITY1_BIT));
			if (even_bit != parity(24, buffer)) return false;

			// 2. check parity on last 12 bits
			bool odd_bit = (buffer & (1UL << PARITY2_BIT));
			if (odd_bit == parity(12, buffer)) return false;

			return true;
		}

		DATA_TYPE get_data() const
		{
			return (get_buffer() >> 1UL) & DATA_MASK;
		}

		void on_falling_data0()
		{
			// This is a 0
			mask_ >>= 1;
		}

		void on_falling_data1()
		{
			// This is a 1
			buffer_ |= mask_;
			mask_ >>= 1;
		}

	private:
		uint32_t get_buffer() const
		{
			synchronized return buffer_;
		}

		static bool parity(uint8_t start, uint32_t buffer)
		{
			uint8_t count = 0;
			uint8_t size = PARITY_BITS_COUNT;
			uint32_t mask = 1UL << start;
			while (size-- != 0)
			{
				if (buffer & mask) ++count;
				mask >>= 1;
			}
			return (count % 2);
		}

		// Positions in stream of parity bits
		static constexpr uint8_t PARITY1_BIT = 25;
		static constexpr uint8_t PARITY2_BIT = 0;
		static constexpr uint8_t PARITY_BITS_COUNT = 12;

		static constexpr uint32_t INITIAL_MASK = 1UL << 25;
		
		uint32_t buffer_ = 0UL;
		uint32_t mask_ = INITIAL_MASK;
	};
}

#endif /* WIEGAND_H */
/// @endcond
