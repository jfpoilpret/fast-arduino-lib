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
 * Special check for ios dependencies and streams agnosticity (Future, async I2C, VL53L0X).
 * This is for compilation only (UNO only).
 * This program checks the following:
 * - all generic stream inserters defined in FastArduino library work whatever selected stream (real or null)
 * 		- DeviceStatus/DeviceError
 * 		- PowerMode
 * 		- GPIOSettings/GPIOFunction
 * 		- SequenceSteps/SequenceStepsTimeouts
 * 		- SPADInfo
 * 		- Status (i2c)
 * 		- DebugStatus (i2c) 
 * 		- FutureStatus
 * 		- I2CCommand/I2CCommandType
 */

#ifndef ARDUINO_UNO
#error "Current target is not yet supported!"
#endif

// Comment out for normal stream / uncomment for empty stream
#define EMPTY_STREAM

#ifndef EMPTY_STREAM
#include <fastarduino/uart.h>
#include <fastarduino/streams.h>
#else
#include <fastarduino/empty_streams.h>
#endif

#include <fastarduino/gpio.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>
#include <fastarduino/devices/vl53l0x.h>

#ifndef EMPTY_STREAM
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#endif

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
static constexpr const uint8_t DEBUG_SIZE = 96;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#define DEBUG(OUT) debugger.trace(OUT, false)
#define RESET_DEBUG() debugger.reset()
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];

REGISTER_I2C_ISR(MANAGER)

using namespace devices::vl53l0x;
using TOF = VL53L0X<MANAGER>;

REGISTER_FUTURE_STATUS_LISTENERS(MANAGER_FUTURE(MANAGER), i2c::I2CSameFutureGroup<MANAGER>, TOF::GetGPIOSettingsFuture)
REGISTER_FUTURE_OUTPUT_NO_LISTENERS()

void update_led(bool status)
{
	if (!status)
		gpio::FastPinType<board::DigitalPin::LED>::set();
}

int main()
{
	board::init();
	sei();

	// Add LED init for visual info (useful with null stream)
	gpio::FastPinType<board::DigitalPin::LED>::set_mode(gpio::PinMode::OUTPUT, false);

#ifndef EMPTY_STREAM
	// open UART for traces
	serial::hard::UATX<UART> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out << streams::boolalpha;
	// UNITBUF is necessary here because insert operators do not flush ostream themselves but may need much buffer!
	out << streams::unitbuf;
#else
	streams::null_ostream out;
#endif
	out << F("Start\n");

	// Initialize I2C async handler
	DEBUGGER debugger;
	MANAGER manager{i2c_buffer, debugger, debugger};

	out << F("Instantiate VL53L0X\n");
	TOF tof{manager};

	out << F("Start I2C manager\n");
	manager.begin();

	bool ok = false;

	{
		out << F("Reset VL53L0X device\n");
		ok = tof.reset_device();
		out << F("tof.reset_device() = ") << ok << '\n';
		DEBUG(out);
		update_led(ok);
	}

	// Check PowerMode insert operator
	{
		PowerMode mode = PowerMode::STANDBY;
		ok = tof.get_power_mode(mode);
		out << F("tof.get_power_mode(mode) = ") << ok << F(", mode = ") << mode << '\n';
		DEBUG(out);
		update_led(ok);
	}

	// Check DeviceStatus/DeviceError insert operator
	{
		DeviceStatus status;
		ok = tof.get_range_status(status);
		out << F("tof.get_range_status(status) = ") << ok 
			<< F(", status = ") << status << '\n';
		DEBUG(out);
		update_led(ok);
	}

	// Check SequenceSteps/SequenceStepsTimeout insert operator
	{
		SequenceSteps steps;
		ok = tof.get_sequence_steps(steps);
		out << F("tof.get_sequence_steps(status) = ") << ok << F(", steps =") << steps << '\n';
		DEBUG(out);
		update_led(ok);
		SequenceStepsTimeout timeouts;
		ok = tof.get_sequence_steps_timeout(timeouts);
		out << F("tof.get_sequence_steps_timeout() = ") << ok << F(", timeouts =") << timeouts << '\n';
		DEBUG(out);
		update_led(ok);
	}

	// Check SPADInfo insert operator
	{
		SPADInfo SPAD_info;
		ok = tof.get_SPAD_info(SPAD_info);
		out << F("tof.get_SPAD_info() = ") << ok << '\n';
		out << F("SPADInfo = ") << SPAD_info << '\n';
		DEBUG(out);
		update_led(ok);
	}

	// Check GPIOSettings/GPIOFunction insert operator
	{
		GPIOSettings settings;
		ok = tof.get_GPIO_settings(settings);
		out << F("tof.get_GPIO_settings() = ") << ok << '\n';
		out << F("GPIO setting=") << settings << '\n';
		DEBUG(out);
		update_led(ok);
	}

	{
		// Check FutureStatus insert operator
		TOF::GetGPIOSettingsFuture future;
		out << F("GetGPIOSettingsFuture status = ") << future.status() << '\n';
		int result = tof.get_GPIO_settings(future);

		// Check I2CCommand/I2CCommandType insert operator
		out << F("i2c_buffer[0]") << i2c_buffer[0] << '\n';
		update_led(result == 0);

		future.await();
		out << F("GetGPIOSettingsFuture status = ") << future.status() << '\n';
		update_led(future.status() == future::FutureStatus::READY);
	}

	manager.end();
}
