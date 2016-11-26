#include "SPI.hh"
#include "Board_traits.hh"

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
		Board::SPI_trait::DDR.set(
			(Board::SPI_trait::DDR.get() & ~_BV(Board::SPI_trait::MISO)) |
			_BV(Board::SPI_trait::MOSI) | _BV(Board::SPI_trait::SCK) | _BV(Board::SPI_trait::SS));
		// Set MISO as pullup and force MOSI and SCK low
		Board::SPI_trait::PORT.set(
			(Board::SPI_trait::PORT.get() | _BV(Board::SPI_trait::MISO)) &
			~(_BV(Board::SPI_trait::MOSI) | _BV(Board::SPI_trait::SCK)));
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
		Board::SPI_trait::DDR.set(
			(Board::SPI_trait::DDR.get() & ~_BV(Board::SPI_trait::MISO)) |
			_BV(Board::SPI_trait::MOSI) | _BV(Board::SPI_trait::SCK));
		// Set MISO as pullup and force MOSI and SCK low
		Board::SPI_trait::PORT.set(
			(Board::SPI_trait::PORT.get() | _BV(Board::SPI_trait::MISO)) &
			~(_BV(Board::SPI_trait::MOSI) | _BV(Board::SPI_trait::SCK)));
	}
}
#endif
