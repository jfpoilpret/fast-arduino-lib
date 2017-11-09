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

#ifndef MPU6050_H
#define MPU6050_H

#include <math.h>
#include <stdint.h>
#include "../i2c_device.h"
#include "../utilities.h"

namespace devices
{
	//FIXME rename namespace: "magneto" is not relevant here, eg "motion" or "motion_sensor"
	//TODO should we review all devices namespace to split into: sensors, actuators, displays and circuits maybe?
	//TODO then should we also change directories structure or not necessarily (yet)
	namespace magneto
	{
		enum class GyroRange : uint8_t
		{
			RANGE_250	= 0 << 3,
			RANGE_500	= 1 << 3,
			RANGE_1000	= 2 << 3,
			RANGE_2000	= 3 << 3,
		};
		
		static constexpr uint16_t GYRO_RANGE_DPS(GyroRange range)
		{
			return (range == GyroRange::RANGE_2000 ? 2000 :
					range == GyroRange::RANGE_1000 ? 1000 :
					range == GyroRange::RANGE_500 ? 500 :
					250);
		}
		
		enum class AccelRange : uint8_t
		{
			RANGE_2G	= 0 << 3,
			RANGE_4G	= 1 << 3,
			RANGE_8G	= 2 << 3,
			RANGE_16G	= 3 << 3,
		};
		
		static constexpr uint16_t ACCEL_RANGE_G(AccelRange range)
		{
			return (range == AccelRange::RANGE_16G ? 16 :
					range == AccelRange::RANGE_8G ? 8 :
					range == AccelRange::RANGE_4G ? 4 :
					2);
		}
		
		struct PowerManagement
		{
			PowerManagement(): clock_select{}, temp_disable{}, reserved{}, cycle{}, sleep{}, device_reset{}
			{
			}
			uint8_t clock_select	:3;
			uint8_t temp_disable	:1;
			uint8_t reserved		:1;
			uint8_t cycle			:1;
			uint8_t sleep			:1;
			uint8_t device_reset	:1;
		};
		enum class ClockSelect : uint8_t
		{
			INTERNAL_8MHZ		= 0,
			PLL_X_AXIS_GYRO		= 1,
			PLL_Y_AXIS_GYRO		= 2,
			PLL_Z_AXIS_GYRO		= 3,
			PLL_EXTERNAL_32KHZ	= 4,
			PLL_EXTERNAL_19MHZ	= 5,
			STOPPED				= 7
		};
		
		enum class DLPF : uint8_t
		{
			ACCEL_BW_260HZ		= 0,
			ACCEL_BW_184HZ		= 1,
			ACCEL_BW_94HZ		= 2,
			ACCEL_BW_44HZ		= 3,
			ACCEL_BW_21HZ		= 4,
			ACCEL_BW_10HZ		= 5,
			ACCEL_BW_5HZ		= 6,
			
			GYRO_BW_256HZ		= 0,
			GYRO_BW_188HZ		= 1,
			GYRO_BW_98HZ		= 2,
			GYRO_BW_42HZ		= 3,
			GYRO_BW_20HZ		= 4,
			GYRO_BW_10HZ		= 5,
			GYRO_BW_5HZ			= 6
		};
		
		struct FIFOEnable
		{
			FIFOEnable(): reserved{}, accel{}, gyro_z{}, gyro_y{}, gyro_x{}, temperature{}
			{
			}
			uint8_t reserved	:3;
			uint8_t accel		:1;
			uint8_t gyro_z		:1;
			uint8_t gyro_y		:1;
			uint8_t gyro_x		:1;
			uint8_t temperature	:1;
		};
		struct INTStatus
		{
			uint8_t data_ready	:1;
			uint8_t reserved1	:3;
			uint8_t overflow	:1;
			uint8_t reserved2	:3;
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
		
		enum class AD0 : uint8_t
		{
			LOW		= 0,
			HIGH	= 1
		};

		//TODO Add some using in code in order to use shorter LOCs
		//NOTE: MPU6050 auxiliary I2C is not supported.
		template<i2c::I2CMode MODE = i2c::I2CMode::Fast, AD0 AD0 = AD0::LOW>
		class MPU6050 : public i2c::I2CDevice<MODE>
		{
		public:
			using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

			MPU6050(MANAGER& manager): i2c::I2CDevice<MODE>(manager)
			{
			}

