#include <util/delay_basic.h>

#include "softuart.hh"

void Soft::UAT::write(uint8_t value)
{
	// Pre-calculate all what we need: num bits, parity bit
	uint8_t bits = 8 + 1;
	bool has_parity = (_parity != Parity::NONE);
	//TODO optimize by computing parity only if needed
	bool parity_bit = calculate_parity(value, _parity);

	// Write start bit
	_tx.clear();
	// Wait before starting actual value transmission
	_delay_loop_2(_start_bit_tx_time);
	while (--bits)
	{
		if (value & 0x01) _tx.set(); else _tx.clear();
		value >>= 1;
		_delay_loop_2(_interbit_tx_time);
	}
	// Add parity if needed
	if (has_parity)
	{
		if (parity_bit) _tx.set(); else _tx.clear();
		_delay_loop_2(_interbit_tx_time);
	}
	// Add stop bit
	_tx.set();
	_delay_loop_2(_stop_bit_tx_time);
}

bool Soft::UAT::calculate_parity(uint8_t value, Parity parity)
{
	bool odd = false;
	while (value)
	{
		if (value & 0x01) odd = !odd;
		value >>= 1;
	}
	return (parity == Parity::ODD ? !odd : odd);
}
