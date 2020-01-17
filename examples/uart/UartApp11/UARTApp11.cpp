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
 * Software UART sample to test begin/end methods.
 * 
 * Wiring:
 * - on Arduino UNO, LEONARDO and MEGA:
 *   - Use standard TX/RX
 */

#include <fastarduino/time.h>
#include <fastarduino/soft_uart.h>

static const board::DigitalPin TX = board::DigitalPin::D1_PD1;
static const board::InterruptPin RX = board::InterruptPin::D0_PD0_PCI2;

// Define vectors we need in the example
REGISTER_UARX_PCI_ISR(RX, 2)

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr uint8_t NUM_SIZES = 10;
static uint8_t sizes[NUM_SIZES];

static void display_input_buffer(streams::ostream& out, const char* label)
{
	out << label << streams::endl;
	for (uint8_t i = 0; i < INPUT_BUFFER_SIZE; ++i)
		out << streams::hex << uint8_t(input_buffer[i]) << ' ';
	out << streams::endl;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Start UATX
	serial::soft::UATX<TX> uatx{output_buffer};
	streams::ostream out = uatx.out();

	// Check buffer handling at end()
	// The following should not appear has output buffer is locked until begin() is called
	out << F("BEFORE: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");

	uatx.begin(9600);
	// The following should partly appear until UATX is ended and buffer cleared
	out << F("FLUSH: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");
	uatx.end();
	time::delay_ms(2000);

	uatx.begin(9600);
	time::delay_ms(2000);

	// Start UARX
	interrupt::PCI_SIGNAL<RX> pci;
	serial::soft::UARX_PCI<RX> uarx{input_buffer, pci};
	pci.enable();
	streams::istream in = uarx.in();
	int value;

	uint8_t index = 0;
	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#1");
	uarx.begin(9600);
	// NOTE: if you type 123 456 (+NL) in console, then 456 will be forgotten
	in >> value;
	time::delay_ms(2000);
	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#2");
	out << F("value=") << streams::dec << value << streams::endl;
	time::delay_ms(2000);
	uarx.end(serial::BufferHandling::CLEAR);

	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#3");
	uarx.begin(9600);
	// NOTE: if you type 456 789 (+NL) in console, then 789 will be available for next step
	in >> value;
	time::delay_ms(2000);
	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#4");
	out << F("value=") << streams::dec << value << streams::endl;
	time::delay_ms(2000);
	uarx.end(serial::BufferHandling::KEEP);

	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#5");
	uarx.begin(9600);
	// NOTE: if you typed 456 789 (+NL) in console beofre then 789 should immediately appear
	in >> value;
	time::delay_ms(2000);
	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#6");
	out << F("value=") << streams::dec << value << streams::endl;
	time::delay_ms(2000);
	uarx.end(serial::BufferHandling::CLEAR);

	sizes[index++] = in.rdbuf().queue().items();
	display_input_buffer(out, "#7");

	out << F("sizes") << streams::endl;
	for (uint8_t i = 0; i < index; ++i)
		out << F("sizes[") << i << F("]=") << sizes[i] << streams::endl;
}
