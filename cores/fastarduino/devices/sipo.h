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
 * API to handle "SIPO" (*serial in parallel out*) chips also known as "shift
 * registers", like the **74HC595**.
 * These chips provide an easy and cheap way to increase the number of digital
 * outputs when the MCU is limited.
 */
#ifndef SIPO_HH
#define SIPO_HH

#include "../gpio.h"

/**
 * Defines all API for all external devices supported by FastArduino.
 * Most devices support API, but not all, define their own namespace within 
 * `devices` namespace.
 */
namespace devices
{
	/**
	 * This template class supports one SIPO chip, connected to the MCU through 
	 * 3 pins.
	 * These 3 pins are described below (different chips may have different naming
	 * for each pin but you should expect the 3 pins in all SIPO chips):
	 * - CLOCK pin: this is used to clock data input to DATA pin: one cycle per bit.
	 * - DATA pin: this is used to pass serial in data to the chip, one bit per
	 * CLOCK cycle.
	 * - LATCH pin: this pin is used to let the chip know that all data bits have
	 * been transferred and should now be made available to the actual output pins
	 * of the chip.
	 * 
	 * Some SIPO chips (e.g. the 74HC595) allow daisy-chaining, allowing
	 * you to use several chips but only 3 pins on the MCU.
	 * Daisy-chained SIPO are transparently supported by this class.
	 * 
	 * @tparam CLOCK_ the output pin that will send the clock signal to the chip
	 * @tparam LATCH_ the output pin that will tell the chip to copy its shift 
	 * register content to its output pins
	 * @tparam DATA_ the output pin that will send serial data to the chip
	 */
	template<board::DigitalPin CLOCK_, board::DigitalPin LATCH_, board::DigitalPin DATA_> class SIPO
	{
	public:
		/** The output pin that will send the clock signal to the chip. */
		static constexpr const board::DigitalPin CLOCK = CLOCK_;
		/** The output pin that will tell the chip to copy its shift register content to its output pins. */
		static constexpr const board::DigitalPin LATCH = LATCH_;
		/** The output pin that will send serial data to the chip. */
		static constexpr const board::DigitalPin DATA = DATA_;

		/// @cond notdocumented
		SIPO(const SIPO&) = delete;
		SIPO& operator=(const SIPO&) = delete;
		/// @endcond
		
		/**
		 * Create a new SIPO handler, according to pins defined by class template
		 * parameters.
		 * This does not initialize the 3 output pins, for this you shall use
		 * `init()`.
		 * @sa init()
		 */
		SIPO() = default;

		/**
		 * Initialize (direction, value) all used pins. This must be executed
		 * before any other API.
		 * This is not executed directly from constructor, in order to allow
		 * more optimized initialization with other pins, at port level, in the
		 * final program.
		 */
		void init()
		{
			clock_.set_mode(gpio::PinMode::OUTPUT, false);
			latch_.set_mode(gpio::PinMode::OUTPUT, true);
			data_.set_mode(gpio::PinMode::OUTPUT, false);
		}

		/**
		 * Handles output and latching of all bits in @p data to the SIPO chip.
		 * @tparam T the type of @p data; this allows pushing more than 8 bits of 
		 * data, e.g. if you have connected several chips in daisy-chain.
		 * @param data the data to serialize to SIPO chip (or chips if daisy 
		 * chained).
		 */
		template<typename T> void output(T data)
		{
			uint8_t* pdata = (uint8_t*) data;
			latch_.clear();
			for (uint8_t i = 0; i < sizeof(T); ++i) bit_bang_data(pdata[i]);
			latch_.set();
		}

		/// @cond notdocumented
		// Specialized output for most common types
		void output(uint8_t data) INLINE
		{
			latch_.clear();
			bit_bang_data(data);
			latch_.set();
		}

		void output(uint16_t data) INLINE
		{
			latch_.clear();
			bit_bang_data(bits::HIGH_BYTE(data));
			bit_bang_data(bits::LOW_BYTE(data));
			latch_.set();
		}
		/// @endcond

	private:
		void bit_bang_data(uint8_t data)
		{
			// Start with MSB
			uint8_t mask = 0x80;
			do
			{
				if (data & mask)
					data_.set();
				else
					data_.clear();
				clock_.set();
				mask >>= 1;
				clock_.clear();
			} while (mask);
		}

		gpio::FAST_PIN<CLOCK> clock_;
		gpio::FAST_PIN<LATCH> latch_;
		gpio::FAST_PIN<DATA> data_;
	};
}

#endif /* SIPO_HH */
/// @endcond
