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

#ifndef SIPO_HH
#define SIPO_HH

#include "../gpio.h"

namespace devices
{
	template<board::DigitalPin CLOCK_, board::DigitalPin LATCH_, board::DigitalPin DATA_> class SIPO
	{
	public:
		static constexpr const board::DigitalPin CLOCK = CLOCK_;
		static constexpr const board::DigitalPin LATCH = LATCH_;
		static constexpr const board::DigitalPin DATA = DATA_;

		static constexpr const board::Port PORT = gpio::FastPinType<CLOCK>::PORT;
		static constexpr const uint8_t DDR_MASK =
			gpio::FastPinType<CLOCK>::MASK | gpio::FastPinType<LATCH>::MASK | gpio::FastPinType<DATA>::MASK;
		static constexpr const uint8_t PORT_MASK = gpio::FastPinType<LATCH>::MASK;

		SIPO() : clock_{}, latch_{}, data_{}
		{
			static_assert(PORT == gpio::FastPinType<LATCH>::PORT && PORT == gpio::FastPinType<DATA>::PORT,
						  "CLOCK, LATCH and DATA pins must belong to the same PORT");
		}

		inline void init()
		{
			clock_.set_mode(gpio::PinMode::OUTPUT, false);
			latch_.set_mode(gpio::PinMode::OUTPUT, true);
			data_.set_mode(gpio::PinMode::OUTPUT, false);
		}

		template<typename T> void output(T data)
		{
			uint8_t* pdata = (uint8_t*) data;
			latch_.clear();
			for (uint8_t i = 0; i < sizeof(T); ++i) bit_bang_data(pdata[i]);
			latch_.set();
		}

		// Specialized output for most common types
		inline void output(uint8_t data) INLINE
		{
			latch_.clear();
			bit_bang_data(data);
			latch_.set();
		}

		inline void output(uint16_t data) INLINE
		{
			latch_.clear();
			bit_bang_data(data >> 8);
			bit_bang_data(data);
			latch_.set();
		}

	private:
		void bit_bang_data(uint8_t data)
		{
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

		typename gpio::FastPinType<CLOCK>::TYPE clock_;
		typename gpio::FastPinType<LATCH>::TYPE latch_;
		typename gpio::FastPinType<DATA>::TYPE data_;
	};
}

#endif /* SIPO_HH */
