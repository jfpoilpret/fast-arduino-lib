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
 * API to handle MPU6050 3-axis gyroscope/accelerometer I2C chip.
 * @sa https://github.com/jfpoilpret/fast-arduino-lib/blob/master/refs/devices/MPU-6000-Datasheet1.pdf
 * @sa https://github.com/jfpoilpret/fast-arduino-lib/blob/master/refs/devices/MPU-6000-Register-Map1.pdf
 */
#ifndef MPU6050_H
#define MPU6050_H

#include <math.h>
#include <stdint.h>
#include "common_magneto.h"
#include "../i2c_device.h"
#include "../time.h"
#include "../utilities.h"

//TODO rename namespace: "magneto" is not relevant here, eg "motion" or "motion_sensor"
namespace devices::magneto
{
	/**
	 * The full-scale range of the gyroscope in dps (datasheet §6.1).
	 * @sa MPU6050::begin()
	 */
	enum class GyroRange : uint8_t
	{
		RANGE_250 = 0 << 3,
		RANGE_500 = 1 << 3,
		RANGE_1000 = 2 << 3,
		RANGE_2000 = 3 << 3,
	};
	
	/**
	 * Convert a `GyroRange` constant to the real gyroscope range in dps.
	 */
	static constexpr uint16_t GYRO_RANGE_DPS(GyroRange range)
	{
		return (range == GyroRange::RANGE_2000 ? 2000 :
				range == GyroRange::RANGE_1000 ? 1000 :
				range == GyroRange::RANGE_500 ? 500 :
				250);
	}
	
	/**
	 * The full-scale range of the accelerometer in g (datasheet §6.2).
	 * @sa MPU6050::begin()
	 */
	enum class AccelRange : uint8_t
	{
		RANGE_2G = 0 << 3,
		RANGE_4G = 1 << 3,
		RANGE_8G = 2 << 3,
		RANGE_16G = 3 << 3,
	};
	
	/**
	 * Convert an `AccelRange` constant to the real accelerometer range in g.
	 */
	static constexpr uint16_t ACCEL_RANGE_G(AccelRange range)
	{
		return (range == AccelRange::RANGE_16G ? 16 :
				range == AccelRange::RANGE_8G ? 8 :
				range == AccelRange::RANGE_4G ? 4 :
				2);
	}
	
	/**
	 * The clock to select for the chip (datasheet §6.6).
	 * @sa MPU6050::begin()
	 */
	enum class ClockSelect : uint8_t
	{
		INTERNAL_8MHZ = 0,
		PLL_X_AXIS_GYRO = 1,
		PLL_Y_AXIS_GYRO = 2,
		PLL_Z_AXIS_GYRO = 3,
		PLL_EXTERNAL_32KHZ = 4,
		PLL_EXTERNAL_19MHZ = 5,
		STOPPED = 7
	};

	/**
	 * The Digital Low Pass Filter bandwidth to select for the chip
	 * (register map §4.3).
	 * This can be expressed either from the gyroscope viewpoint ot from the
	 * accelerometer viewpoint, but any setting is common to both features, i.e.
	 * selecting a DLPF setting for the accelerometer will force the matching
	 * setting for the gyroscope.
	 * @sa MPU6050::begin()
	 */
	enum class DLPF : uint8_t
	{
		ACCEL_BW_260HZ = 0,
		ACCEL_BW_184HZ = 1,
		ACCEL_BW_94HZ = 2,
		ACCEL_BW_44HZ = 3,
		ACCEL_BW_21HZ = 4,
		ACCEL_BW_10HZ = 5,
		ACCEL_BW_5HZ = 6,

		GYRO_BW_256HZ = 0,
		GYRO_BW_188HZ = 1,
		GYRO_BW_98HZ = 2,
		GYRO_BW_42HZ = 3,
		GYRO_BW_20HZ = 4,
		GYRO_BW_10HZ = 5,
		GYRO_BW_5HZ = 6
	};
	
