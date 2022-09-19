//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * RFID 125KHz Grove reader example, Wiegand mode, using 2 external interrupt pins.
 * This program shows usage of FastArduino Grove 125KHz RFID Reader support.
 * It displays the ID of tags that are nearing the device coil.
 * 
 * Wiring: 
 * - on Arduino UNO:
 *   - D2 (EXT0): connect to Grove DATA0 (white cable)
 *   - D3 (EXT1): connect to Grove DATA1 (yellow cable)
 *   - USB: connect to a console
 */

#include <fastarduino/uart.h>
#include <fastarduino/devices/grove_rfid_reader.h>
#include <fastarduino/time.h>

#if !defined(ARDUINO_UNO)
#error "Current target is not yet supported!"
#endif

#define USART_NUM 0
static constexpr board::USART USART = board::USART::USART0;
using UATX = serial::hard::UATX<USART>;

#define DATA0_INT 0
static constexpr board::ExternalInterruptPin DATA0 = board::ExternalInterruptPin::D2_PD2_EXT0;
#define DATA1_INT 1
static constexpr board::ExternalInterruptPin DATA1 = board::ExternalInterruptPin::D3_PD3_EXT1;

using GROVE = devices::rfid::Grove125KHzRFIDReaderWiegandEXT<DATA0, DATA1>;

static constexpr uint8_t DEBUG_OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[DEBUG_OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(USART_NUM)
REGISTER_OSTREAMBUF_LISTENERS(UATX)
REGISTER_GROVE_RFID_READER_INT_ISR(DATA0_INT, DATA1_INT, GROVE)

using streams::endl;
using streams::hex;
using streams::uppercase;

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

	GROVE rfid;
	rfid.begin();

	while (true)
	{
		if (rfid.has_data())
		{
			GROVE::DATA_TYPE data;
			rfid.get_data(data);
			// display data to SW serial
			out << uppercase << hex << data << endl;
		}
		time::delay_ms(100);
	}

	rfid.end();
	return 0;
}