			bool begin( GyroRange gyro_range = GyroRange::RANGE_250,
						AccelRange accel_range = AccelRange::RANGE_2G,
						DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
						ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
			{
				PowerManagement power;
				power.clock_select = uint8_t(clock_select);
				return		this->write(DEVICE_ADDRESS, CONFIG, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(low_pass_filter), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(gyro_range), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(accel_range), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, PWR_MGMT_1, i2c::BusConditions::REPEAT_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, power, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}
			
			bool begin( FIFOEnable fifo_enable,
						INTStatus int_enable,
						uint8_t sample_rate_divider,
						GyroRange gyro_range = GyroRange::RANGE_250,
						AccelRange accel_range = AccelRange::RANGE_2G,
						DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
						ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
			{
				fifo_enable.reserved = 0;
				if (utils::as_uint8_t(fifo_enable) == 0)
					return begin(gyro_range, accel_range, low_pass_filter, clock_select);
				int_enable.reserved1 = int_enable.reserved2 = 0;
				PowerManagement power;
				power.clock_select = uint8_t(clock_select);
				return		this->write(DEVICE_ADDRESS, CONFIG, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(low_pass_filter), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(gyro_range), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(accel_range), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						
						&&	this->write(DEVICE_ADDRESS, SMPRT_DIV, i2c::BusConditions::REPEAT_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, sample_rate_divider, i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						
						&&	this->write(DEVICE_ADDRESS, FIFO_EN, i2c::BusConditions::REPEAT_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, utils::as_uint8_t(fifo_enable), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						
						&&	this->write(DEVICE_ADDRESS, INT_PIN_CFG, i2c::BusConditions::REPEAT_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(0), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, utils::as_uint8_t(int_enable), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						
						&&	this->write(DEVICE_ADDRESS, USER_CTRL, i2c::BusConditions::REPEAT_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(0x40), i2c::BusConditions::NO_START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, power, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}
			
			inline bool end() INLINE
			{
				// Put to sleep mode
				PowerManagement power;
				power.sleep = 1;
				return write_power(power);
			}
			
			inline bool reset() INLINE
			{
				PowerManagement power;
				power.device_reset = 1;
				return write_power(power);
			}

			bool gyro_measures(Sensor3D& gyro)
			{
				if (	this->write(DEVICE_ADDRESS, GYRO_XOUT, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&&	this->read(DEVICE_ADDRESS, gyro, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					format_sensors(gyro);
					return true;
				}
				else
					return false;
			}

			int16_t temperature()
			{
				int16_t temperature = -32768;
				if (this->write(DEVICE_ADDRESS, TEMP_OUT, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK)
					this->read(DEVICE_ADDRESS, temperature, i2c::BusConditions::REPEAT_START_STOP);
				return temperature;
			}
			
			static int16_t convert_temp_to_centi_degrees(int16_t temp)
			{
				// MPU-6000 Register Map datasheet §4.18 formula: Tc = TEMP / 340 + 36.53
				return int16_t(temp * 10L / 34L + 3653);
			}

			bool accel_measures(Sensor3D& accel)
			{
				if (	this->write(DEVICE_ADDRESS, ACCEL_XOUT, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&&	this->read(DEVICE_ADDRESS, accel, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					format_sensors(accel);
					return true;
				}
				else
					return false;
			}
			
			bool all_measures(AllSensors& sensors)
			{
				if (	this->write(DEVICE_ADDRESS, ACCEL_XOUT, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&&	this->read(DEVICE_ADDRESS, sensors, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					format_sensors(sensors.accel);
					format_sensors(sensors.gyro);
					utils::swap_bytes(sensors.temperature);
					return true;
				}
				else
					return false;
			}
			
			INTStatus interrupt_status()
			{
				INTStatus status;
				if (this->write(DEVICE_ADDRESS, INT_STATUS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK)
					this->read(DEVICE_ADDRESS, status, i2c::BusConditions::REPEAT_START_STOP);
				return status;
			}
			
			bool reset_fifo()
			{
				return		this->write(DEVICE_ADDRESS, USER_CTRL, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, uint8_t(0x44), i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}
			
			uint16_t fifo_count()
			{
				uint16_t count = 0;
				if (this->write(DEVICE_ADDRESS, FIFO_COUNT, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK)
					this->read(DEVICE_ADDRESS, count, i2c::BusConditions::REPEAT_START_STOP);
				utils::swap_bytes(count);
				return count;
			}
			
			template<typename T>
			inline bool fifo_pop(T& output, bool wait = false)
			{
				return fifo_pop((uint8_t*) &output, sizeof(T), wait);
			}

		private:
			static constexpr const uint8_t DEVICE_ADDRESS = (0x68 | uint8_t(AD0)) << 1;

			static constexpr const uint8_t SMPRT_DIV = 0x19;
			static constexpr const uint8_t CONFIG = 0x1A;
			static constexpr const uint8_t GYRO_CONFIG = 0x1B;
			static constexpr const uint8_t ACCEL_CONFIG = 0x1C;
			
			static constexpr const uint8_t FIFO_EN = 0x23;
			static constexpr const uint8_t INT_PIN_CFG = 0x37;
			static constexpr const uint8_t INT_ENABLE = 0x38;
			static constexpr const uint8_t INT_STATUS = 0x3A;
			
			static constexpr const uint8_t ACCEL_XOUT = 0x3B;
			static constexpr const uint8_t TEMP_OUT = 0x41;
			static constexpr const uint8_t GYRO_XOUT = 0x43;
			
			static constexpr const uint8_t USER_CTRL = 0x6A;
			static constexpr const uint8_t PWR_MGMT_1 = 0x6B;
			static constexpr const uint8_t PWR_MGMT_2 = 0x6C;
			
			static constexpr const uint8_t FIFO_COUNT = 0x72;
			static constexpr const uint8_t FIFO_R_W = 0x74;

			static constexpr const uint8_t WHO_AM_I = 0x75;

			inline bool write_power(PowerManagement power)
			{
				return		this->write(DEVICE_ADDRESS, PWR_MGMT_1, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
						&&	this->write(DEVICE_ADDRESS, power, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}
			
			void format_sensors(Sensor3D& sensors)
			{
				utils::swap_bytes(sensors.x);
				utils::swap_bytes(sensors.y);
				utils::swap_bytes(sensors.z);
			}
			
			bool fifo_pop(uint8_t* buffer, uint8_t size, bool wait)
			{
				while (fifo_count() < size)
					if (!wait) 
						return false;
					else
					{
						//TODO yield here instead?
					}
				if (	this->write(DEVICE_ADDRESS, FIFO_R_W, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
					&&	this->read(DEVICE_ADDRESS, buffer, size, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					// Swap all 2-bytes words
					uint16_t* temp = (uint16_t*) buffer;
					size /= 2;
					while (size--) utils::swap_bytes(*temp++);
					return true;
				}
				return false;
			}

		};
	}
}

#endif /* MPU6050_H */
