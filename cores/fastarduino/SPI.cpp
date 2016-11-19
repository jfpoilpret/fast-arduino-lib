#include "SPI.hh"

#ifdef SPDR
// Handle SPI for ATmega
void SPI::init()
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
#else
// Handle USI for ATtiny
void SPI::init()
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
#endif
