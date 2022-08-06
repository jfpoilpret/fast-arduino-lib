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
 * Simple example, checking all get methods of VL53L0X Time-of-flight range
 * sensor I2C device.
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
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
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
using MANAGER = i2c::I2CAsyncManager<
	MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
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

void check_timing(streams::ostream& out, TOF& tof, SequenceSteps steps)
{
	bool ok = tof.set_sequence_steps(steps);
	out << F("tof.set_sequence_steps(") << steps << F(") = ") << ok << endl;
	out << endl;

	SequenceStepsTimeout timeouts{};
	ok = tof.get_sequence_steps_timeout(timeouts);
	out << F("tof.get_sequence_steps_timeout(timeouts) = ") << ok << flush
		<< F(", pre_range_vcsel_period_pclks = ") << dec << timeouts.pre_range_vcsel_period_pclks() << flush
		<< F(", final_range_vcsel_period_pclks = ") << dec << timeouts.final_range_vcsel_period_pclks() << flush
		<< F(", msrc_dss_tcc_mclks = ") << dec << timeouts.msrc_dss_tcc_mclks() << flush
		<< F(", pre_range_mclks = ") << dec << timeouts.pre_range_mclks() << flush
		<< F(", final_range_mclks = ") << dec << timeouts.final_range_mclks(steps.is_pre_range()) << endl;
	out << F("timeouts.msrc_dss_tcc_us() = ") << dec << timeouts.msrc_dss_tcc_us() << flush
		<< F(", timeouts.pre_range_us() = ") << dec << timeouts.pre_range_us() << flush
		<< F(", timeouts.final_range_us() = ") << dec << timeouts.final_range_us(steps.is_pre_range()) << endl;
	out << endl;

	uint32_t budget_us = 0;
	ok = tof.get_measurement_timing_budget(budget_us);
	out << F("tof.get_measurement_timing_budget() = ") << ok << F(", budget_us = ") << dec << budget_us << endl;
	out << endl;
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

	uint8_t result = 0;
	bool ok = false;

	{
		out << F("Reset VL53L0X device") << endl;
		ok = tof.reset_device();
		display_memory(out);
		out << F("tof.reset_device() = ") << ok << endl;
		DEBUG(out);
	}

	{
		out << F("Read VL53L0X status") << endl;
		ok = tof.get_revision(result);
		display_memory(out);
		out << F("tof.get_revision(result) = ") << ok << F(", result = ") << hex << result << endl;
		DEBUG(out);
	}

	{
		ok = tof.get_model(result);
		display_memory(out);
		out << F("tof.get_model(result) = ") << ok << F(", result = ") << hex << result << endl;
		DEBUG(out);
	}

	{
		PowerMode mode = PowerMode::STANDBY;
		ok = tof.get_power_mode(mode);
		display_memory(out);
		out << F("tof.get_power_mode(mode) = ") << ok << F(", mode = ") << mode << endl;
		DEBUG(out);
	}

	{
		DeviceStatus status;
		ok = tof.get_range_status(status);
		display_memory(out);
		out << F("tof.get_range_status(status) = ") << ok 
			<< F(", status = ") << status << endl;
		DEBUG(out);
	}

	{
		SequenceSteps steps1;
		ok = tof.get_sequence_steps(steps1);
		display_memory(out);
		out << F("tof.get_sequence_steps(status) = ") << ok << F(", steps =") << steps1 << endl;
		DEBUG(out);
	}

	{
		uint8_t period = 0;
		ok = tof.get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(period);
		display_memory(out);
		out << F("tof.get_vcsel_pulse_period<PRE_RANGE>(period) = ") << ok << F(", period = ") << dec << period << endl;
		ok = tof.get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(period);
		display_memory(out);
		out << F("tof.get_vcsel_pulse_period<FINAL_RANGE>(period) = ") << ok << F(", period = ") << dec << period << endl;
		DEBUG(out);
	}

	{
		// The following block adds 3KB to program size (float arithmetic libs)
		float signal_rate = 0.0;
		ok = tof.get_signal_rate_limit(signal_rate);
		display_memory(out);
		out << F("tof.get_signal_rate_limit(signal_rate) = ") << ok << F(", signal_rate = ")
			<< fixed << signal_rate << endl;
		DEBUG(out);
	}

	{
		SPADInfo SPAD_info{};
		ok = tof.get_SPAD_info(SPAD_info);
		display_memory(out);
		out << F("tof.get_SPAD_info() = ") << ok << endl;
		out << F("SPADInfo = ") << SPAD_info << endl;
		DEBUG(out);
	}

	{
		SequenceStepsTimeout timeouts{};
		ok = tof.get_sequence_steps_timeout(timeouts);
		display_memory(out);
		out << F("tof.get_sequence_steps_timeout() = ") << ok << endl;
		out << F("tof.get_sequence_steps_timeout(timeouts) = ") << ok << flush
			<< F(", pre_range_vcsel_period_pclks = ") << dec << timeouts.pre_range_vcsel_period_pclks() << flush
			<< F(", final_range_vcsel_period_pclks = ") << dec << timeouts.final_range_vcsel_period_pclks() << flush
			<< F(", msrc_dss_tcc_mclks = ") << dec << timeouts.msrc_dss_tcc_mclks() << flush
			<< F(", pre_range_mclks = ") << dec << timeouts.pre_range_mclks() << flush
			<< F(", final_range_mclks = ") << dec << timeouts.final_range_mclks(true) << endl;
		// check calculated values
		out << F("timeouts.msrc_dss_tcc_us() = ") << dec << timeouts.msrc_dss_tcc_us() << flush
			<< F(", timeouts.pre_range_us() = ") << dec << timeouts.pre_range_us() << flush
			<< F(", timeouts.final_range_us() = ") << dec << timeouts.final_range_us(true) << endl;
		DEBUG(out);
	}

	{
		uint32_t budget_us = 0;
		ok = tof.get_measurement_timing_budget(budget_us);
		out << F("tof.get_measurement_timing_budget() = ") << ok << F(", budget_us = ") << dec << budget_us << endl;
		display_memory(out);
		DEBUG(out);
	}

	{
		GPIOSettings settings{};
		ok = tof.get_GPIO_settings(settings);
		display_memory(out);
		out << F("tof.get_GPIO_settings() = ") << ok << endl;
		out << F("GPIO setting=") << settings << endl;
		DEBUG(out);
	}

	{
		InterruptStatus status{};
		ok = tof.get_interrupt_status(status);
		display_memory(out);
		out << F("tof.get_interrupt_status(status) = ") << ok
			<< F(", status = ") << hex << uint8_t(status) << endl;
	}

	{
		// Check timings
		check_timing(out, tof, SequenceSteps::all());
		DEBUG(out);
		check_timing(out, tof, SequenceSteps::all().no_dss());
		DEBUG(out);
		check_timing(out, tof, SequenceSteps::all().no_tcc());
		DEBUG(out);
		check_timing(out, tof, SequenceSteps::all().no_msrc());
		DEBUG(out);
		check_timing(out, tof, SequenceSteps::all().no_pre_range());
		DEBUG(out);
		check_timing(out, tof, SequenceSteps::create().pre_range().final_range());
		DEBUG(out);
	}

	manager.end();
}
