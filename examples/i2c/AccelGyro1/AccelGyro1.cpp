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
 * Read and display sensor values from MPU6050 3D gyroscope/accelerometer (I2C device).
 * Values are displayed to the UART console.
 * 
 * Wiring:
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to MPU6050 SDA pin
 *   - A5 (PC5, SCL): connected to MPU6050 SCL pin
 *   - direct USB access
 * - on Arduino LEONARDO:
 *   - D2 (PD1, SDA): connected to MPU6050 SDA pin
 *   - D3 (PD0, SCL): connected to MPU6050 SDA pin
 *   - direct USB access
 * - on Arduino MEGA:
 *   - D20 (PD1, SDA): connected to MPU6050 SDA pin
 *   - D21 (PD0, SCL): connected to MPU6050 SDA pin
 *   - direct USB access
 * - on ATtinyX4 based boards:
 *   - D6 (PA6, SDA): connected to MPU6050 SDA pin
 *   - D4 (PA4, SCL): connected to MPU6050 SDA pin
 *   - D8 (PB0, TX): connected to SerialUSB converter
 * - on ATmega644 based boards:
 *   - D17 (PC1, SDA): connected to MPU6050 SDA pin
 *   - D16 (PC0, SCL): connected to MPU6050 SCL pin
 *   - D25 (PD1): TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/time.h>
#include <fastarduino/devices/mpu6050.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t DEBUG_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t DEBUG_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t DEBUG_SIZE = 32;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 32;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t DEBUG_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

#if HARDWARE_UART
	REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
	REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#endif

// #define DEBUG_I2C
#define FORCE_SYNC

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
#if HARDWARE_UART
static serial::hard::UATX<UART> uart{output_buffer};
#else
static serial::soft::UATX<TX> uart{output_buffer};
#endif
static streams::ostream out = uart.out();

using utils::UnitPrefix;
using utils::map_raw_to_physical;
using devices::magneto::MPU6050;
using devices::magneto::AllSensors;
using devices::magneto::AccelRange;
using devices::magneto::GyroRange;
using devices::magneto::ACCEL_RANGE_G;
using devices::magneto::GYRO_RANGE_DPS;

#ifdef DEBUG_I2C
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusDebugManager<i2c::I2CMode::FAST, DEBUGGER&, DEBUGGER&>;
#	endif
#define DEBUG(OUT) debugger.trace(OUT)
#define RESET_DEBUG() debugger.reset()

#else

#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
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
using streams::endl;

static constexpr const GyroRange GYRO_RANGE = GyroRange::RANGE_250;
static constexpr const AccelRange ACCEL_RANGE = AccelRange::RANGE_2G;

inline int16_t gyro(int16_t value)
{
	return map_raw_to_physical(value, UnitPrefix::CENTI, GYRO_RANGE_DPS(GYRO_RANGE), 15);
}
inline int16_t accel(int16_t value)
{
	return map_raw_to_physical(value, UnitPrefix::MILLI, ACCEL_RANGE_G(ACCEL_RANGE), 15);
}

void trace_i2c_status(uint8_t expected_status, uint8_t actual_status)
{
	if (expected_status != actual_status)
		out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << endl;
}

using ACCELEROMETER = MPU6050<MANAGER>;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	uart.begin(115200);
	out.width(2);
	out << streams::boolalpha;
	out << F("Start") << endl;

	// Initialize I2C async handler
#ifdef DEBUG_I2C
	DEBUGGER debugger;
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

	manager.begin();
	out << F("I2C interface started") << endl;

	ACCELEROMETER mpu{manager};
	
	bool ok = mpu.begin(GyroRange::RANGE_250, AccelRange::RANGE_2G);
	out << dec << F("begin() ") << ok << endl;
	DEBUG(out);
	while (true)
	{
		AllSensors sensors;
		ok = mpu.all_measures(sensors);
		out << dec << F("all_measures() ") << ok << endl;
		DEBUG(out);
		if (ok)
		{
			out	<< dec 
				<< F("raw Gyro x = ") << sensors.gyro.x 
				<< F(", y = ") << sensors.gyro.y 
				<< F(", z = ") << sensors.gyro.z << endl;
			out	<< dec 
				<< F("cdps Gyro x = ") << gyro(sensors.gyro.x)
				<< F(", y = ") << gyro(sensors.gyro.y) 
				<< F(", z = ") << gyro(sensors.gyro.z) << endl;
			out	<< dec 
				<< F("raw Accel x = ") << sensors.accel.x 
				<< F(", y = ") << sensors.accel.y 
				<< F(", z = ") << sensors.accel.z << endl;
			out	<< dec 
				<< F("mG Accel x = ") << accel(sensors.accel.x) 
				<< F(", y = ") << accel(sensors.accel.y) 
				<< F(", z = ") << accel(sensors.accel.z) << endl;
			// Also check the temperature precision as per datasheet
			out << hex << F("Raw temp = ") << sensors.temperature << endl;
			out << dec << F("Temp = ") << mpu.convert_temp_to_centi_degrees(sensors.temperature) << F(" centi-C") << endl;
		}
		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
