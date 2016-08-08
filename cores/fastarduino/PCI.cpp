#include "PCI.hh"
#include "utilities.hh"

void AbstractPCI::_enable_all(volatile uint8_t& pcicr, uint8_t port)
{
	ClearInterrupt clint;
	pcicr |= port;
}

void AbstractPCI::_disable_all(volatile uint8_t& pcicr, uint8_t port)
{
	ClearInterrupt clint;
	pcicr &= ~port;
}

void AbstractPCI::_clear(volatile uint8_t& pcifr, uint8_t port)
{
	ClearInterrupt clint;
	pcifr |= port;
}

void AbstractPCI::_enable(volatile uint8_t& pcmsk, uint8_t mask)
{
	ClearInterrupt clint;
	pcmsk = mask;
}

void AbstractPCI::_enable(volatile uint8_t& pcmsk, Board::InterruptPin pin)
{
	uint8_t mask = _BV(Board::BIT((uint8_t) pin));
	ClearInterrupt clint;
	pcmsk |= mask;
}