	/**
	 * Configuration for MPU6050 FIFO Enable register (register map §4.6).
	 * This allows setting which sensor measurements should be loaded in the chip
	 * FIFO buffer (see also datasheet §7.17).
	 * @sa MPU6050::begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
	 */
	struct FIFOEnable
	{
		FIFOEnable() : reserved{}, accel{}, gyro_z{}, gyro_y{}, gyro_x{}, temperature{} {}

		uint8_t reserved : 3;
		/** If `1`, accelerometer measures on 3 axes will be loaded to FIFO buffer. */
		uint8_t accel : 1;
		/** If `1`, gyroscope measures on Z axis will be loaded to FIFO buffer. */
		uint8_t gyro_z : 1;
		/** If `1`, gyroscope measures on Y axis will be loaded to FIFO buffer. */
		uint8_t gyro_y : 1;
		/** If `1`, gyroscope measures on X axis will be loaded to FIFO buffer. */
		uint8_t gyro_x : 1;
		/** If `1`, chip temperature will be loaded to FIFO buffer. */
		uint8_t temperature : 1;
	};

	/** 
	 * The structure of the Interrupt Status register (register map §4.16).
	 * @sa MPU6050::interrupt_status()
	 * @sa INTEnable
	 */
	struct INTStatus
	{
		INTStatus() : data_ready{}, reserved1{}, overflow{}, reserved2{} {}

		/** If `1`, the Data Ready interrupt is enabled. */
		uint8_t data_ready : 1;
		uint8_t reserved1 : 3;
		/** If `1`, a FIFO buffer overflow will generate an interrupt. */
		uint8_t overflow : 1;
		uint8_t reserved2 : 3;
	};

	/**
	 * The structure of the Interrupt Enable register (register map §4.15).
	 * @sa MPU6050::begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
	 * @sa INTStatus
	 */
	using INTEnable = INTStatus;

	/**
	 * Structure to store all MPU6050 sensors data (3 axis gyroscope and 
	 * accelerometer, chip temperature).
	 * @sa MPU6050::all_measures()
	 */
	struct AllSensors
	{
		Sensor3D gyro;
		int16_t temperature;
		Sensor3D accel;
	};

	/**
	 * Possible values of I2C address lower bit for the chip (the chip may have
	 * one of two possible addresses, based on the level of pin AD0, datasheet 
	 * §6.4, §7.1).
	 * @sa MPU6050
	 */
	enum class AD0 : uint8_t
	{
		/** When `AD0` pin is low, I2C address is `0x68`. */
		LOW = 0,
		/** When `AD0` pin is high, I2C address is `0x69`. */
		HIGH = 1
	};

