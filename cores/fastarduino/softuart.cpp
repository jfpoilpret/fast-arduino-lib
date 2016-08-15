#include <util/delay_basic.h>

#include "softuart.hh"

void Soft::AbstractUAT::_begin(uint32_t rate, Parity parity, StopBits stop_bits)
{
	_parity = parity;
	_stop_bits = stop_bits;
	// Calculate timing for TX in number of cycles
	uint16_t bit_time = uint16_t(F_CPU / rate);
	// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2()
	// 11 or 12 cycles + delay counted from start bit (cbi) to first bit (sbi or cbi)
	_interbit_tx_time = (bit_time - 12) / 4;
	// 11 or 12 cycles + delay counted from first bit (sbi or cbi) to second bit (sbi or cbi)
	_start_bit_tx_time = (bit_time - 12) / 4;
	// For stop bit we lengthten the bit duration of 25% to guarantee alignment of RX side on stop duration
	_stop_bit_tx_time = (bit_time / 4) * 5 / 4;
	if (stop_bits == StopBits::TWO) _stop_bit_tx_time *= 2;
}

Soft::AbstractUAT::Parity Soft::AbstractUAT::calculate_parity(uint8_t value)
{
	if (_parity == Parity::NONE) return Parity::NONE;
	bool odd = false;
	while (value)
	{
		if (value & 0x01) odd = !odd;
		value >>= 1;
	}
	return (odd ? Parity::ODD : Parity::EVEN);
}

