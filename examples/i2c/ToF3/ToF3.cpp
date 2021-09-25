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
 * It first asks user to input (through USB console) various VL53L0X settings,
 * then it uses these settings to start continuous ranging and display distance
 * measurements conitnuously (until reset).
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
#include <fastarduino/devices/vl53l0x.h>

// #define FORCE_SYNC

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static constexpr const uint8_t INPUT_BUFFER_SIZE = 32;
static char input_buffer[INPUT_BUFFER_SIZE];

REGISTER_UART_ISR(0)

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
REGISTER_I2C_ISR(MANAGER)
#else
using MANAGER = i2c::I2CSyncManager<MODE>;
#endif

using streams::dec;
using streams::hex;
using streams::fixed;
using streams::endl;
using streams::flush;

using namespace devices::vl53l0x;
using TOF = VL53L0X<MANAGER>;

#define CHECK_OK(STATEMENT) if (! STATEMENT) out << F(#STATEMENT) << F(" ERROR!") << endl

static void display_status(streams::ostream& out, TOF& tof)
{
	DeviceStatus status;
	bool ok = tof.get_range_status(status);
	out << F("tof.get_range_status(status) = ") << ok
		<< F(", error = ") << dec << status.error()
		<< F(", data_ready = ") << status.data_ready() << endl;
}

static bool yes_no(streams::ostream& out, streams::istream& in, const flash::FlashStorage* label)
{
	while (true)
	{
		out << label << flush;
		char answer = 0;
		in >> answer;
		switch (answer)
		{
			case 'y':
			case 'Y':
			out << endl;
			return true;

			case 'n':
			case 'N':
			out << endl;
			return false;

			default:
			out << F("Only Y or N are allowed!") << endl;
		}
	}
}

static constexpr uint8_t PRE_RANGE_VCSEL_VALUES[] = {12, 14, 16, 18};
static constexpr uint8_t PRE_RANGE_VCSEL_COUNT = sizeof(PRE_RANGE_VCSEL_VALUES);
static constexpr uint8_t FINAL_RANGE_VCSEL_VALUES[] = {8, 10, 12, 14};
static constexpr uint8_t FINAL_RANGE_VCSEL_COUNT = sizeof(FINAL_RANGE_VCSEL_VALUES);

static uint8_t vcsel_period(streams::ostream& out, streams::istream& in, const flash::FlashStorage* label,
	uint8_t current, const uint8_t* values, uint8_t count)
{
	while (true)
	{
		out << label << F("(");
		for (uint8_t i = 0; i < count; ++i)
			out << values[i] << ", ";
		out << F("current = ") << current << F("): ") << flush;
		uint16_t answer = 0;
		in >> answer;
		for (uint8_t i = 0; i < count; ++i)
			if (answer == values[i])
			{
				out << endl;
				return answer;
			}
		out << F("Unauthorized value entered!") << endl;
	}
}

static float signal_rate(streams::ostream& out, streams::istream& in, float current)
{
	while (true)
	{
		out << F("Signal rate (float in ]0;1], current = ") << current << F("): ") << flush;
		double answer = 0;
		in >> answer;
		if (answer > 0.0 && answer <= 1.0)
		{
			out << endl;
			return answer;
		}
		out << F("Only floats in ]0;1] are allowed!") << endl;
	}
}

static uint32_t timing_budget(streams::ostream& out, streams::istream& in, uint32_t current)
{
	while (true)
	{
		out << F("Measurement timing budget in us (current = ") << current << F("us): ") << flush;
		uint32_t budget = 0;
		in >> budget;
		if (budget > 0)
		{
			out << endl;
			return budget;
		}
		out << F("Only positive numbers are allowed!") << endl;
	}
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	// open UART for traces
	serial::hard::UART<UART> uart{input_buffer, output_buffer};
	streams::ostream out = uart.out();
	streams::istream in = uart.in();
	uart.begin(115200);
	out << streams::boolalpha;

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

	out << F("Define VL53L0X parameters...\n") << endl;

	// Define steps for init_static_second()
	SequenceSteps steps = SequenceSteps::create();
	out << F("Sequence steps:") << endl;
	if (yes_no(out, in, F("  TCC (Y/N): "))) steps = steps.tcc();
	if (yes_no(out, in, F("  DSS (Y/N): "))) steps = steps.dss();
	if (yes_no(out, in, F("  MSRC (Y/N): "))) steps = steps.msrc();
	if (yes_no(out, in, F("  PRE-RANGE (Y/N): "))) steps = steps.pre_range();
	if (yes_no(out, in, F("  FINAL-RANGE (Y/N): "))) steps = steps.final_range();
	
	// Initialize VL53L0X chip
	out << F("Initialize VL53L0X chip...\n") << endl;
	CHECK_OK(tof.init_data_first());
	CHECK_OK(tof.init_static_second(GPIOSettings::sample_ready(), steps));
	CHECK_OK(tof.perform_ref_calibration());

	// set_vcsel_pulse_period PRE-RANGE/FINAL-RANGE
	out << F("VCSEL pulse period:") << endl;
	uint8_t current_period = 0;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(current_period));
	uint8_t period = vcsel_period(
		out, in, F("  PRE-RANGE "), current_period, PRE_RANGE_VCSEL_VALUES, PRE_RANGE_VCSEL_COUNT);
	CHECK_OK(tof.set_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(period));
	current_period = 0;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(current_period));
	period = vcsel_period(
		out, in, F("  FINAL-RANGE "), current_period, FINAL_RANGE_VCSEL_VALUES, FINAL_RANGE_VCSEL_COUNT);
	CHECK_OK(tof.set_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(period));

	float rate = 0.0;
	CHECK_OK(tof.get_signal_rate_limit(rate));
	rate = signal_rate(out, in, rate);
	CHECK_OK(tof.set_signal_rate_limit(rate));

	uint32_t budget = 0;
	CHECK_OK(tof.get_measurement_timing_budget(budget));
	budget = timing_budget(out, in, budget);
	CHECK_OK(tof.set_measurement_timing_budget(budget));

	display_status(out, tof);

	// Feedback on all settings
	out << F("Final settings") << endl;
	out << F("Steps = ") << steps << endl;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(current_period));
	out << F("VCSEL PRE-RANGE pulse period = ") << current_period << endl;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(current_period));
	out << F("VCSEL FINAL-RANGE pulse period = ") << current_period << endl;
	CHECK_OK(tof.get_signal_rate_limit(rate));
	out << F("Signal rate limit = ") << rate << endl;
	CHECK_OK(tof.get_measurement_timing_budget(budget));
	out << F("Measurement timing budget = ") << budget << F("us") << endl;
	SequenceStepsTimeout timeouts;
	CHECK_OK(tof.get_sequence_steps_timeout(timeouts));
	out << F("Timeouts for each step = ") << timeouts << endl;

	// Start continuous ranging
	CHECK_OK(tof.start_continuous_ranging(1000U));

	while (true)
	{
		time::delay_ms(995U);
		// Read continuous ranges now
		uint16_t range = 0;
		if (tof.await_continuous_range(range))
			out << F("Range = ") << dec << range << F("mm") << endl;
		display_status(out, tof);
	}
}
