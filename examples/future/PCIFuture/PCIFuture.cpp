//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * This program checks FastArduino Future API in use with ISR.
 * It shows the state of a port at the time one of its inputs changed; the port
 * "snapshot" is taken by a PCI ISR and stored into a future for display by main().
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D8-D13 (port B): branch 6 push buttons connected to GND
 *   - Standard USB to console
 * - on Arduino LEONARDO
 *   - D8-D11 (port B): branch 4 push buttons connected to GND
 *   - Standard USB to console
 * - on Arduino MEGA
 *   - A8-A15 (port K): branch 8 push buttons connected to GND
 *   - Standard USB to console
 * - on ATtinyX4 breadboard
 *   - D0-D7 (PA0-7, port A): branch 8 push buttons connected to GND
 *   - D8 (PB0): TX output connected to Serial-USB allowing traces display on a PC terminal
 * - on ATtinyX5 breadboard
 *   - D0,2-4 (PB0,2-4, port B): branch 4 push buttons connected to GND
 *   - D1 (PB1) as TX to a Serial-USB converter
 * 
 * Notes: if you do not connect as many buttons as expected, the example will still 
 * work but it will always show high bits for unconnected inputs.
 */

#include <fastarduino/future.h>
#include <fastarduino/move.h>
#include <fastarduino/time.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/pci.h>
#include <fastarduino/gpio.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr board::USART USART = board::USART::USART0;
static constexpr board::Port PORT = board::Port::PORT_B;
static constexpr uint8_t PORT_MASK = 0x3F;
static constexpr board::InterruptPin IPIN0 = board::InterruptPin::D8_PB0_PCI0;
#define PCINT 0
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr board::USART USART = board::USART::USART1;
static constexpr board::Port PORT = board::Port::PORT_B;
static constexpr uint8_t PORT_MASK = 0xF0;
static constexpr board::InterruptPin IPIN0 = board::InterruptPin::D8_PB4_PCI0;
#define PCINT 0
REGISTER_UATX_ISR(1)
#elif defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr board::USART USART = board::USART::USART0;
static constexpr board::Port PORT = board::Port::PORT_K;
static constexpr uint8_t PORT_MASK = 0xFF;
static constexpr board::InterruptPin IPIN0 = board::InterruptPin::D62_PK0_PCI2;
#define PCINT 2
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr board::Port PORT = board::Port::PORT_A;
static constexpr uint8_t PORT_MASK = 0xFF;
static constexpr board::InterruptPin IPIN0 = board::InterruptPin::D0_PA0_PCI0;
#define PCINT 0
#elif defined (BREADBOARD_ATTINYX5)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D1_PB1;
static constexpr board::Port PORT = board::Port::PORT_B;
static constexpr uint8_t PORT_MASK = 0x1C;
static constexpr board::InterruptPin IPIN0 = board::InterruptPin::D0_PB0_PCI0;
#define PCINT 0
#else
#error "Current target is not yet supported!"
#endif

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr uint8_t MAX_FUTURES = 5;

using namespace future;
using namespace streams;

// global variable holding the id of the future to fill in
static uint8_t port_snapshot_id = 0;

using FUTURE_MANAGER = FutureManager<MAX_FUTURES>;
template<typename OUT = void, typename IN = void> using FUTURE = FUTURE_MANAGER::FUTURE<OUT, IN>;

// PCINT0 ISR
//NOTE: a beter design would define a class encasulating FUTURE_MANAGER instance, and port_snashot_id.
void take_snapshot() {
    gpio::FastPort<PORT> port;
	interrupt::HandlerHolder<FUTURE_MANAGER>::handler()->set_future_value_(port_snapshot_id, port.get_PIN());
}

REGISTER_PCI_ISR_FUNCTION(PCINT, take_snapshot, IPIN0)

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
#if HARDWARE_UART
	serial::hard::UATX<USART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
	uart.begin(115200);
	ostream out = uart.out();
	out << showbase << uppercase;

	// First create a FutureManager singleton
	FUTURE_MANAGER manager;
	// Register Futuremanager to make it accessible by ISR
	interrupt::register_handler(manager);

	// Initialize PORT and PCI
	gpio::FastPort<PORT> port{PORT_MASK, PORT_MASK};
	interrupt::PCI_PORT_SIGNAL<PORT> signal;
	signal.set_enable_pins(PORT_MASK);
	signal.enable();

	out << F("STARTED") << endl;

	while (true)
	{
		// Create a Future and register it
		FUTURE<uint8_t> port_snapshot;
		manager.register_future(port_snapshot);
		port_snapshot_id = port_snapshot.id();
		out << F("ID = ") << dec << port_snapshot_id << endl;

		// Wait for the future to be filled in by the PCINT0 ISR
		uint8_t snapshot = 0;
		if (port_snapshot.get(snapshot)) {
			out << F("SNAPSHOT = ") << hex << (snapshot & PORT_MASK) << endl;
		}
	}
}
