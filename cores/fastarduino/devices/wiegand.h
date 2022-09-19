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
	/**
	 * Supporting class for the [Wiegand 26 bits protocol](https://en.wikipedia.org/wiki/Wiegand_interface)
	 * (used by many access control devices).
	 * 
	 * To use Wiegand class, you just need to instantiate it in your own device class,
	 * ensure you call `on_falling_data0()` and `on_falling_data1()` when you detect 
	 * (ISR preeferred) that line DATA0 or DATA1 is falling to 0V.
	 * 
	 * Other API methods allow you to know the state of reading (finished, parity valid),
	 * get the read data (if available and valid) and reset the reading.
	 * 
	 * @note only the "original" Wiegand interface standard (26-bits, including 2
	 * parity bits) is supported here; there are many variations in terms of bits 
	 * number and parity checks, developped by access control companies, but it is
	 * not clear if some have been standardized. If some needs occur for extended
	 * support in the future, it is probable this class will be refactored to
	 * support these extensions.
	 */
	class Wiegand
	{
	public:
		/** 
		 * The data type used to return data read from the access control device.
		 * This may be larger than what is actually needed.
		 */
		using DATA_TYPE = uint32_t;

		/** 
		 * The actual number of bits of data in `DATA_TYPE`. This does not include
		 * parity bits which do not hold any access control data.
		 * The actual bits are always the LSB in `DATA_TYPE`, other bits (HSB)
		 * are set to `0`.
		 */
		static constexpr uint8_t DATA_BITS = 24;

		/**
		 * Construct a new Wiegand instance to be sued by an access control device.
		 * The instance is ready to be used for data reading.
		 */
		Wiegand()
		{
			reset_();
		}

		/**
		 * Reset current read state of this instance. Any bits read will be lost.
		 * You should call this method after having read latest data with `get_data_()`.
		 * 
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `reset_()` instead.
		 * 
		 * @sa reset_()
		 * @sa get_data_()
		 */
		void reset()
		{
			synchronized reset_();
		}

		/**
		 * Reset current read state of this instance. Any bits read will be lost.
		 * You should call this method after having read latest data with `get_data_()`.
		 * 
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `reset_()` instead.
		 * 
		 * @sa reset_()
		 * @sa get_data_()
		 */
		void reset_()
		{
			buffer_ = 0UL;
			current_ = &data_[DATA_BYTES - 1];
			mask_ = INITIAL_MASK;
		}

		/**
		 * Check if data is available, i\.e\. all 26 bits have been received already.
		 * 
		 * @note this method does not care about validity of received bits (parity check),
		 * for this you will need to also call `valid_()`.
		 * 
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context.
		 * 
		 * @sa valid_()
		 */
		bool available_() const
		{
			return count_ == FRAME_BITS;
		}

		/**
		 * Check if current data is valid, i\.e\. parity has been checked against 
		 * both even and odd parity bits (as per Wiegand 26 bits standard).
		 * 
		 * @note this method does not care about availability of data (all 26 bits
		 * received or not yet), for this you will need to first call `available_()`.
		 * 
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context.
		 * 
		 * @sa available_()
		 */
		bool valid_() const
		{
			uint32_t data = data24_;

			// 1. check parity on first 12 bits
			if (parity1_ != parity(PARITY1_HIGH_BIT_INDEX, data)) return false;

			// 2. check parity on last 12 bits
			if (parity2_ == parity(PARITY2_HIGH_BIT_INDEX, data)) return false;

			return true;
		}

		/**
		 * Get data read from access control device.
		 * 
		 * @note this method does not care about availability or validity of data,
		 * it will just return the current state of data received so far. This is why
		 * you should first call `available_()` and `valid_()` before calling `get_data_()`.
		 * 
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context.
		 * 
		 * @sa available_()
		 * @sa valid_()
		 */
		DATA_TYPE get_data_() const
		{
			return data24_;
		}

		/**
		 * Your device shall call this method whenever DATA0 line level is falling 
		 * to `0`, which means a `0` bit must be added to read data.
		 * 
		 * It is expected to work with interrupts (EXT or PCI) for connecting
		 * the access control device DATA0 and DATA1 pins.
		 * 
		 * @sa on_falling_data1()
		 */
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

		/**
		 * Your device shall call this method whenever DATA1 line level is falling 
		 * to `0`, which means a `1` bit must be added to read data.
		 * 
		 * It is expected to work with interrupts (EXT or PCI) for connecting
		 * the access control device DATA0 and DATA1 pins.
		 * 
		 * @sa on_falling_data0()
		 */
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
