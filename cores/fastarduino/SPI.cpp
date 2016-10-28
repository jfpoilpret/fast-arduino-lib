#include "SPI.hh"

void SPI::SPIDevice::init()
{
	//TODO Handle also USI for ATtiny
	synchronized
	{
		// Set MOSI and SCK as Output
		// Set MISO as Input (high impedence)
		// Also set SS as Output (mandatory for Master SPI as per Atmel datasheet)
		Board::DDR_SPI_REG.set(
			(Board::DDR_SPI_REG.get() & ~_BV(Board::SPI_MISO)) |
			_BV(Board::SPI_MOSI) | _BV(Board::SPI_SCK) | _BV(Board::SPI_SS));
		// Set MISO as high impedence
		Board::PORT_SPI_REG.set(Board::PORT_SPI_REG.get() & ~_BV(Board::SPI_MISO));
	}
}

SPI::ClockRate SPI::SPIDevice::compute_clockrate(uint32_t frequency)
{
	//TODO
	return ClockRate::CLOCK_DIV_4;
}

SPI::SPIDevice::SPIDevice(	Board::DigitalPin cs, ChipSelect cs_mode, 
							ClockRate rate, Mode mode, DataOrder order)
	:	_spcr{_BV(SPE) | _BV(MSTR) | (uint8_t(rate) & 0x03) | uint8_t(order) | uint8_t(mode)},
		_spsr{(uint8_t(rate) & 0x10) ? SPI2X : 0},
		_cs{cs, PinMode::OUTPUT, cs_mode == ChipSelect::ACTIVE_LOW}
{
}
