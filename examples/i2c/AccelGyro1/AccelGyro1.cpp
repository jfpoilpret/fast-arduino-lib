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
 */

#include <fastarduino/time.h>
#include <fastarduino/devices/new_mpu6050.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static constexpr uint8_t MAX_FUTURES = 128;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
static constexpr uint8_t MAX_FUTURES = 128;
static i2c::I2CCommand i2c_buffer[I2C_BUFFER_SIZE];
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t MAX_FUTURES = 8;
#else
#error "Current target is not yet supported!"
#endif

#if I2C_TRUE_ASYNC
REGISTER_I2C_ISR(i2c::I2CMode::FAST)
#endif

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

using ACCELEROMETER = MPU6050<i2c::I2CMode::FAST>;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	uart.begin(115200);
	out.width(2);
	out << F("Start") << endl;

	// Initialize FutureManager
	future::FutureManager<MAX_FUTURES> future_manager;

	// Initialize I2C async handler
#if I2C_TRUE_ASYNC
	ACCELEROMETER::MANAGER manager{i2c_buffer, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS};
#else
	ACCELEROMETER::MANAGER manager{i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS};
#endif
	manager.begin();
	out << F("I2C interface started") << endl;

	ACCELEROMETER mpu{manager};
	
	bool ok = mpu.begin(GyroRange::RANGE_250, AccelRange::RANGE_2G);
	out << dec << F("begin() ") << ok << endl;
	while (true)
	{
		AllSensors sensors;
		ok = mpu.all_measures(sensors);
		out << dec << F("all_measures() ") << ok << endl;
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
			out << dec << F("Temp = ") << mpu.convert_temp_to_centi_degrees(sensors.temperature) << F(" centi-C") << endl;
		}
		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
