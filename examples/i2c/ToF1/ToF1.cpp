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

#define DEBUG_I2C
// #define FORCE_SYNC

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 96;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#define DEBUG(OUT) debugger.trace(OUT)
#define RESET_DEBUG() debugger.reset()
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using DEBUGGER = i2c::debug::I2CDebugStatusLiveLogger;
#define DEBUG(OUT)
#define RESET_DEBUG()
using MANAGER = i2c::I2CSyncStatusDebugManager<i2c::I2CMode::FAST, DEBUGGER&, DEBUGGER&>;
#	endif

#else

#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncManager<i2c::I2CMode::FAST>;
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

void trace(streams::ostream& out, SequenceSteps steps)
{
	out << F("TCC=") << steps.is_tcc()
		<< F(", DSS=") << steps.is_dss()
		<< F(", MSRC=") << steps.is_msrc()
		<< F(", PRE_RANGE=") << steps.is_pre_range()
		<< F(", FINAL_RANGE=") << steps.is_final_range() << endl;
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

	out << F("Instantiate VL53L0X") << endl;
	TOF tof{manager};

	out << F("Start I2C manager") << endl;
	manager.begin();

	out << F("Read VL53L0X status") << endl;
	uint8_t result = 0;
	bool ok = tof.get_revision(result);
	out << F("tof.get_revision(result) = ") << ok << F(", result = ") << hex << result << endl;
	DEBUG(out);

	ok = tof.get_model(result);
	out << F("tof.get_model(result) = ") << ok << F(", result = ") << hex << result << endl;
	DEBUG(out);

	PowerMode mode = PowerMode::STANDBY;
	ok = tof.get_power_mode(mode);
	out << F("tof.get_power_mode(mode) = ") << ok << F(", mode = ") << dec << uint8_t(mode) << endl;
	DEBUG(out);

	DeviceStatus status;
	ok = tof.get_range_status(status);
	out << F("tof.get_range_status(status) = ") << ok 
		<< F(", error = ") << dec << uint8_t(status.error())
		<< F(", data_ready = ") << status.data_ready() << endl;
	DEBUG(out);

	SequenceSteps steps1;
	ok = tof.get_sequence_steps(steps1);
	out << F("tof.get_sequence_steps(status) = ") << ok << F(", steps =") << flush;
	trace(out, steps1);
	DEBUG(out);

	constexpr SequenceSteps steps2 = SequenceSteps::create().tcc().pre_range().final_range();
	out << F("steps2 = ") << hex << steps2.value() << endl;
	ok = tof.set_sequence_steps(steps2);
	out << F("tof.get_sequence_steps(status) = ") << ok << endl;
	DEBUG(out);

	SequenceSteps steps3;
	ok = tof.get_sequence_steps(steps3);
	out << F("tof.get_sequence_steps(status) = ") << ok << F(", steps =") << flush;
	trace(out, steps3);
	DEBUG(out);

	uint8_t period = 0;
	ok = tof.get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(period);
	out << F("tof.get_vcsel_pulse_period<PRE_RANGE>(period) = ") << ok << F(", period = ") << dec << period << endl;
	ok = tof.get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(period);
	out << F("tof.get_vcsel_pulse_period<FINAL_RANGE>(period) = ") << ok << F(", period = ") << dec << period << endl;
	DEBUG(out);

	// The following block adds 4KB to program size (float arithmetic libs)
	// float signal_rate = 0.0;
	// ok = tof.get_signal_rate_limit(signal_rate);
	// out << F("tof.get_signal_rate_limit(signal_rate) = ") << ok << F(", signal_rate = ")
	// 	<< fixed << signal_rate << endl;
	// ok = tof.set_signal_rate_limit(0.5f);
	// out << F("tof.set_signal_rate_limit(0.5) = ") << ok << endl;
	// ok = tof.get_signal_rate_limit(signal_rate);
	// out << F("tof.get_signal_rate_limit(signal_rate) = ") << ok << F(", signal_rate = ")
	// 	<< fixed << signal_rate << endl;
	// DEBUG(out);

	// Call first initialization step
	out << F("Calling init_data_first()...") << endl;
	TOF::InitDataFuture future{};
	int error = tof.init_data_first(future);
	out << F("tof.init_data_first(future) = ") << dec << error << endl;
	time::delay_ms(100);
	DEBUG(out);
	out << F("future.status() = ") << future.status() << endl;

	ok = tof.get_range_status(status);
	out << F("tof.get_range_status(status) = ") << ok 
		<< F(", error = ") << dec << uint8_t(status.error())
		<< F(", data_ready = ") << status.data_ready() << endl;
	DEBUG(out);

	manager.end();
}
