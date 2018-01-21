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

#ifndef FASTSPI_HH
#define FASTSPI_HH

#include "boards/board.h"
#include "gpio.h"

//TODO Make a SPISlave class with a data handler/buffer
namespace spi
{
	void init();
	
	enum class ClockRate : uint8_t
	{
		CLOCK_DIV_4 = 0x00,
		CLOCK_DIV_16 = 0x01,
		CLOCK_DIV_64 = 0x02,
		CLOCK_DIV_128 = 0x03,
		CLOCK_DIV_2 = 0x10,
		CLOCK_DIV_8 = 0x11,
		CLOCK_DIV_32 = 0x12
	};
	
	constexpr ClockRate compute_clockrate(uint32_t frequency)
	{
		return	frequency >= (F_CPU / 2) ? ClockRate::CLOCK_DIV_2 :
				frequency >= (F_CPU / 4) ? ClockRate::CLOCK_DIV_4 :
				frequency >= (F_CPU / 8) ? ClockRate::CLOCK_DIV_8 :
				frequency >= (F_CPU / 16) ? ClockRate::CLOCK_DIV_16 :
				frequency >= (F_CPU / 32) ? ClockRate::CLOCK_DIV_32 :
				frequency >= (F_CPU / 64) ? ClockRate::CLOCK_DIV_64 :
				ClockRate::CLOCK_DIV_128;
	}

#ifdef SPDR
	enum class DataOrder : uint8_t
	{
		MSB_FIRST = 0,
		LSB_FIRST = _BV(DORD)
	};
#else
	enum class DataOrder : uint8_t
	{
		MSB_FIRST = 0
	};
#endif

#ifdef SPDR
	enum class Mode : uint8_t
	{
		MODE_0 = 0,
		MODE_1 = _BV(CPHA),
		MODE_2 = _BV(CPOL),
		MODE_3 = _BV(CPHA) | _BV(CPOL)
	};
#else
	enum class Mode : uint8_t
	{
		MODE_0 = _BV(USIWM0) | _BV(USICLK) | _BV(USICS1),
		MODE_1 = _BV(USIWM0) | _BV(USICLK) | _BV(USICS1) | _BV(USICS0)
	};
#endif
	
	enum class ChipSelect : uint8_t
	{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};

	class AbstractSPIDevice
	{
	protected:
#ifdef SPDR
		inline uint8_t transfer(uint8_t data)
		{
			SPDR = data;
			loop_until_bit_is_set(SPSR, SPIF);
			return SPDR;
		}
#else
		inline uint8_t transfer(uint8_t data)
		{
			//TODO check produced assembly code
			USIDR = data;
			// Clear counter overflow before transmission
			USISR = _BV(USIOIF);
			synchronized
			{
				do
				{
					USICR |= _BV(USITC);
				}
				while (bit_is_clear(USISR, USIOIF));
			}
			return USIDR;
		}
#endif
		
		inline void transfer(uint8_t* data, uint16_t size)
		{
			while (size--)
			{
				uint8_t value = *data;
				*data++ = transfer(value);
			}
		}
		inline void transfer(uint8_t* data, uint16_t size, uint8_t sent)
		{
			while (size--) *data++ = transfer(sent);
		}
	};

	template<board::DigitalPin CS,
			ChipSelect CS_MODE = ChipSelect::ACTIVE_LOW, 
			ClockRate RATE = ClockRate::CLOCK_DIV_4, 
			Mode MODE = Mode::MODE_0,
			DataOrder ORDER = DataOrder::MSB_FIRST>
	class SPIDevice : public AbstractSPIDevice
	{
	protected:
		SPIDevice() INLINE : cs_{gpio::PinMode::OUTPUT, CS_MODE == ChipSelect::ACTIVE_LOW}
		{
		}

#ifdef SPDR
		inline void start_transfer()
		{
			cs_.toggle();
			SPCR = SPCR_;
			SPSR = SPSR_;
		}
#else
		inline void start_transfer()
		{
			cs_.toggle();
			// Set 3-wire mode (SPI) and requested SPI mode (0 or 1) and use software clock strobe (through USITC)
			USICR = USICR_;
		}
#endif
		inline void end_transfer() INLINE
		{
			cs_.toggle();
		}

	private:
#ifdef SPDR
		// Configuration values to reset at beginning of each transfer
		static const constexpr uint8_t SPCR_ = 
			_BV(SPE) | _BV(MSTR) | (uint8_t(RATE) & 0x03) | uint8_t(ORDER) | uint8_t(MODE);
		static const constexpr uint8_t SPSR_ = (uint8_t(RATE) & 0x10) ? _BV(SPI2X) : 0;
#else
		static const constexpr uint8_t USICR_ = uint8_t(MODE);
#endif
		typename gpio::FastPinType<CS>::TYPE cs_;
	};
};

#endif /* FASTSPI_HH */
