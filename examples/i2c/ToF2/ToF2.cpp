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
 * Simple ranger example, using VL53L0X Time-of-flight range sensor I2C device.
 * This program uses FastArduino VL53L0X support API.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to VL53L0X SDA pin
 *   - A5 (PC5, SCL): connected to VL53L0X SCL pin
 *   - direct USB access
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>
#include <fastarduino/devices/vl53l0x.h>
#include <fastarduino/memory.h>

// #define DEBUG_I2C
// #define FORCE_SYNC

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
// static constexpr const i2c::I2CMode MODE = i2c::I2CMode::STANDARD;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 96;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#define DEBUG(OUT) debugger.trace(OUT)
#define RESET_DEBUG() debugger.reset()
using MANAGER = i2c::I2CAsyncStatusDebugManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using DEBUGGER = i2c::debug::I2CDebugStatusLiveLogger;
#define DEBUG(OUT)
#define RESET_DEBUG()
using MANAGER = i2c::I2CSyncStatusDebugManager<MODE, DEBUGGER&, DEBUGGER&>;
#	endif

#else

#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncManager<MODE>;
#	endif
#define DEBUG(OUT)
#define RESET_DEBUG()

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

void display_memory(streams::ostream& out)
{
	out << F("free mem=") << dec << memory::free_mem() << endl;
}

void display_status(streams::ostream& out, TOF& tof)
{
	DeviceStatus status;
	bool ok = tof.get_range_status(status);
	display_memory(out);
	out << F("tof.get_range_status(status) = ") << ok 
		<< F(", error = ") << dec << status.error()
		<< F(", data_ready = ") << status.data_ready() << endl;
	DEBUG(out);
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
#ifdef DEBUG_I2C
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	DEBUGGER debugger;
#	else
	DEBUGGER debugger{out};
#	endif
#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
#	ifdef DEBUG_I2C
	MANAGER manager{i2c_buffer, debugger, debugger};
#	else
	MANAGER manager{i2c_buffer};
#	endif
#else
#	ifdef DEBUG_I2C
	MANAGER manager{debugger, debugger};
#	else
	MANAGER manager;
#	endif
#endif

	display_memory(out);

	out << F("Instantiate VL53L0X") << endl;
	TOF tof{manager};
	display_memory(out);

	out << F("Start I2C manager") << endl;
	manager.begin();
	display_memory(out);

	bool ok = false;

	{
		// ok = tof.set_signal_rate_limit(0.5f);
		// display_memory(out);
		// out << F("tof.set_signal_rate_limit(0.5) = ") << ok << endl;
		// DEBUG(out);
	}

	{
		// Call first initialization step
		out << F("Calling init_data_first()...") << endl;
		ok = tof.init_data_first();
		display_memory(out);
		out << F("tof.init_data_first() = ") << ok << endl;
		DEBUG(out);
	}

	display_status(out, tof);

	{
		// Call second initialization step
		out << F("Calling init_static_second()...") << endl;
		constexpr SequenceSteps STEPS = SequenceSteps::all().no_msrc().no_tcc();
		ok = tof.init_static_second(GPIOSettings::sample_ready(), STEPS);
		display_memory(out);
		out << F("tof.init_static_second() = ") << ok << endl;
		DEBUG(out);
	}

	display_status(out, tof);

	{
		// Perform reference calibration
		ok = tof.perform_ref_calibration();
		display_memory(out);
		out << F("tof.perform_ref_calibration() = ") << ok << endl;
		DEBUG(out);
	}

	display_status(out, tof);

	{
		// Start continuous ranging
		ok = tof.start_continuous_ranging(1000U);
		display_memory(out);
		out << F("tof.start_continuous_ranging(1000) = ") << ok << endl;
		DEBUG(out);
	}

	for (uint8_t i = 0; i < 60; ++i)
	{
		time::delay_ms(100U);
		// Read continuous ranges now
		uint16_t range = 0;
		ok = tof.await_continuous_range(range);
		display_memory(out);
		out << F("tof.await_continuous_range() = ") << ok << endl;
		display_status(out, tof);
		if (ok)
			out << F("Range = ") << dec << range << F("mm") << endl;
		// if (tof.await_interrupt())
		// {
		// 	display_status(out, tof);
		// 	DEBUG(out);
		// 	// Read continuous ranges now
		// 	uint16_t range = 0;
		// 	if (tof.get_direct_range(range))
		// 	{
		// 		out << F("Range = ") << dec << range << F("mm") << endl;
		// 	}
		// 	tof.clear_interrupt();
		// 	// DEBUG(out);
		// }
	}

	{
		// Start continuous ranging
		ok = tof.stop_continuous_ranging();
		display_memory(out);
		out << F("tof.stop_continuous_ranging() = ") << ok << endl;
		DEBUG(out);
	}

	manager.end();
}
