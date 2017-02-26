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

/*
 * Example that checks compilation failures (static_assert) due to bad usage of FastArduino library.
 * This example shall never compile successfully but only expose a list of compile-time errors.
 */

#include <fastarduino/fast_io.h>
#include <fastarduino/int.h>
#include <fastarduino/pci.h>
#include <fastarduino/soft_uart.h>
#include <fastarduino/uart.h>

#include <fastarduino/realtime_timer.h>
#include <fastarduino/timer.h>

void callback() {}
struct Callback
{
	void callback() {}
};

// Try to register INT0 vector for a non INT pin
REGISTER_INT_ISR_EMPTY(0, Board::DigitalPin::D0)
REGISTER_INT_ISR_METHOD(0, Board::DigitalPin::D0, Callback, &Callback::callback)
REGISTER_INT_ISR_FUNCTION(0, Board::DigitalPin::D0, callback)
// Try to register INT0 vector for a non INT1 pin
REGISTER_INT_ISR_EMPTY(0, Board::ExternalInterruptPin::D3_EXT1)
REGISTER_INT_ISR_METHOD(0, Board::ExternalInterruptPin::D3_EXT1, Callback, &Callback::callback)
REGISTER_INT_ISR_FUNCTION(0, Board::ExternalInterruptPin::D3_EXT1, callback)

//TODO Try to register PCINT0 vector for a non PCINT pin
//NOTE this is not possible with UNO as all pins are mapped to a PCINT (only possible with MEGA)
// Try to register PCINT0 vector for a PCINT2 pin
REGISTER_PCI_ISR_EMPTY(0, Board::InterruptPin::D0_PCI2)
REGISTER_PCI_ISR_METHOD(0, Callback, &Callback::callback, Board::InterruptPin::D0_PCI2)
REGISTER_PCI_ISR_FUNCTION(0, callback, Board::InterruptPin::D0_PCI2)
// Try to register PCINT0 vector for several PCINT0 pins and one PCINT2 pin
REGISTER_PCI_ISR_EMPTY(0, Board::InterruptPin::D8_PCI0, Board::InterruptPin::D10_PCI0, Board::InterruptPin::D0_PCI2)
REGISTER_PCI_ISR_METHOD(0, Callback, &Callback::callback, Board::InterruptPin::D8_PCI0, Board::InterruptPin::D10_PCI0, Board::InterruptPin::D0_PCI2)
REGISTER_PCI_ISR_FUNCTION(0, callback, Board::InterruptPin::D8_PCI0, Board::InterruptPin::D10_PCI0, Board::InterruptPin::D0_PCI2)

//TODO Try to register SW UART for a non PCINT pin
//NOTE this is not possible with UNO as all pins are mapped to a PCINT (only possible with MEGA)
// Try to register SW UART with bad PCINT pin
REGISTER_UART_PCI_ISR(Board::InterruptPin::D0_PCI2, 0)
// Try to register SW UART with a non INT pin
REGISTER_UART_INT_ISR(Board::DigitalPin::D0, 0)
// Try to register SW UART with a bad INT pin
REGISTER_UART_INT_ISR(Board::ExternalInterruptPin::D3_EXT1, 0)

//IMPORTANT NOTE the following checks generate each a whole bunch of errors because there is no static_assert
// but only "normal" compilation errors, due to use of non existing value for an enum, with plenty of consequent errors

// Try to register HW UART for non existing UART NUM
REGISTER_UATX_ISR(1)
REGISTER_UARX_ISR(1)
REGISTER_UART_ISR(1)

// Try to register TIMER vector for non existing TIMER NUM
REGISTER_TIMER_ISR_EMPTY(3)
REGISTER_TIMER_ISR_METHOD(3, Callback, &Callback::callback)
REGISTER_TIMER_ISR_FUNCTION(3, callback)

// Try to register TIMER vector for RTT for non existing TIMER NUM
REGISTER_RTT_ISR(3)
REGISTER_RTT_ISR_METHOD(3, Callback, &Callback::callback)
REGISTER_RTT_ISR_FUNCTION(3, callback)

int main()
{
	return 0;
}
