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
 * RFID 125KHz Grove reader example, UART mode, using Hardware UART.
 * This program shows usage of FastArduino Grove 125KHz RFID Reader support.
 * It displays the ID of tags that are nearing the device coil.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - RX (D0): connect to Grove TX (yellow cable)
 *   - D2: connect to serial USB converter, connected to a console
 */

#include <fastarduino/uart.h>
#include <fastarduino/soft_uart.h>
#include <fastarduino/devices/grove_rfid_reader.h>
#include <fastarduino/time.h>

#if !defined(ARDUINO_UNO)
#error "Current target is not yet supported!"
#endif

#define USART_NUM 0
static constexpr board::USART USART = board::USART::USART0;
using UARX = serial::hard::UARX<USART>;
using GROVE = devices::rfid::Grove125KHzRFIDReaderUART<UARX>;

static constexpr board::DigitalPin UATX_TX = board::DigitalPin::D2;
using UATX = serial::soft::UATX<UATX_TX>;

static constexpr uint8_t RFID_INPUT_BUFFER_SIZE = 64;
static char input_buffer[RFID_INPUT_BUFFER_SIZE];

static constexpr uint8_t DEBUG_OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[DEBUG_OUTPUT_BUFFER_SIZE];

static constexpr uint8_t TAG_DATA_SIZE = 16;

REGISTER_UARX_ISR(USART_NUM)
REGISTER_OSTREAMBUF_LISTENERS(UATX)

using streams::endl;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	UATX uatx{output_buffer};
	uatx.begin(115200);
	auto out = uatx.out();
	out << F("Starting...") << endl;

	UARX uarx{input_buffer};
	GROVE rfid{uarx};
	rfid.begin();

	while (true)
	{
		if (rfid.has_data())
		{
			char data[TAG_DATA_SIZE];
			rfid.get_data(data, TAG_DATA_SIZE);
			// display data to SW serial
			out << data << endl;
		}
		time::delay_ms(100);
	}

	rfid.end();
	return 0;
}
