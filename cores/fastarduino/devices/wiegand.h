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
	//TODO study Wiegand protocol extensions and improve this class as a template (on enum defining all Wiegand protocols)
	//TODO may need traits for generic use of extended protocols
	class Wiegand
	{
	public:
		using DATA_TYPE = uint32_t;
		static constexpr uint8_t DATA_BITS = 24;

		Wiegand()
		{
			reset_();
		}

		void reset()
		{
			synchronized reset_();
		}

		void reset_()
		{
			buffer_ = 0UL;
			current_ = &data_[DATA_BYTES - 1];
			mask_ = INITIAL_MASK;
		}

		bool available_() const
		{
			return count_ == FRAME_BITS;
		}

		bool valid_() const
		{
			uint32_t data = data24_;

			// 1. check parity on first 12 bits
			if (parity1_ != parity(PARITY1_HIGH_BIT_INDEX, data)) return false;

			// 2. check parity on last 12 bits
			if (parity2_ == parity(PARITY2_HIGH_BIT_INDEX, data)) return false;

			return true;
		}

		DATA_TYPE get_data_() const
		{
			return data24_;
		}

		void on_falling_data0()
		{
			// This is a 0
			// Check if frame is finished (do not add any bit)
			if (available_()) return;
			// check if parity1
			if (count_ == PARITY1_BIT_FRAME_INDEX)
				parity1_ = false;
			else if (count_ == PARITY2_BIT_FRAME_INDEX)
				parity2_ = false;
			else
			{
				// Normal data bit, update mask and byte to write
				mask_ >>= 1;
				if (mask_ == 0)
				{
					mask_ = INITIAL_MASK;
					--current_;
				}
			}
			// Update bits count
			++count_;
		}

		void on_falling_data1()
		{
			// This is a 1
			// Check if frame is finished (do not add any bit)
			if (available_()) return;
			// check if parity1
			if (count_ == PARITY1_BIT_FRAME_INDEX)
				parity1_ = true;
			// check if parity2
			else if (count_ == PARITY2_BIT_FRAME_INDEX)
				parity2_ = true;
			// Normal data bit (1 only), store it
			else
			{
				*current_ |= mask_;
				// Update mask and byte to write
				mask_ >>= 1;
				if (mask_ == 0)
				{
					mask_ = INITIAL_MASK;
					--current_;
				}
			}
			// Update bits count
			++count_;
		}

	private:
		// Positions in stream of parity bits
		static constexpr uint8_t FRAME_BITS = 26;
		static constexpr uint8_t PARITY1_BIT_FRAME_INDEX = 0;
		static constexpr uint8_t PARITY2_BIT_FRAME_INDEX = 25;

		static constexpr uint8_t PARITY_BITS_COUNT = 12;
		static constexpr uint8_t PARITY1_HIGH_BIT_INDEX = 23;
		static constexpr uint8_t PARITY2_HIGH_BIT_INDEX = 11;

		static constexpr uint8_t INITIAL_MASK = 0x80;
		static constexpr uint8_t DATA_BYTES = DATA_BITS / 8;

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

		union
		{
			// Used to clear all in one instruction
			uint32_t buffer_;
			struct
			{
				// Used to store bits, byte after byte
				uint8_t data_[DATA_BYTES];
				// Count of received bits (including parity bits)
				uint8_t count_;
			};
			struct
			{
				// Used to access the actual 24-bits data
				uint32_t data24_ : DATA_BITS;
				uint32_t :8;
			};
		};
		// Points to current read byte in data_[]
		uint8_t* current_;
		// Mask for current read bit
		uint8_t mask_;
		// First parity bit read (even)
		bool parity1_;
		// Last parity bit read (odd)
		bool parity2_;
	};
}

#endif /* WIEGAND_H */
/// @endcond
