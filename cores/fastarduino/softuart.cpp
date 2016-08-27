#include <util/delay_basic.h>

#include "softuart.hh"

void Soft::AbstractUATX::_begin(uint32_t rate, Soft::Serial::Parity parity, Soft::Serial::StopBits stop_bits)
{
	_parity = parity;
	// Calculate timing for TX in number of cycles
	uint16_t bit_time = uint16_t(F_CPU / rate);
	// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2()
	// 11 or 12 cycles + delay counted from start bit (cbi) to first bit (sbi or cbi)
	_interbit_tx_time = (bit_time - 12) / 4;
	// 11 or 12 cycles + delay counted from first bit (sbi or cbi) to second bit (sbi or cbi)
	_start_bit_tx_time = (bit_time - 12) / 4;
	// For stop bit we lengthten the bit duration of 25% to guarantee alignment of RX side on stop duration
	_stop_bit_tx_time = (bit_time / 4) * 5 / 4;
	if (stop_bits == Soft::Serial::StopBits::TWO) _stop_bit_tx_time *= 2;
}

constexpr uint16_t compute_delay(uint16_t total_cycles, uint16_t less_cycles)
{
	return (total_cycles > less_cycles ? (total_cycles - less_cycles + 3) / 4 : 1);
}

void Soft::AbstractUARX::_begin(uint32_t rate, Soft::Serial::Parity parity, Soft::Serial::StopBits stop_bits)
{
	//TODO
	_parity = parity;
	// Calculate timing for RX in number of cycles
	uint16_t bit_time = uint16_t(F_CPU / rate);
	// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2()
	// 87 cycles (+4N delay) elapse until start bit is detected from PCI interrupt and 1st bit is sampled:
	// - 3 cycles to generate the PCI interrupt
	// - 1-4 (take 2) cycles to complete current instruction
	// - 4 cycles to process the interrupt + 2 cycles rjmp in vector table
	// - 50 cycles spent in PCINT vector to save context and call handler
	// - 4 cycles spent in virtual handler thunk
	// - 4N + 4 in delay
	// - 15 cycles until first bit sample read
	// For sampling of first bit we wait until middle of first bit
	// We add 3 cycles to allow rounding
	_start_bit_rx_time = compute_delay(3 * bit_time / 2, 84);
//	_start_bit_rx_time = compute_delay(3 * bit_time / 2, 81);
	
	// 8+4+4N cycles elapse between processing of each bit
//	_interbit_rx_time = compute_delay(bit_time, 12);
	_interbit_rx_time = compute_delay(bit_time, 12);
	
	// If no parity, 64+4N cycles elapse until stop bit, we only want to be sure we got the stop bit edge before
	// re-enabling PCINT
	// - 11 cycles to call _push
	// - 41 cycles in _push
	// - 4+4N cycles in delay
	// - 5 cycles to re-enable PCI
	// - 5 additional cycles to make sure we passed the edge of stop bit
	_stop_bit_rx_time = compute_delay(bit_time / 2, 66);
//	_stop_bit_rx_time = compute_delay(bit_time / 2, 61);
	
	// We don't care about actual stop bit time, we just ensure we are raeady for PCI before the end on stop bit
	// Additionally, 49 cycles elapse until next PCI can be handled
	
}

Soft::Serial::Parity Soft::AbstractUATX::calculate_parity(Serial::Parity parity, uint8_t value)
{
	if (parity == Soft::Serial::Parity::NONE) return Soft::Serial::Parity::NONE;
	bool odd = false;
	while (value)
	{
		if (value & 0x01) odd = !odd;
		value >>= 1;
	}
	return (odd ? Soft::Serial::Parity::ODD : Soft::Serial::Parity::EVEN);
}
