//   Copyright 2016-2019 Jean-Francois Poilpret
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

/// @cond api

/**
 * @file
 * API to handle HMC5883L 3-axis digital compass I2C chip.
 * @sa https://github.com/jfpoilpret/fast-arduino-lib/blob/master/refs/devices/HMC5883L.pdf
 */
#ifndef HMC5883L_H
#define HMC5883L_H

#include <math.h>
#include "common_magneto.h"
#include "../i2c_device.h"
#include "../utilities.h"

//TODO Add (or just document) optional support for DRDY pin (but it does not seem to work?)
/**
 * Defines API for magnetic sensors for direction, speed and acceleration properties.
 */
namespace devices::magneto
{
	/**
	 * Calculate the magnetic heading (heading measured clockwise from magnetic 
	 * north) from X and Y magnetic fields.
	 */
	inline float magnetic_heading(int16_t x, int16_t y)
	{
		float theta = atan2(y, x);
		return theta;
	}

	/**
	 * The number of samples to average every time a measurement is required from
	 * the HMC5883L chip (datasheet p12).
	 */
	enum class SamplesAveraged : uint8_t
	{
		ONE_SAMPLE = 0 << 5,
		TWO_SAMPLES = 1 << 5,
		FOUR_SAMPLES = 2 << 5,
		EIGHT_SAMPLES = 3 << 5
	};

	/**
	 * The output rate when used in continuous mode (datasheet p12).
	 * @sa OperatingMode
	 */
	enum class DataOutput : uint8_t
	{
		RATE_0_75HZ = 0 << 2,
		RATE_1_5HZ = 1 << 2,
		RATE_3HZ = 2 << 2,
		RATE_7_5HZ = 3 << 2,
		RATE_15HZ = 4 << 2,
		RATE_30HZ = 5 << 2,
		RATE_75HZ = 6 << 2
	};

	/**
	 * The measurement mode as defined in datasheet p12, table6.
	 */
	enum class MeasurementMode : uint8_t
	{
		NORMAL = 0,
		POSITIVE_BIAS = 1,
		NEGATIVE_BIAS = 2
	};

	/**
	 * The operating mode of the chip as defined in datasheet p10, p14 table 12.
	 */
	enum class OperatingMode : uint8_t
	{
		CONTINUOUS = 0,
		SINGLE = 1,
		IDLE = 2
	};

	/**
	 * The gain to set for the chip, as defined in datasheet p13, table9.
	 */
	enum class Gain : uint8_t
	{
		GAIN_0_88GA = 0 << 5,
		GAIN_1_3GA = 1 << 5,
		GAIN_1_9GA = 2 << 5,
		GAIN_2_5GA = 3 << 5,
		GAIN_4_0GA = 4 << 5,
		GAIN_4_7GA = 5 << 5,
		GAIN_5_6GA = 6 << 5,
		GAIN_8_1GA = 7 << 5
	};

	/**
	 * The chip status, as defined in datasheet p16.
	 */
	struct Status
	{
		Status() : ready{}, lock{}, error{1}, reserved{} {}

		uint8_t ready : 1;
		uint8_t lock : 1;
		//FIXME there is no error bit in status register, where does this bit come from?
		uint8_t error : 1;
		uint8_t reserved : 5;
	};

	/**
	 * I2C device driver for the HMC5883L compass chip.
	 * @tparam MODE_ the I2C transmission mode to use for this device; this chip
	 * supports both available modes.
	 */
	template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast> class HMC5883L : public i2c::I2CDevice<MODE_>
	{
	private:
		using BusCond = i2c::BusConditions;

	public:
		/** The I2C transmission mode (speed) used for this device. */
		static constexpr const i2c::I2CMode MODE = MODE_;

		/** The type of `i2c::I2CManager` that must be used to handle this device.  */
		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		/**
		 * Create a new device driver for a HMC5883L chip.
		 * @param manager reference to a suitable i2c::I2CManager for this device
		 */
		HMC5883L(MANAGER& manager) : i2c::I2CDevice<MODE>(manager) {}

		/**
		 * Start operation of this compass chip. Once this method has been called,
		 * you may use `magnetic_fields()` to find out the directions of the device.
		 * 
		 * @param mode the `OperatingMode` to operate this chip
		 * @param gain the `Gain` to use to increase measured magnetic fields
		 * @param rate the `DataOutput` rate to use in `OperatingMode::CONTINUOUS` mode
		 * @param samples the `SamplesAveraged` to use for each measurement
		 * @param measurement the `MeasurementMode` to use on the chip sensors
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa end()
		 */
		bool begin(OperatingMode mode = OperatingMode::SINGLE, Gain gain = Gain::GAIN_1_3GA,
				   DataOutput rate = DataOutput::RATE_15HZ, SamplesAveraged samples = SamplesAveraged::ONE_SAMPLE,
				   MeasurementMode measurement = MeasurementMode::NORMAL)
		{
			gain_ = GAIN(gain);
			return write_register(CONFIG_REG_A, uint8_t(measurement) | uint8_t(rate) | uint8_t(samples))
				   && write_register(CONFIG_REG_B, uint8_t(gain)) && write_register(MODE_REG, uint8_t(mode));
		}

		/**
		 * Stop operation of this compass chip. You should not call `magnetic_fields()`
		 * after calling this method.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa begin()
		 */
		inline bool end() INLINE
		{
			return write_register(MODE_REG, uint8_t(OperatingMode::IDLE));
		}

		/**
		 * Get the curent chip status.
		 */
		inline Status status() INLINE
		{
			Status status;
			read_register(STATUS_REG, (uint8_t&) status);
			return status;
		}

		/**
		 * Read the magnetic fields (as raw values) on 3 axes (datasheet p15-16).
		 * In order to convert raw measurements to physical values, you should
		 * call `convert_fields_to_mGA()`.
		 * 
		 * @param fields a reference to a `Sensor3D` variable that will be
		 * filled with values upon method return
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa convert_fields_to_mGA()
		 */
		bool magnetic_fields(Sensor3D& fields)
		{
			using namespace i2c::Status;
			if (this->write(DEVICE_ADDRESS, OUTPUT_REG_1, BusCond::START_NO_STOP) == OK
				&& this->read(DEVICE_ADDRESS, fields, BusCond::REPEAT_START_STOP) == OK)
			{
				utils::swap_bytes(fields.x);
				utils::swap_bytes(fields.y);
				utils::swap_bytes(fields.z);
				return true;
			}
			else
				return false;
		}

		/**
		 * Convert raw fields measured obtained with `magnetic_fields()` to actual
		 * physical values, using the `Gain` configured for the device.
		 * 
		 * @param fields a reference to a `Sensor3D` variable that will be
		 * converted from raw to physical values
		 * 
		 * @sa magnetic_fields()
		 * @sa begin()
		 */
		void convert_fields_to_mGA(Sensor3D& fields)
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
			using namespace i2c::Status;
			return (this->write(DEVICE_ADDRESS, address, BusCond::START_NO_STOP) == OK
					&& this->write(DEVICE_ADDRESS, value, BusCond::NO_START_STOP) == OK);
		}

		bool read_register(uint8_t address, uint8_t& value)
		{
			using namespace i2c::Status;
			return (this->write(DEVICE_ADDRESS, address, BusCond::START_NO_STOP) == OK
					&& this->read(DEVICE_ADDRESS, value, BusCond::REPEAT_START_STOP) == OK);
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

#endif /* HMC5883L_H */
/// @endcond
