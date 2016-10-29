#include "SPI.hh"

#ifdef SPDR
// Handle SPI for ATmega
void SPI::SPIDevice::init()
{
	//TODO check code size and possibly revert to macro defined DDR_SPI and PORT_SPI if better
	synchronized
	{
		// Set MOSI and SCK as Output
		// Set MISO as Input (high impedance)
		// Also set SS as Output (mandatory for Master SPI as per Atmel datasheet)
		Board::DDR_SPI_REG.set(
			(Board::DDR_SPI_REG.get() & ~_BV(Board::SPI_MISO)) |
			_BV(Board::SPI_MOSI) | _BV(Board::SPI_SCK) | _BV(Board::SPI_SS));
		// Set MISO as pullup and force MOSI and SCK low
		Board::PORT_SPI_REG.set(
			(Board::PORT_SPI_REG.get() | _BV(Board::SPI_MISO)) &
			~(_BV(Board::SPI_MOSI) | _BV(Board::SPI_SCK)));
	}
}

SPI::ClockRate SPI::SPIDevice::compute_clockrate(uint32_t frequency)
{
	if (frequency >= (F_CPU / 2)) return ClockRate::CLOCK_DIV_2;
	if (frequency >= (F_CPU / 4)) return ClockRate::CLOCK_DIV_4;
	if (frequency >= (F_CPU / 8)) return ClockRate::CLOCK_DIV_8;
	if (frequency >= (F_CPU / 16)) return ClockRate::CLOCK_DIV_16;
	if (frequency >= (F_CPU / 32)) return ClockRate::CLOCK_DIV_32;
	if (frequency >= (F_CPU / 64)) return ClockRate::CLOCK_DIV_64;
	return ClockRate::CLOCK_DIV_128;
}

SPI::SPIDevice::SPIDevice(	Board::DigitalPin cs, ChipSelect cs_mode, 
							ClockRate rate, Mode mode, DataOrder order)
	:	_spcr{_BV(SPE) | _BV(MSTR) | (uint8_t(rate) & 0x03) | uint8_t(order) | uint8_t(mode)},
		_spsr{(uint8_t(rate) & 0x10) ? _BV(SPI2X) : 0},
		_cs{cs, PinMode::OUTPUT, cs_mode == ChipSelect::ACTIVE_LOW}
{
}

#else
// Handle USI for ATtiny
void SPI::SPIDevice::init()
{
	synchronized
	{
		// Set MOSI and SCK as Output
		// Set MISO as Input (high impedance)
		Board::DDR_SPI_REG.set(
			(Board::DDR_SPI_REG.get() & ~_BV(Board::SPI_MISO)) |
			_BV(Board::SPI_MOSI) | _BV(Board::SPI_SCK));
		// Set MISO as pullup and force MOSI and SCK low
		Board::PORT_SPI_REG.set(
			(Board::PORT_SPI_REG.get() | _BV(Board::SPI_MISO)) &
			~(_BV(Board::SPI_MOSI) | _BV(Board::SPI_SCK)));
	}
}

SPI::ClockRate SPI::SPIDevice::compute_clockrate(uint32_t frequency)
{
	return ClockRate::CLOCK_DIV_2;
}

SPI::SPIDevice::SPIDevice(	Board::DigitalPin cs, ChipSelect cs_mode, 
							UNUSED ClockRate rate, Mode mode, UNUSED DataOrder order)
	:	_usicr{uint8_t(mode)},
		_cs{cs, PinMode::OUTPUT, cs_mode == ChipSelect::ACTIVE_LOW}
{
}

#endif
