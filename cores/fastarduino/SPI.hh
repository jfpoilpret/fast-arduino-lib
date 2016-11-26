#ifndef FASTSPI_HH
#define FASTSPI_HH

#include "Board.hh"
#include "FastIO.hh"

//TODO Make a SPISlave class with a data handler/buffer
namespace SPI
{
	void init();
	
	enum class ClockRate: uint8_t
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
	enum class DataOrder: uint8_t
	{
		MSB_FIRST = 0,
		LSB_FIRST = _BV(DORD)
	};
#else
	enum class DataOrder: uint8_t
	{
		MSB_FIRST = 0
	};
#endif

#ifdef SPDR
	enum class Mode: uint8_t
	{
		MODE_0 = 0,
		MODE_1 = _BV(CPHA),
		MODE_2 = _BV(CPOL),
		MODE_3 = _BV(CPHA) | _BV(CPOL)
	};
#else
	enum class Mode: uint8_t
	{
		MODE_0 = _BV(USIWM0) | _BV(USICLK) | _BV(USICS1),
		MODE_1 = _BV(USIWM0) | _BV(USICLK) | _BV(USICS1) | _BV(USICS0)
	};
#endif
	
	enum class ChipSelect: uint8_t
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
			while(size--)
			{
				register uint8_t value = *data;
				*data++ = transfer(value);
			}
		}
		inline void transfer(uint8_t* data, uint16_t size, uint8_t sent)
		{
			while(size--)
				*data++ = transfer(sent);
		}
	};

	template<Board::DigitalPin CS,
			ChipSelect CS_MODE = ChipSelect::ACTIVE_LOW, 
			ClockRate RATE = ClockRate::CLOCK_DIV_4, 
			Mode MODE = Mode::MODE_0,
			DataOrder ORDER = DataOrder::MSB_FIRST>
	class SPIDevice: public AbstractSPIDevice
	{
	protected:
		SPIDevice() INLINE:_cs{PinMode::OUTPUT, CS_MODE == ChipSelect::ACTIVE_LOW} {}

#ifdef SPDR
		inline void start_transfer()
		{
			_cs.toggle();
			SPCR = _spcr;
			SPSR = _spsr;
		}
#else
		inline void start_transfer()
		{
			_cs.toggle();
			// Set 3-wire mode (SPI) and requested SPI mode (0 or 1) and use software clock strobe (through USITC)
			USICR = _usicr;
		}
#endif
		inline void end_transfer() INLINE
		{
			_cs.toggle();
		}

	private:
		// Configuration values to reset at beginning of each transfer
#ifdef SPDR
		static const constexpr uint8_t _spcr = 
			_BV(SPE) | _BV(MSTR) | (uint8_t(RATE) & 0x03) | uint8_t(ORDER) | uint8_t(MODE);
		static const constexpr uint8_t _spsr = (uint8_t(RATE) & 0x10) ? _BV(SPI2X) : 0;
#else
		static const constexpr uint8_t _usicr = uint8_t(MODE);
#endif
		typename FastPinType<CS>::TYPE _cs;
	};

};

#endif /* FASTSPI_HH */
