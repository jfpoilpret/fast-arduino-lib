//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * RFID 125KHz Grove reader example, only for compile check.
 * This program demonstrates that grove UART class will compile only for
 * SW or HW UART with RX capability.
 * 
 * Wiring: 
 * - no wiring
 */

#include <fastarduino/uart.h>
#include <fastarduino/soft_uart.h>
#include <fastarduino/devices/grove_rfid_reader.h>

#if !defined(ARDUINO_UNO)
#error "Current target is not yet supported!"
#endif

using board::DigitalPin, board::InterruptPin, board::ExternalInterruptPin, board::USART;
using devices::rfid::Grove125KHzRFIDReaderUART;

// Following types are OK
using GROVE1 = Grove125KHzRFIDReaderUART<serial::hard::UARX<USART::USART0>>;
using GROVE2 = Grove125KHzRFIDReaderUART<serial::hard::UART<USART::USART0>>;
using GROVE3 = Grove125KHzRFIDReaderUART<serial::soft::UARX_EXT<ExternalInterruptPin::D2_PD2_EXT0>>;
using GROVE4 = Grove125KHzRFIDReaderUART<serial::soft::UARX_PCI<InterruptPin::D10_PB2_PCI0>>;
using GROVE5 = Grove125KHzRFIDReaderUART<serial::soft::UART_EXT<ExternalInterruptPin::D2_PD2_EXT0, DigitalPin::D4>>;
using GROVE6 = Grove125KHzRFIDReaderUART<serial::soft::UART_PCI<InterruptPin::D10_PB2_PCI0, DigitalPin::D5>>;

// Following types are NOT OK
using BADGROVE0 = Grove125KHzRFIDReaderUART<int>;
using BADGROVE1 = Grove125KHzRFIDReaderUART<serial::hard::UATX<USART::USART0>>;
using BADGROVE2 = Grove125KHzRFIDReaderUART<serial::soft::UATX<DigitalPin::D6>>;

static constexpr uint8_t BUFFER_SIZE = 64;
static char input_buffer[BUFFER_SIZE];
static char output_buffer[BUFFER_SIZE];

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// The following shall compile
	GROVE1::UART uart1{input_buffer};
	GROVE1 rfid1{uart1};
	
	GROVE2::UART uart2{input_buffer, output_buffer};
	GROVE2 rfid2{uart2};
	
	GROVE3::UART::INT_TYPE enabler3;
	GROVE3::UART uart3{input_buffer, enabler3};
	GROVE3 rfid3{uart3};
	
	GROVE4::UART::PCI_TYPE enabler4;
	GROVE4::UART uart4{input_buffer, enabler4};
	GROVE4 rfid4{uart4};
	
	GROVE5::UART::INT_TYPE enabler5;
	GROVE5::UART uart5{input_buffer, output_buffer, enabler5};
	GROVE5 rfid5{uart5};
	
	GROVE6::UART::PCI_TYPE enabler6;
	GROVE6::UART uart6{input_buffer, output_buffer, enabler6};
	GROVE6 rfid6{uart6};
	
	// The following shall NOT compile
	BADGROVE0::UART baduart0;
	BADGROVE0 badrfid0{baduart0};

	BADGROVE1::UART baduart1{output_buffer};
	BADGROVE1 badrfid1{baduart1};

	BADGROVE2::UART baduart2{output_buffer};
	BADGROVE2 badrfid2{baduart2};

	return 0;
}
