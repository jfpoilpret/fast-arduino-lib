//   Copyright 2016-2017 Jean-Francois Poilpret
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

#include <fastarduino/time.h>
#include <fastarduino/devices/mpu6050.h>

#if defined(ARDUINO_UNO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#else
#error "Current target is not yet supported!"
#endif

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
#if HARDWARE_UART
static serial::hard::UATX<UART> uart{output_buffer};
#else
static serial::soft::UATX<TX> uart{output_buffer};
#endif
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

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
using streams::flush;

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
		out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << '\n' << flush;
}

using ACCELEROMETER = MPU6050<i2c::I2CMode::Fast>;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
#if HARDWARE_UART
	uart.register_handler();
#endif
	uart.begin(115200);
	out.width(2);
	out << F("Start\n") << flush;

	ACCELEROMETER::MANAGER manager;
	manager.begin();
	out << F("I2C interface started\n") << flush;

	ACCELEROMETER mpu{manager};
	
	bool ok = mpu.begin(GyroRange::RANGE_250, AccelRange::RANGE_2G);
	out << dec << F("begin() ") << ok << '\n' << flush;
	while (true)
	{
		AllSensors sensors;
		ok = mpu.all_measures(sensors);
		out << dec << F("all_measures() ") << ok << '\n' << flush;
		if (ok)
		{
			out	<< dec 
				<< F("raw Gyro x = ") << sensors.gyro.x 
				<< F(", y = ") << sensors.gyro.y 
				<< F(", z = ") << sensors.gyro.z << '\n' << flush;
			out	<< dec 
				<< F("cdps Gyro x = ") << gyro(sensors.gyro.x)
				<< F(", y = ") << gyro(sensors.gyro.y) 
				<< F(", z = ") << gyro(sensors.gyro.z) << '\n' << flush;
			out	<< dec 
				<< F("raw Accel x = ") << sensors.accel.x 
				<< F(", y = ") << sensors.accel.y 
				<< F(", z = ") << sensors.accel.z << '\n' << flush;
			out	<< dec 
				<< F("mG Accel x = ") << accel(sensors.accel.x) 
				<< F(", y = ") << accel(sensors.accel.y) 
				<< F(", z = ") << accel(sensors.accel.z) << '\n' << flush;
			// Also check the temperature precision as per datasheet
			out << dec << F("Temp = ") << mpu.convert_temp_to_centi_degrees(sensors.temperature) << F(" centi-C\n") << flush;
		}
		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End\n") << flush;
}
