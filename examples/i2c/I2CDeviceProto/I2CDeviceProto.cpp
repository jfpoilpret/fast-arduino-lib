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
#include <fastarduino/i2c_device.h>
#include <fastarduino/utilities.h>
#include <fastarduino/uart.h>

#include <math.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

// I2C Device specific stuff goes here
//=====================================
static constexpr const i2c::I2CMode MODE = i2c::I2CMode::Fast;
static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;

//static constexpr const uint8_t SMPRT_DIV = 0x19;
static constexpr const uint8_t CONFIG = 0x1A;
static constexpr const uint8_t ACCEL_XOUT = 0x3B;
static constexpr const uint8_t TEMP_OUT = 0x41;
static constexpr const uint8_t GYRO_XOUT = 0x43;
//static constexpr const uint8_t USER_CTRL = 0x6A;
//static constexpr const uint8_t PWR_MGMT_2 = 0x6C;
static constexpr const uint8_t WHO_AM_I = 0x75;

static constexpr const uint8_t GYRO_CONFIG = 0x1B;
enum class GyroRange: uint8_t
{
	RANGE_250	= 0 << 3,
	RANGE_500	= 1 << 3,
	RANGE_1000	= 2 << 3,
	RANGE_2000	= 3 << 3,
};
static constexpr const uint8_t ACCEL_CONFIG = 0x1C;
enum class AccelRange: uint8_t
{
	RANGE_2G	= 0 << 3,
	RANGE_4G	= 1 << 3,
	RANGE_8G	= 2 << 3,
	RANGE_16G	= 3 << 3,
};

static constexpr const uint8_t PWR_MGMT_1 = 0x6B;
struct PowerManagement
{
	PowerManagement(): clock_select{}, temp_disable{}, reserved{}, cycle{}, sleep{}, device_reset{} {}
	uint8_t clock_select	:3;
	uint8_t temp_disable	:1;
	uint8_t reserved		:1;
	uint8_t cycle			:1;
	uint8_t sleep			:1;
	uint8_t device_reset	:1;
};
enum class ClockSelect: uint8_t
{
	INTERNAL_8MHZ		= 0,
	PLL_X_AXIS_GYRO		= 1,
	PLL_Y_AXIS_GYRO		= 2,
	PLL_Z_AXIS_GYRO		= 3,
	PLL_EXTERNAL_32KHZ	= 4,
	PLL_EXTERNAL_19MHZ	= 5,
	STOPPED				= 7
};

struct Sensor3D
{
	int16_t	x;
	int16_t	y;
	int16_t	z;
};

struct AllSensors
{
	Sensor3D	gyro;
	int16_t		temperature;
	Sensor3D	accel;
};

void format_sensors(AllSensors& sensors)
{
	utils::swap_bytes(sensors.gyro.x);
	utils::swap_bytes(sensors.gyro.y);
	utils::swap_bytes(sensors.gyro.z);
	utils::swap_bytes(sensors.temperature);
	utils::swap_bytes(sensors.accel.x);
	utils::swap_bytes(sensors.accel.y);
	utils::swap_bytes(sensors.accel.z);
}

// Subclass I2CDevice to make protected methods available
class PublicDevice: public i2c::I2CDevice<MODE>
{
public:
	PublicDevice(MANAGER& manager): I2CDevice(manager) {}
	friend int main();
};

using streams::flush;
using streams::dec;
using streams::hex;

void trace_i2c_status(uint8_t expected_status, uint8_t actual_status)
{
	if (expected_status != actual_status)
		out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << '\n' << flush;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(2);
//	out << F("Start\n") << flush;
	
	// Start TWI interface
	//====================
	PublicDevice::MANAGER manager{trace_i2c_status};
	manager.begin();
	out << F("I2C interface started\n") << flush;
//	out << hex << F("status #1 ") << manager.status() << '\n' << flush;
	
	PublicDevice device{manager};
	
	// Check chip ID
	uint8_t id;
	device.write(DEVICE_ADDRESS, WHO_AM_I, i2c::BusConditions::START_NO_STOP);
	device.read(DEVICE_ADDRESS, id, i2c::BusConditions::REPEAT_START_STOP);
	out << hex << F("MPU-6050 ID = ") << id << '\n' << flush;
	
	// Init MPU6050: wake it up, set ranges for accelerometer and gyroscope
	PowerManagement power;
	device.write(DEVICE_ADDRESS, GYRO_CONFIG, i2c::BusConditions::START_NO_STOP);
	device.write(DEVICE_ADDRESS, uint8_t(GyroRange::RANGE_250), i2c::BusConditions::NO_START_NO_STOP);
//	device.write(DEVICE_ADDRESS, uint8_t(AccelRange::RANGE_2G), i2c::BusConditions::NO_START_STOP);
	device.write(DEVICE_ADDRESS, uint8_t(AccelRange::RANGE_2G), i2c::BusConditions::NO_START_NO_STOP);
	//TODO try repeat start instead!
	device.write(DEVICE_ADDRESS, PWR_MGMT_1, i2c::BusConditions::REPEAT_START_NO_STOP);
//	device.write(DEVICE_ADDRESS, PWR_MGMT_1, i2c::BusConditions::START_NO_STOP);
	device.write(DEVICE_ADDRESS, power, i2c::BusConditions::NO_START_STOP);
//	out << hex << F("status #2 ") << manager.status() << '\n' << flush;
	
	//TODO is that necessary?
//	time::delay_ms(200);
	
	//TODO loop to show measures
	while (true)
	{
		// Read accel/temperature/gyro in one read
		AllSensors sensors;
		device.write(DEVICE_ADDRESS, ACCEL_XOUT, i2c::BusConditions::START_NO_STOP);
		device.read(DEVICE_ADDRESS, sensors, i2c::BusConditions::REPEAT_START_STOP);
		format_sensors(sensors);
		// trace sensors
		out << dec << F("Gyro x = ") << sensors.gyro.x << F(", y = ") << sensors.gyro.y << F(", z = ") << sensors.gyro.z << '\n' << flush;
		out << dec << F("Accel x = ") << sensors.accel.x << F(", y = ") << sensors.accel.y << F(", z = ") << sensors.accel.z << '\n' << flush;
//		out << dec << F("Temp = ") << sensors.temperature << '\n' << flush;
		//TODO methods to convert data: accel, gyro, temperature
		//TODO avoid floating point arithmetic, rather with integral only, eg use milli-degrees instead of degrees
		// Also check the temperature precision as per datasheet
		float temp = sensors.temperature / 340.0 + 36.53;
		out << F("Temp = ") << temp << "C\n" << flush;
		
		time::delay_ms(1000);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
//	out << hex << F("status #4 ") << manager.status() << '\n' << flush;
	out << F("End\n") << flush;
}
