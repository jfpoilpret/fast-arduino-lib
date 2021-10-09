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
 * It first asks user to input (through USB console) the profile to use,
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

static DeviceStatus display_status(streams::ostream& out, TOF& tof)
{
	DeviceStatus status;
	bool ok = tof.get_range_status(status);
	out << F("tof.get_range_status(status) = ") << ok
		<< F(", error = ") << dec << status.error()
		<< F(", data_ready = ") << status.data_ready() << endl;
	return status;
}

static Profile input_profile(streams::ostream& out, streams::istream& in)
{
	while (true)
	{
		out << F("1. Standard profile") << endl;
		out << F("2. Long range profile") << endl;
		out << F("3. Standard but accurate profile") << endl;
		out << F("4. Long range but accurate profile") << endl;
		out << F("5. Standard but fast profile") << endl;
		out << F("6. Long range but fast profile") << endl;
		uint16_t value = 0;
		in >> value;
		switch (value)
		{
			case 1:
			return Profile::STANDARD;

			case 2:
			return Profile::LONG_RANGE;

			case 3:
			return Profile::STANDARD_ACCURATE;

			case 4:
			return Profile::LONG_RANGE_ACCURATE;

			case 5:
			return Profile::STANDARD_FAST;

			case 6:
			return Profile::LONG_RANGE_FAST;

			default:
			out << F("You must select a value between 1 and 6!") << endl;
			break;
		}
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

	out << F("Define VL53L0X profile...\n") << endl;
	Profile profile = input_profile(out, in);
	CHECK_OK(tof.begin(profile));

	display_status(out, tof);

	// Feedback on all settings
	out << F("Final settings") << endl;
	SequenceSteps steps;
	CHECK_OK(tof.get_sequence_steps(steps));
	out << F("Steps = ") << steps << endl;
	uint8_t current_period = 0;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(current_period));
	out << F("VCSEL PRE-RANGE pulse period = ") << current_period << endl;
	CHECK_OK(tof.get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(current_period));
	out << F("VCSEL FINAL-RANGE pulse period = ") << current_period << endl;
	float rate = 0.0;
	CHECK_OK(tof.get_signal_rate_limit(rate));
	out << F("Signal rate limit = ") << rate << endl;
	uint32_t budget = 0;
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
		{
			DeviceStatus status;
			bool ok = tof.get_range_status(status);
			if (ok && status.error() == DeviceError::RANGE_COMPLETE)
				out << F("Range = ") << dec << range << F("mm") << endl;
			else
				out << '.' << flush;
		}
	}
}
