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

#include "boards/board_traits.h"
#include "spi.h"

#ifdef SPDR
// Handle SPI for ATmega
void spi::init()
{
	synchronized
	{
		// Set MOSI and SCK as Output
		// Set MISO as Input (high impedance)
		// Also set SS as Output (mandatory for Master SPI as per Atmel datasheet)
		board_traits::SPI_trait::DDR = (board_traits::SPI_trait::DDR & ~_BV(board_traits::SPI_trait::MISO)) |
									   _BV(board_traits::SPI_trait::MOSI) | _BV(board_traits::SPI_trait::SCK) |
									   _BV(board_traits::SPI_trait::SS);
		// Set MISO as pullup and force MOSI and SCK low
		board_traits::SPI_trait::PORT = (board_traits::SPI_trait::PORT | _BV(board_traits::SPI_trait::MISO)) &
										~(_BV(board_traits::SPI_trait::MOSI) | _BV(board_traits::SPI_trait::SCK));
	}
}
#else
// Handle USI for ATtiny
void spi::init()
{
	synchronized
	{
		// Set MOSI and SCK as Output
		// Set MISO as Input (high impedance)
		board_traits::SPI_trait::DDR = (board_traits::SPI_trait::DDR & ~_BV(board_traits::SPI_trait::MISO)) |
									   _BV(board_traits::SPI_trait::MOSI) | _BV(board_traits::SPI_trait::SCK);
		// Set MISO as pullup and force MOSI and SCK low
		//TODO not sure this is really needed
		// board_traits::SPI_trait::PORT = (board_traits::SPI_trait::PORT | _BV(board_traits::SPI_trait::MISO)) &
		// 								~(_BV(board_traits::SPI_trait::MOSI) | _BV(board_traits::SPI_trait::SCK));
	}
}
#endif
