//   Copyright 2016-2018 Jean-Francois Poilpret
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

#ifndef HMC5883L_H
#define HMC5883L_H

#include <math.h>
#include "../i2c_device.h"
#include "../utilities.h"

//TODO Add optional support for DRDY pin (but it does not seem to work?)
//TODO Check and optimize source code size
namespace devices
{
	namespace magneto
	{
		struct MagneticFields
		{
			int16_t x;
			int16_t z;
			int16_t y;
		};

		inline float magnetic_heading(int16_t x, int16_t y)
		{
			float theta = atan2(y, x);
			return theta;
		}

		enum class SamplesAveraged : uint8_t
		{
			ONE_SAMPLE		= 0 << 5,
			TWO_SAMPLES		= 1 << 5,
			FOUR_SAMPLES	= 2 << 5,
			EIGHT_SAMPLES	= 3 << 5
		};

		enum class DataOutput : uint8_t
		{
			RATE_0_75HZ		= 0 << 2,
			RATE_1_5HZ		= 1 << 2,
			RATE_3HZ		= 2 << 2,
			RATE_7_5HZ		= 3 << 2,
			RATE_15HZ		= 4 << 2,
			RATE_30HZ		= 5 << 2,
			RATE_75HZ		= 6 << 2
		};

		enum class MeasurementMode : uint8_t
		{
			NORMAL			= 0,
			POSITIVE_BIAS	= 1,
			NEGATIVE_BIAS	= 2
		};

		enum class OperatingMode : uint8_t
		{
			CONTINUOUS	= 0,
			SINGLE		= 1,
			IDLE		= 2
		};

		enum class Gain : uint8_t
		{
			GAIN_0_88GA	= 0 << 5,
			GAIN_1_3GA	= 1 << 5,
			GAIN_1_9GA	= 2 << 5,
			GAIN_2_5GA	= 3 << 5,
			GAIN_4_0GA	= 4 << 5,
			GAIN_4_7GA	= 5 << 5,
			GAIN_5_6GA	= 6 << 5,
			GAIN_8_1GA	= 7 << 5
		};

		struct Status
		{
			Status() : ready{}, lock{}, error{1}, reserved{}
			{
			}

			uint8_t ready		: 1;
			uint8_t lock		: 1;
			uint8_t error		: 1;
			uint8_t reserved	: 5;
		};

		template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast>
		class HMC5883L : public i2c::I2CDevice<MODE_>
		{
		public:
			static constexpr const i2c::I2CMode MODE = MODE_;

			using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

			HMC5883L(MANAGER& manager) : i2c::I2CDevice<MODE>(manager)
			{
			}

			bool begin(
				OperatingMode mode = OperatingMode::SINGLE,
				Gain gain = Gain::GAIN_1_3GA,
				DataOutput rate = DataOutput::RATE_15HZ, 
				SamplesAveraged samples = SamplesAveraged::ONE_SAMPLE, 
				MeasurementMode measurement = MeasurementMode::NORMAL)
			{
				gain_ = GAIN(gain);
				return	write_register(CONFIG_REG_A, uint8_t(measurement) | uint8_t(rate) | uint8_t(samples)) &&
						write_register(CONFIG_REG_B, uint8_t(gain)) && write_register(MODE_REG, uint8_t(mode));
			}

			inline bool end() INLINE
			{
				return write_register(MODE_REG, uint8_t(OperatingMode::IDLE));
			}

			inline Status status() INLINE
			{
				Status status;
				read_register(STATUS_REG, (uint8_t&) status);
				return status;
			}

			bool magnetic_fields(MagneticFields& fields)
			{
				if (this->write(DEVICE_ADDRESS, OUTPUT_REG_1, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					this->read(DEVICE_ADDRESS, fields, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					utils::swap_bytes(fields.x);
					utils::swap_bytes(fields.y);
					utils::swap_bytes(fields.z);
					return true;
				}
				else
					return false;
			}

			void convert_fields_to_mGA(MagneticFields& fields)
			{
				convert_field_to_mGa(fields.x);
				convert_field_to_mGa(fields.y);
				convert_field_to_mGa(fields.z);
			}

		private:
			static constexpr const uint8_t DEVICE_ADDRESS = 0x1E << 1;

			static constexpr const uint8_t CONFIG_REG_A = 0;
			static constexpr const uint8_t CONFIG_REG_B = 1;
			static constexpr const uint8_t MODE_REG = 2;
			static constexpr const uint8_t OUTPUT_REG_1 = 3;
			static constexpr const uint8_t STATUS_REG = 9;
			static constexpr const uint8_t IDENT_REG_A = 10;
			static constexpr const uint8_t IDENT_REG_B = 11;
			static constexpr const uint8_t IDENT_REG_C = 12;

			bool write_register(uint8_t address, uint8_t value)
			{
				return (this->write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
						this->write(DEVICE_ADDRESS, value, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK);
			}

			bool read_register(uint8_t address, uint8_t& value)
			{
				return (this->write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
						this->read(DEVICE_ADDRESS, value, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK);
			}

			void convert_field_to_mGa(int16_t& value)
			{
				value = value * 1000L / gain_;
			}

			static constexpr uint16_t GAIN(Gain gain)
			{
				return (gain == Gain::GAIN_0_88GA ? 1370 :
						gain == Gain::GAIN_1_3GA ? 1090 :
						gain == Gain::GAIN_1_9GA ? 820 :
						gain == Gain::GAIN_2_5GA ? 660 :
						gain == Gain::GAIN_4_0GA ? 440 :
						gain == Gain::GAIN_4_7GA ? 390 :
						gain == Gain::GAIN_5_6GA ? 330 : 230);
			}

			uint16_t gain_;
		};
	}
}

#endif /* HMC5883L_H */
