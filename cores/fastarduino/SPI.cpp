//   Copyright 2016-2017 Jean-Francois Poilpret
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