	/**
	 * I2C device driver for the MPU6050 gyroscope/accelerometer chip.
	 * Note that the I2C auxiliary mode of the chip is not supported by the driver.
	 * 
	 * @tparam MODE the I2C transmission mode to use for this device; this chip
	 * supports both available modes.
	 * @tparam AD0 the level of the AD0 pin, which fixes the chip address on the
	 * I2C bus 
	 * 
	 * @sa AD0
	 */
	template<i2c::I2CMode MODE = i2c::I2CMode::Fast, AD0 AD0 = AD0::LOW> class MPU6050 : public i2c::I2CDevice<MODE>
	{
	private:
		using BusCond = i2c::BusConditions;

	public:
		/** The type of `i2c::I2CManager` that must be used to handle this device.  */
		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		/**
		 * Create a new device driver for a MPU6050 chip.
		 * @param manager reference to a suitable i2c::I2CManager for this device
		 */
		explicit MPU6050(MANAGER& manager) : i2c::I2CDevice<MODE>(manager) {}

		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, you may use other methods such as `gyro_measures()` 
		 * to get sensors measurements from the device.
		 * 
		 * @param gyro_range the `GyroRange` to use for the gyroscope measurements
		 * @param accel_range the `AccelRange` to use for the accelerometer measurements
		 * @param low_pass_filter the `DLPF` bandwidth to use for operations
		 * @param clock_select the `ClockSelect` to use as the device clock source
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa end()
		 * @sa begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
		 */
		bool begin( GyroRange gyro_range = GyroRange::RANGE_250,
					AccelRange accel_range = AccelRange::RANGE_2G,
					DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
					ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
		{
			using namespace i2c::Status;
			PowerManagement power;
			power.clock_select = uint8_t(clock_select);
			return this->write(DEVICE_ADDRESS, CONFIG, BusCond::START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(low_pass_filter), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(gyro_range), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(accel_range), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, PWR_MGMT_1, BusCond::REPEAT_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, power, BusCond::NO_START_STOP) == OK;
		}

		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, you may use other methods such as `gyro_measures()` 
		 * to get sensors measurements from the device.
		 * This shall be used when you want continuous measurements performed by 
		 * the device.
		 * 
		 * @param fifo_enable the `FIFOEnable` settings for continuous measurements
		 * @param int_enable the `INTEnable` settings for interrupt generation; note 
		 * that the device driver does not handle interrupts (ISR) itself, you need
		 * to use other FastArduino API for that.
		 * @param sample_rate_divider the divider from the gyroscope output rate
		 * to generate the Sample Rate of the chip (register map §4.2)
		 * @param gyro_range the `GyroRange` to use for the gyroscope measurements
		 * @param accel_range the `AccelRange` to use for the accelerometer measurements
		 * @param low_pass_filter the `DLPF` bandwidth to use for operations
		 * @param clock_select the `ClockSelect` to use as the device clock source
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa end()
		 * @sa begin(GyroRange, AccelRange, DLPF, ClockSelect)
		 * @sa interrupts::INTSignal
		 * @sa interrupts::PCISignal
		 */
		bool begin( FIFOEnable fifo_enable,
					INTEnable int_enable,
					uint8_t sample_rate_divider,
					GyroRange gyro_range = GyroRange::RANGE_250,
					AccelRange accel_range = AccelRange::RANGE_2G,
					DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
					ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
		{
			using namespace i2c::Status;
			fifo_enable.reserved = 0;
			if (utils::as_uint8_t(fifo_enable) == 0)
				return begin(gyro_range, accel_range, low_pass_filter, clock_select);
			int_enable.reserved1 = int_enable.reserved2 = 0;
			PowerManagement power;
			power.clock_select = uint8_t(clock_select);
			return this->write(DEVICE_ADDRESS, CONFIG, BusCond::START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(low_pass_filter), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(gyro_range), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(accel_range), BusCond::NO_START_NO_STOP) == OK

				   && this->write(DEVICE_ADDRESS, SMPRT_DIV, BusCond::REPEAT_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, sample_rate_divider, BusCond::NO_START_NO_STOP) == OK

				   && this->write(DEVICE_ADDRESS, FIFO_EN, BusCond::REPEAT_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, utils::as_uint8_t(fifo_enable), BusCond::NO_START_NO_STOP) == OK

				   && this->write(DEVICE_ADDRESS, INT_PIN_CFG, BusCond::REPEAT_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(0), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, utils::as_uint8_t(int_enable), BusCond::NO_START_NO_STOP) == OK

				   && this->write(DEVICE_ADDRESS, USER_CTRL, BusCond::REPEAT_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(0x40), BusCond::NO_START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, power, BusCond::NO_START_STOP) == OK;
		}

		/**
		 * Put the chip to sleep mode (low-power mode); stops sampling operations 
		 * if any.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa begin()
		 */
		inline bool end() INLINE
		{
			// Put to sleep mode
			PowerManagement power;
			power.sleep = 1;
			return write_power(power);
		}

		/**
		 * Reset the chip (register map §4.28).
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		inline bool reset() INLINE
		{
			PowerManagement power;
			power.device_reset = 1;
			return write_power(power);
		}

		/**
		 * Get latest gyroscope measurements from the device (register map §4.19).
		 * @param gyro a reference to a `Sensor3D` variable that will be filled
		 * with latest gyroscope measurements on 3 axis.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool gyro_measures(Sensor3D& gyro)
		{
			using namespace i2c::Status;
			if (this->write(DEVICE_ADDRESS, GYRO_XOUT, BusCond::START_NO_STOP) == OK
				&& this->read(DEVICE_ADDRESS, gyro, BusCond::REPEAT_START_STOP) == OK)
			{
				format_sensors(gyro);
				return true;
			}
			else
				return false;
		}

		/**
		 * Get latest chip temperature measurement (register map §4.18).
		 * The returned value is nternal raw value from the chip, it can be 
		 * converted to human-readable temperature with `convert_temp_to_centi_degrees()`.
		 * 
		 * @return the latest raw temperature in degrees if no error occurred
		 * @retval -32768 if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa convert_temp_to_centi_degrees()
		 */
		int16_t temperature()
		{
			using namespace i2c::Status;
			int16_t temperature = -32768;
			if (this->write(DEVICE_ADDRESS, TEMP_OUT, BusCond::START_NO_STOP) == OK)
				this->read(DEVICE_ADDRESS, temperature, BusCond::REPEAT_START_STOP);
			return temperature;
		}

		/**
		 * Convert the raw temperature obtained from `temperature()` to 
		 * centi-degrees Celsius.
		 */
		static constexpr int16_t convert_temp_to_centi_degrees(int16_t temp)
		{
			// MPU-6000 Register Map datasheet §4.18 formula: Tc = TEMP / 340 + 36.53
			return int16_t(temp * 10L / 34L + 3653);
		}

		/**
		 * Get latest accelerometer measurements from the device (register map §4.17).
		 * @param accel a reference to a `Sensor3D` variable that will be filled
		 * with latest accelerometer measurements on 3 axis.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool accel_measures(Sensor3D& accel)
		{
			using namespace i2c::Status;
			if (this->write(DEVICE_ADDRESS, ACCEL_XOUT, BusCond::START_NO_STOP) == OK
				&& this->read(DEVICE_ADDRESS, accel, BusCond::REPEAT_START_STOP) == OK)
			{
				format_sensors(accel);
				return true;
			}
			else
				return false;
		}

		/**
		 * Get latest measurements of all device sensors (gyroscope, accelerometer,
		 * temperature).
		 * @param sensors a reference to an `AllSensors` variable that will be filled
		 * with all latest measurements.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool all_measures(AllSensors& sensors)
		{
			using namespace i2c::Status;
			if (this->write(DEVICE_ADDRESS, ACCEL_XOUT, BusCond::START_NO_STOP) == OK
				&& this->read(DEVICE_ADDRESS, sensors, BusCond::REPEAT_START_STOP) == OK)
			{
				format_sensors(sensors.accel);
				format_sensors(sensors.gyro);
				utils::swap_bytes(sensors.temperature);
				return true;
			}
			else
				return false;
		}

		/**
		 * Get the interrupt status (register map §4.16) after an interrupt has 
		 * occurred. After this method is called, the Interrupt Status register
		 * is cleared.
		 * @return the latest interrupt status as an `INTStatus` structure where
		 * each field maps to the interrupt that occurred; in case of an error,
		 * the returned status is fully cleared. In order to ensure the returned
		 * status can be inspected, you should first call `i2c::I2CManager.status()`.
		 */
		INTStatus interrupt_status()
		{
			using namespace i2c::Status;
			INTStatus status;
			if (this->write(DEVICE_ADDRESS, INT_STATUS, BusCond::START_NO_STOP) == OK)
				this->read(DEVICE_ADDRESS, status, BusCond::REPEAT_START_STOP);
			return status;
		}

		/**
		 * Reset the FIFO buffer (parameter map §4.27).
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool reset_fifo()
		{
			using namespace i2c::Status;
			return this->write(DEVICE_ADDRESS, USER_CTRL, BusCond::START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, uint8_t(0x44), BusCond::NO_START_STOP) == OK;
		}

		/**
		 * Get the number of bytes currently stored in the FIFO buffer (register 
		 * map §4.30). This number is a multiple of the size of sensor samples as
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @return the number of bytes currently present in the FIFO buffer; when not `0`,
		 * you should read the samples with `fifo_pop()`.
		 * @retval 0 if the FIFO buffer is empty or if the operation failed; 
		 * in order to ensure the returned count is cactually `0`, you should first
		 * call `i2c::I2CManager.status()`.
		 * 
		 * @sa fifo_pop()
		 */
		uint16_t fifo_count()
		{
			using namespace i2c::Status;
			uint16_t count = 0;
			if (this->write(DEVICE_ADDRESS, FIFO_COUNT, BusCond::START_NO_STOP) == OK)
				this->read(DEVICE_ADDRESS, count, BusCond::REPEAT_START_STOP);
			utils::swap_bytes(count);
			return count;
		}

		/**
		 * Get one sample out of the FIFO buffer (register map §4.31).
		 * This method may block until a full sample is available in the FIFO
		 * buffer; if you do not want to wait, first call `fifo_count()` to ensure
		 * a sample is available.
		 * 
		 * @tparam T the type of sample to get from the FIFO buffer; must be one
		 * of `Sensor3D`, `int16_t` or `AllSensors`, based on the sensor samples
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @param output a reference to a `T`-type variable that will be filled
		 * with the required sample.
		 * @param wait set to `true` if the method shall block until a sample of 
		 * the required size is available in the FIFO buffer
		 * @param yield set to `true` if you want the method to yield time (i.e. 
		 * enter default pwoer sleep mode) while waiting; this is effective only 
		 * when @p wait is `true`. When @p wait is `true` and @p yield is `false`,
		 * then waiting is performed by a busy loop.
		 * @retval true if a sample has been read into @p output
		 * @retval false if no sample was ready or if the operation failed; when
		 * so, `i2c::I2CManager.status()` shall be called for further information
		 * on the error.
		 * 
		 * @sa fifo_count()
		 */
		template<typename T> inline bool fifo_pop(T& output, bool wait = false, bool yield = false)
		{
			return fifo_pop((uint8_t*) &output, sizeof(T), wait, yield);
		}

	private:
		struct PowerManagement
		{
			PowerManagement() : clock_select{}, temp_disable{}, reserved{}, cycle{}, sleep{}, device_reset{} {}

			uint8_t clock_select : 3;
			uint8_t temp_disable : 1;
			uint8_t reserved : 1;
			uint8_t cycle : 1;
			uint8_t sleep : 1;
			uint8_t device_reset : 1;
		};

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
			using namespace i2c::Status;
			return this->write(DEVICE_ADDRESS, PWR_MGMT_1, BusCond::START_NO_STOP) == OK
				   && this->write(DEVICE_ADDRESS, power, BusCond::NO_START_STOP) == OK;
		}

		void format_sensors(Sensor3D& sensors)
		{
			utils::swap_bytes(sensors.x);
			utils::swap_bytes(sensors.y);
			utils::swap_bytes(sensors.z);
		}

		bool fifo_pop(uint8_t* buffer, uint8_t size, bool wait, bool yield)
		{
			using namespace i2c::Status;
			while (fifo_count() < size)
			{
				if (!wait) return false;
				if (yield) time::yield();
			}

			if (this->write(DEVICE_ADDRESS, FIFO_R_W, BusCond::START_NO_STOP) == OK
				&& this->read(DEVICE_ADDRESS, buffer, size, BusCond::REPEAT_START_STOP) == OK)
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

#endif /* MPU6050_H */
/// @endcond
