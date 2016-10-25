#ifndef SPI_HH
#define SPI_HH

#include "Board.hh"
#include "IO.hh"

namespace SPI
{
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
	enum class DataOrder: uint8_t
	{
		MSB_FIRST = 0,
		LSB_FIRST = _BV(DORD)
	};
	// Replace with one of 4 SPI modes?
	enum class Mode: uint8_t
	{
		MODE_0 = 0,
		MODE_1 = _BV(CPHA),
		MODE_2 = _BV(CPOL),
		MODE_3 = _BV(CPHA) | _BV(CPOL)
	};
	
	enum class ChipSelect: uint8_t
	{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};

	class SPIDevice
	{
	public:
		static void init();
		static ClockRate compute_clockrate(uint32_t frequency);
		
	protected:
		SPIDevice(	Board::DigitalPin cs, 
					ChipSelect cs_mode = ChipSelect::ACTIVE_LOW, 
					ClockRate rate = ClockRate::CLOCK_DIV_4, 
					Mode mode = Mode::MODE_0,
					DataOrder order = DataOrder::MSB_FIRST);

		inline void start_transfer()
		{
#ifdef SPDR
			SPCR = _spcr;
			SPSR = _spsr;
#else
			//TODO
#endif
			_cs.toggle();
		}
		inline uint8_t transfer(uint8_t data)
		{
#ifdef SPDR
			SPDR = data;
			loop_until_bit_is_set(SPSR, SPIF);
			return SPDR;
#else
			//TODO
#endif
		}
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
		inline void end_transfer()
		{
			_cs.toggle();
		}

	private:
		// Configuration values to reset at beginning of each transfer
#ifdef SPDR
		const uint8_t _spcr;
		const uint8_t _spsr;
#else
			//TODO
#endif
		IOPin _cs;
	};

};

#endif /* SPI_HH */
