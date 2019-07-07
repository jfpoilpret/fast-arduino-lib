//   Copyright 2016-2019 Jean-Francois Poilpret
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

#include "soft_uart.h"

void serial::soft::AbstractUATX::compute_times(uint32_t rate, StopBits stop_bits)
{
	// Calculate timing for TX in number of cycles
	uint16_t bit_time = uint16_t(F_CPU / rate);
	// 11 or 12 cycles + delay counted from start bit (cbi) to first bit (sbi or cbi)
	start_bit_tx_time_ = (bit_time - 12) / 4;
	// 11 or 12 cycles + delay counted from first bit (sbi or cbi) to second bit (sbi or cbi)
	interbit_tx_time_ = (bit_time - 12) / 4;
	// For stop bit we lengthten the bit duration of 25% to guarantee alignment of RX side on stop duration
	stop_bit_tx_time_ = (bit_time / 4) * 5 / 4;
	if (stop_bits == StopBits::TWO) stop_bit_tx_time_ *= 2;
}

static constexpr uint16_t compute_delay(uint16_t total_cycles, uint16_t less_cycles)
{
	// We add 3 cycles to allow rounding
	return (total_cycles > less_cycles ? (total_cycles - less_cycles + 3) / 4 : 1);
}

//TODO possible code size optimizations by using a static inline constexpr to calculate all values?
void serial::soft::AbstractUARX::compute_times(uint32_t rate, UNUSED bool has_parity, UNUSED StopBits stop_bits)
{
	// Calculate timing for RX in number of cycles
	uint16_t bit_time = uint16_t(F_CPU / rate);

	// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2()

	// Time to wait (_delay_loop_2) between detection of start bit and sampling of first bit
	// For sampling of first bit we wait until middle of first bit
	// We remove processing time due to ISR call and ISR code:
	// - 3 cycles to generate the PCI interrupt
	// - 1-4 (take 2) cycles to complete current instruction
	// - 4 cycles to process the interrupt + 2 cycles rjmp in vector table
	// - 32 cycles spent in PCINT vector to save context and check stop bit (sbic)
	// - 8 cycles to setup stack variables
	// - (4N) + 4 in delay
	// - 8 cycles until first bit sample read (sbis)
	start_bit_rx_time_ = compute_delay(3 * bit_time / 2, 3 + 2 + 4 + 2 + 32 + 8 + 4 + 8);

	// Time to wait (_delay_loop_2) between sampling of 2 consecutive data bits
	// This is also use between last bit and parity bit (if checked) or stop bit
	// We have to wait exactly for `bit_time` cycles
	// We remove processing time due to each bit sampling and data value update
	// - 10+4N cycles elapse between processing of each bit
	interbit_rx_time_ = compute_delay(bit_time, 10);

	// No extra delay is used fater sampling of first stop bit (as there are already 
	// enough code cycles used for pushing data and restoring context on ISR leave)
	// Actually, >80 cycles elapse until next PCI can be handled
}

serial::Parity serial::soft::AbstractUATX::calculate_parity(Parity parity, uint8_t value)
{
	if (parity == Parity::NONE) return Parity::NONE;
	bool odd = false;
	while (value)
	{
		if (value & 0x01) odd = !odd;
		value >>= 1;
	}
	return (odd ? Parity::ODD : Parity::EVEN);
}
