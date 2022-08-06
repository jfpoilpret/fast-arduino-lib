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
 * Simple ranger example, using VL53L0X Time-of-flight range sensor I2C device.
 * This program uses FastArduino VL53L0X support API.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to VL53L0X SDA pin
 *   - A5 (PC5, SCL): connected to VL53L0X SCL pin
 *   - direct USB access
 */

#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/i2c_status.h>
#include <fastarduino/devices/vl53l0x.h>

// #define FORCE_SYNC

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#else
using MANAGER = i2c::I2CSyncManager<MODE>;
#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
REGISTER_I2C_ISR(MANAGER)
#endif

using streams::dec;
using streams::hex;
using streams::fixed;
using streams::endl;
using streams::flush;

using namespace devices::vl53l0x;
using TOF = VL53L0X<MANAGER>;

void display_status(streams::ostream& out, TOF& tof)
{
	DeviceStatus status;
	bool ok = tof.get_range_status(status);
	out << F("tof.get_range_status(status) = ") << ok 
		<< F(", error = ") << dec << status.error()
		<< F(", data_ready = ") << status.data_ready() << endl;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	// open UART for traces
	serial::hard::UATX<UART> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out << streams::boolalpha;
	out << F("Start") << endl;

	// Initialize I2C async handler
#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	MANAGER manager{i2c_buffer};
#else
	MANAGER manager;
#endif

	out << F("Instantiate VL53L0X") << endl;
	TOF tof{manager};

	out << F("Start I2C manager") << endl;
	manager.begin();

	bool ok = false;

	{
		// Call first initialization step
		out << F("Calling init_data_first()...") << endl;
		ok = tof.init_data_first();
		out << F("tof.init_data_first() = ") << ok << endl;
	}

	{
		// Call second initialization step
		out << F("Calling init_static_second()...") << endl;
		constexpr SequenceSteps STEPS = SequenceSteps::all().no_msrc().no_tcc();
		ok = tof.init_static_second(GPIOSettings::sample_ready(), STEPS);
		out << F("tof.init_static_second() = ") << ok << endl;
	}

	{
		// Perform reference calibration
		ok = tof.perform_ref_calibration();
		out << F("tof.perform_ref_calibration() = ") << ok << endl;
	}

	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.begin();

	for (uint8_t i = 0; i < 60; ++i)
	{
		time::delay_ms(1000U);
		// Read continuous ranges now
		uint16_t range = 0;
		time::RTTTime now = rtt.time();
		ok = tof.await_single_range(rtt, range);
		time::RTTTime duration = rtt.time() - now;
		out << F("tof.await_single_range() = ") << ok << endl;
		out << F("single range after ") << duration.millis() << "ms " << duration.micros() << "us" << streams::endl;
		display_status(out, tof);
		if (ok)
			out << F("Range = ") << dec << range << F("mm") << endl;
	}

	manager.end();
}
