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
#include "../array.h"
#include "../new_i2c_device.h"
#include "../utilities.h"

//TODO rename namespace: "magneto" is not relevant here, eg "motion" or "motion_sensor"
namespace devices::magneto
{
	/**
	 * The full-scale range of the gyroscope in dps (datasheet §6.1).
	 * @sa MPU6050::begin()
	 * @sa MPU6050::BeginFuture
	 * @sa MPU6050::FifoBeginFuture
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
		if (range == GyroRange::RANGE_2000) return 2000;
		if (range == GyroRange::RANGE_1000) return 1000;
		if (range == GyroRange::RANGE_500) return 500;
		return 250;
	}
	
	/**
	 * The full-scale range of the accelerometer in g (datasheet §6.2).
	 * @sa MPU6050::begin()
	 * @sa MPU6050::BeginFuture
	 * @sa MPU6050::FifoBeginFuture
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
		if (range == AccelRange::RANGE_16G) return 16;
		if (range == AccelRange::RANGE_8G) return 8;
		if (range == AccelRange::RANGE_4G) return 4;
		return 2;
	}
	
	/**
	 * The clock to select for the chip (datasheet §6.6).
	 * @sa MPU6050::begin()
	 * @sa MPU6050::BeginFuture
	 * @sa MPU6050::FifoBeginFuture
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
	 * @sa MPU6050::BeginFuture
	 * @sa MPU6050::FifoBeginFuture
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
	 * @sa MPU6050::FifoBeginFuture
	 */
	class FIFOEnable
	{
	public:
		/**
		 * Create a new FIFOEnable configuration value.
		 * @param accel `true` if you want FIFO enabled for accelerometer sensor measures
		 * @param gyro_x `true` if you want FIFO enabled for X-axis gyroscope sensor measures
		 * @param gyro_y `true` if you want FIFO enabled for Y-axis gyroscope sensor measures
		 * @param gyro_z `true` if you want FIFO enabled for Z-axis gyroscope sensor measures
		 * @param temperature `true` if you want FIFO enabled for temperature sensor measures
		 */
		constexpr FIFOEnable(
			bool accel = false, bool gyro_x = false, bool gyro_y = false, bool gyro_z = false, bool temperature = false)
			:	reserved_{}, 
				accel_{accel}, gyro_z_{gyro_z}, gyro_y_{gyro_y}, gyro_x_{gyro_x}, 
				temperature_{temperature} {}

		/** If `true`, accelerometer measures on 3 axes will be loaded to FIFO buffer. */
		bool accel() const
		{
			return accel_;
		}

		/** If `true`, gyroscope measures on X axis will be loaded to FIFO buffer. */
		bool gyro_x() const
		{
			return gyro_x_;
		}

		/** If `true`, gyroscope measures on Y axis will be loaded to FIFO buffer. */
		bool gyro_y() const
		{
			return gyro_y_;
		}

		/** If `true`, gyroscope measures on Z axis will be loaded to FIFO buffer. */
		bool gyro_z() const
		{
			return gyro_z_;
		}

		/** If `true`, chip temperature will be loaded to FIFO buffer. */
		bool temperature() const
		{
			return temperature_;			
		}

	private:
		uint8_t reserved_ : 3;
		bool accel_ : 1;
		bool gyro_z_ : 1;
		bool gyro_y_ : 1;
		bool gyro_x_ : 1;
		bool temperature_ : 1;
	};

	/** 
	 * The structure of the Interrupt Status register (register map §4.16).
	 * 
	 * Note that this structure is also used as `INTEnable` type in order to 
	 * enable or disable interrupts.
	 * @sa MPU6050::interrupt_status()
	 * @sa INTEnable
	 */
	class INTStatus
	{
	public:
		/**
		 * Create a new INTStatus configuration value.
		 * @param data_ready `true` to enable Data Ready interrupt
		 * @param overflow `true` to enable FIFO buffer overflow interrupt
		 */
		constexpr INTStatus(bool data_ready = false, bool overflow = false)
			: data_ready_{data_ready}, reserved1_{}, overflow_{overflow}, reserved2_{} {}

		/** If `true`, the Data Ready interrupt is enabled. */
		bool data_ready() const
		{
			return data_ready_;
		}

		/** If `true`, a FIFO buffer overflow will generate an interrupt. */
		bool overflow() const
		{
			return overflow_;
		}

	private:
		bool data_ready_ : 1;
		uint8_t reserved1_ : 3;
		bool overflow_ : 1;
		uint8_t reserved2_ : 3;
	};

	/**
	 * The structure of the Interrupt Enable register (register map §4.15).
	 * @sa MPU6050::begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
	 * @sa MPU6050::FifoBeginFuture
	 * @sa INTStatus
	 */
	using INTEnable = INTStatus;

	/**
	 * Structure to store all MPU6050 sensors data (3 axis gyroscope and 
	 * accelerometer, chip temperature).
	 * @sa MPU6050::all_measures()
	 * @sa MPU6050::AllMeasuresFuture
	 */
	struct AllSensors
	{
		Sensor3D accel;
		int16_t temperature;
		Sensor3D gyro;
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
	 * @tparam MANAGER one of FastArduino available I2CManager
	 */
	template<typename MANAGER>
	class MPU6050 : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		// Forward declarations needed by compiler
		class Sensor3DFuture;
		class PowerManagement;
		class PowerManagementFuture;

	public:
		/**
		 * Create a new device driver for a MPU6050 chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 * @param ad0 the level of the AD0 pin, which fixes the chip address on the
		 * I2C bus 
		 * 
		 * @sa AD0
		 */
		explicit MPU6050(MANAGER& manager, AD0 ad0 = AD0::LOW)
			: PARENT(manager, DEVICE_ADDRESS(ad0), i2c::I2C_FAST) {}

		// Asynchronous API
		//==================
		/**
		 * Create a future to be used by asynchronous method begin(BeginFuture&).
		 * This is used by `begin()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished, hence
		 * when you may use other methods such as `gyro_measures()` to get sensors
		 * measurements from the device.
		 * 
		 * @param gyro_range the `GyroRange` to use for the gyroscope measurements
		 * @param accel_range the `AccelRange` to use for the accelerometer measurements
		 * @param low_pass_filter the `DLPF` bandwidth to use for operations
		 * @param clock_select the `ClockSelect` to use as the device clock source
		 * 
		 * @sa begin(BeginFuture&)
		 */
		class BeginFuture : public FUTURE<void, containers::array<uint8_t, 6>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 6>>;
		public:
			explicit BeginFuture(	GyroRange gyro_range = GyroRange::RANGE_250,
									AccelRange accel_range = AccelRange::RANGE_2G,
									DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
									ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
				: PARENT{{	CONFIG, uint8_t(low_pass_filter), uint8_t(gyro_range), uint8_t(accel_range),
							PWR_MGMT_1, utils::as_uint8_t(PowerManagement{clock_select})}} {}
			BeginFuture(BeginFuture&&) = default;
			BeginFuture& operator=(BeginFuture&&) = default;
		};

		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, and @p future is ready, you may use other methods such
		 * as `gyro_measures()` to get sensors measurements from the device.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `BeginFuture` passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa BeginFuture
		 * @sa end()
		 * @sa begin(GyroRange, AccelRange, DLPF, ClockSelect)
		 * @sa begin(FifoBeginFuture&)
		 * @sa errors
		 */
		int begin(PROXY<BeginFuture> future)
		{
			// We split the transaction in 2 write commands (3 bytes starting at CONFIG, 1 byte at PWR_MGT_1)
			return this->launch_commands(future, {this->write(4), this->write(2)});
		}

		/**
		 * Create a future to be used by asynchronous method begin(FifoBeginFuture&).
		 * This is used by `begin()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished, hence
		 * when you may use other methods such as `gyro_measures()` to get sensors
		 * measurements from the device.
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
		 * 
		 * @sa begin(FifoBeginFuture&)
		 */
		class FifoBeginFuture : public FUTURE<void, containers::array<uint8_t, 14>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 14>>;
		public:
			explicit FifoBeginFuture(	FIFOEnable fifo_enable,
										INTEnable int_enable,
										uint8_t sample_rate_divider,
										GyroRange gyro_range = GyroRange::RANGE_250,
										AccelRange accel_range = AccelRange::RANGE_2G,
										DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
										ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
				: PARENT{{	CONFIG, uint8_t(low_pass_filter), uint8_t(gyro_range), uint8_t(accel_range),
							PWR_MGMT_1, utils::as_uint8_t(PowerManagement{clock_select}),
							SMPRT_DIV, sample_rate_divider,
							FIFO_EN, utils::as_uint8_t(fifo_enable),
							INT_PIN_CFG, 0, utils::as_uint8_t(int_enable),
							USER_CTRL, FIFO_ENABLE}} {}
			FifoBeginFuture(FifoBeginFuture&&) = default;
			FifoBeginFuture& operator=(FifoBeginFuture&&) = default;

			/// @cond notdocumented
			bool is_fifo_enabled() const
			{
				return (this->get_input()[9] != 0);
			}
			/// @endcond
		};

		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, and @p future is ready, you may use other methods such
		 * as `gyro_measures()` to get sensors measurements from the device.
		 * This shall be used when you want continuous measurements performed by 
		 * the device.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `FifoBeginFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa FifoBeginFuture
		 * @sa end()
		 * @sa begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
		 * @sa begin(BeginFuture&)
		 * @sa errors
		 */
		int begin(PROXY<FifoBeginFuture> future)
		{
			if (!PARENT::resolve(future).is_fifo_enabled()) return errors::EINVAL;
			return this->launch_commands(future, {
				this->write(4), this->write(2), this->write(2), this->write(3), this->write(3)});
		}

		/**
		 * Create a future to be used by asynchronous method end(EndFuture&).
		 * This is used by `end()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa end(EndFuture&)
		 */
		class EndFuture : public PowerManagementFuture
		{
		public:
			EndFuture() : PowerManagementFuture{PowerManagement{false, false, true, false}} {}
			EndFuture(EndFuture&&) = default;
			EndFuture& operator=(EndFuture&&) = default;
		};

		/**
		 * Put the chip to sleep mode (low-power mode); stops sampling operations 
		 * if any.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `EndFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa begin(BeginFutre&)
		 * @sa begin(FifoBeginFuture&)
		 * @sa end()
		 * @sa errors
		 */
		int end(PROXY<EndFuture> future) INLINE
		{
			// Put to sleep mode
			return write_power(future);
		}

		/**
		 * Create a future to be used by asynchronous method reset(ResetFuture&).
		 * This is used by `reset()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa reset(ResetFuture&)
		 */
		class ResetFuture : public PowerManagementFuture
		{
		public:
			ResetFuture() : PowerManagementFuture{PowerManagement{false, false, false, true}} {}
			ResetFuture(ResetFuture&&) = default;
			ResetFuture& operator=(ResetFuture&&) = default;
		};

		/**
		 * Reset the chip (register map §4.28).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `ResetFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa reset()
		 * @sa errors
		 */
		int reset(PROXY<ResetFuture> future) INLINE
		{
			return write_power(future);
		}

		/**
		 * Create a future to be used by asynchronous method gyro_measures(GyroFuture&).
		 * This is used by `gyro_measures()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa gyro_measures(GyroFuture&)
		 */
		class GyroFuture : public Sensor3DFuture
		{
		public:
			GyroFuture() : Sensor3DFuture{GYRO_XOUT} {}
			GyroFuture(GyroFuture&&) = default;
			GyroFuture& operator=(GyroFuture&&) = default;
		};

		/**
		 * Get latest gyroscope measurements from the device (register map §4.19).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `GyroFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa gyro_measures(Sensor3D&)
		 * @sa errors
		 */
		int gyro_measures(PROXY<GyroFuture> future)
		{
			return this->launch_commands(future, this->write(), this->read());
		}

		/**
		 * Create a future to be used by asynchronous method temperature(TemperatureFuture&).
		 * This is used by `temperature()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * The value returned by `get()` is internal raw value from the chip, it can be 
		 * converted to human-readable temperature with `convert_temp_to_centi_degrees()`.
		 * 
		 * @sa temperature(TemperatureFuture&)
		 */
		class TemperatureFuture : public FUTURE<int16_t, uint8_t>
		{
			using PARENT = FUTURE<int16_t, uint8_t>;
		public:
			TemperatureFuture() : PARENT{TEMP_OUT} {}
			TemperatureFuture(TemperatureFuture&&) = default;
			TemperatureFuture& operator=(TemperatureFuture&&) = default;

			bool get(int16_t& result)
			{
				if (!PARENT::get(result)) return false;
				utils::swap_bytes(result);
				return true;
			}
		};

		/**
		 * Get latest chip temperature measurement (register map §4.18).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `TemperatureFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa TemperatureFuture
		 * @sa temperature()
		 * @sa convert_temp_to_centi_degrees()
		 * @sa errors
		 */
		int temperature(PROXY<TemperatureFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
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
		 * Create a future to be used by asynchronous method accel_measures(AccelFuture&).
		 * This is used by `accel_measures()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa accel_measures(AccelFuture&)
		 */
		class AccelFuture : public Sensor3DFuture
		{
		public:
			AccelFuture() : Sensor3DFuture{ACCEL_XOUT} {}
			AccelFuture(AccelFuture&&) = default;
			AccelFuture& operator=(AccelFuture&&) = default;
		};

		/**
		 * Get latest accelerometer measurements from the device (register map §4.17).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `AccelFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa accel_measures(Sensor3D&)
		 * @sa errors
		 */
		int accel_measures(PROXY<AccelFuture> future)
		{
			return this->launch_commands(future, this->write(), this->read());
		}

		/**
		 * Create a future to be used by asynchronous method all_measures(AllMeasuresFuture&).
		 * This is used by `all_measures()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa all_measures(AllMeasuresFuture&)
		 */
		class AllMeasuresFuture : public FUTURE<AllSensors, uint8_t>
		{
			using PARENT = FUTURE<AllSensors, uint8_t>;
		public:
			AllMeasuresFuture() : PARENT{ACCEL_XOUT} {}
			AllMeasuresFuture(AllMeasuresFuture&&) = default;
			AllMeasuresFuture& operator=(AllMeasuresFuture&&) = default;

			bool get(AllSensors& result)
			{
				if (!PARENT::get(result)) return false;
				format_sensors(result.accel);
				format_sensors(result.gyro);
				utils::swap_bytes(result.temperature);
				return true;
			}
		};

		/**
		 * Get latest measurements of all device sensors (gyroscope, accelerometer,
		 * temperature).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `AllMeasuresFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa all_measures(AllSensors&)
		 * @sa errors
		 */
		int all_measures(PROXY<AllMeasuresFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method interrupt_status(InterruptStatusFuture&).
		 * This is used by `interrupt_status()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa interrupt_status(InterruptStatusFuture&)
		 */
		class InterruptStatusFuture : public FUTURE<INTStatus, uint8_t>
		{
			using PARENT = FUTURE<INTStatus, uint8_t>;
		public:
			InterruptStatusFuture() : PARENT{INT_STATUS} {}
			InterruptStatusFuture(InterruptStatusFuture&&) = default;
			InterruptStatusFuture& operator=(InterruptStatusFuture&&) = default;
		};

		/**
		 * Get the interrupt status (register map §4.16) after an interrupt has 
		 * occurred. After this method is called, the Interrupt Status register
		 * is cleared.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `InterruptStatusFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa interrupt_status()
		 * @sa errors
		 */
		int interrupt_status(PROXY<InterruptStatusFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method reset_fifo(ResetFifoFuture&).
		 * This is used by `reset_fifo()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa reset_fifo(ResetFifoFuture&)
		 */
		class ResetFifoFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		public:
			ResetFifoFuture() : PARENT{{USER_CTRL, FIFO_ENABLE | FIFO_RESET}} {}
			ResetFifoFuture(ResetFifoFuture&&) = default;
			ResetFifoFuture& operator=(ResetFifoFuture&&) = default;
		};

		/**
		 * Reset the FIFO buffer (parameter map §4.27).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `ResetFifoFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa reset_fifo()
		 * @sa errors
		 */
		int reset_fifo(PROXY<ResetFifoFuture> future)
		{
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method fifo_count(FifoCountFuture&).
		 * This is used by `fifo_count()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa fifo_count(FifoCountFuture&)
		 */
		class FifoCountFuture : public FUTURE<uint16_t, uint8_t>
		{
			using PARENT = FUTURE<uint16_t, uint8_t>;
		public:
			FifoCountFuture() : PARENT{FIFO_COUNT} {}
			FifoCountFuture(FifoCountFuture&&) = default;
			FifoCountFuture& operator=(FifoCountFuture&&) = default;

			bool get(uint16_t& result)
			{
				if (!PARENT::get(result)) return false;
				utils::swap_bytes(result);
				return true;
			}
		};

		/**
		 * Get the number of bytes currently stored in the FIFO buffer (register 
		 * map §4.30). This number is a multiple of the size of sensor samples as
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `FifoCountFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa fifo_pop()
		 * @sa fifo_count()
		 * @sa errors
		 */
		int fifo_count(PROXY<FifoCountFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method fifo_pop(FifoPopFuture<T>&).
		 * This is used by `fifo_pop(FifoPopFuture<T>&)` to asynchronously launch the 
		 * I2C transaction, and it shall be used by the caller to determine when the
		 * I2C transaction is finished.
		 * 
		 * @tparam T the type of measurement to read from FIFO; shall be one of
		 * `Sensor3D` (gyroscope or accelerometer measure), `int16_t` (temperature),
		 * `AllSensors` (eveything), or a combination of these.
		 * 
		 * @sa fifo_pop(FifoPopFuture<T>&)
		 * @sa Sensor3D
		 * @sa AllSensors
		 */
		template<typename T>
		class FifoPopFuture : public FUTURE<T, uint8_t>
		{
			using PARENT = FUTURE<T, uint8_t>;
		public:
			FifoPopFuture() : PARENT{FIFO_R_W} {}
			FifoPopFuture(FifoPopFuture&&) = default;
			FifoPopFuture& operator=(FifoPopFuture&&) = default;

			bool get(T& result)
			{
				if (!PARENT::get(result)) return false;
				// We need to swap bytes of all words (big-endian to little-endian)
				uint16_t* temp = reinterpret_cast<uint16_t*>(&result);
				uint8_t size = sizeof(T) / 2;
				while (size--) utils::swap_bytes(*temp++);
				return true;
			}
		};

		/**
		 * Get one sample out of the FIFO buffer (register map §4.31).
		 * @warning You should first call `fifo_count()` to ensure the MPU6050
		 * FIFO queue contains a sample of the right size! Otherwise this method
		 * will not return any error but set arbitrary values to @p putput !
		 * @warning Asynchronous API!
		 * 
		 * @tparam T the type of sample to get from the FIFO buffer; must be one
		 * of `Sensor3D`, `int16_t` or `AllSensors`, based on the sensor samples
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @param future an `FifoCountFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2CManager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2CManager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa fifo_count()
		 * @sa fifo_pop(T&)
		 * @sa errors
		 */
		template<typename T> int fifo_pop(PROXY<FifoPopFuture<T>> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		// Synchronous API
		//=================
		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, you may use other methods such as `gyro_measures()` 
		 * to get sensors measurements from the device.
		 * @warning Blocking API!
		 * 
		 * @param gyro_range the `GyroRange` to use for the gyroscope measurements
		 * @param accel_range the `AccelRange` to use for the accelerometer measurements
		 * @param low_pass_filter the `DLPF` bandwidth to use for operations
		 * @param clock_select the `ClockSelect` to use as the device clock source
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa end()
		 * @sa begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)
		 * @sa begin(BeginFuture&)
		 */
		bool begin( GyroRange gyro_range = GyroRange::RANGE_250,
					AccelRange accel_range = AccelRange::RANGE_2G,
					DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
					ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
		{
			BeginFuture future{gyro_range, accel_range, low_pass_filter, clock_select};
			if (begin(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Start operation of this gyroscope/accelerometer chip. Once this method
		 * has been called, you may use other methods such as `gyro_measures()` 
		 * to get sensors measurements from the device.
		 * This shall be used when you want continuous measurements performed by 
		 * the device.
		 * @warning Blocking API!
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
		 * @retval false if the operation failed
		 * 
		 * @sa end()
		 * @sa begin(GyroRange, AccelRange, DLPF, ClockSelect)
		 * @sa begin(FifoBeginFuture&)
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
			if (utils::as_uint8_t(fifo_enable) == 0)
				return begin(gyro_range, accel_range, low_pass_filter, clock_select);
			FifoBeginFuture future{
				fifo_enable, int_enable, sample_rate_divider, gyro_range, accel_range, low_pass_filter, clock_select};
			if (begin(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Put the chip to sleep mode (low-power mode); stops sampling operations 
		 * if any.
		 * @warning Blocking API!
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa begin()
		 * @sa end(EndFuture&)
		 */
		bool end() INLINE
		{
			EndFuture future;
			if (write_power(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Reset the chip (register map §4.28).
		 * @warning Blocking API!
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa reset(ResetFuture&)
		 */
		bool reset() INLINE
		{
			ResetFuture future;
			if (write_power(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get latest gyroscope measurements from the device (register map §4.19).
		 * @warning Blocking API!
		 * 
		 * @param gyro a reference to a `Sensor3D` variable that will be filled
		 * with latest gyroscope measurements on 3 axis.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa gyro_measures(GyroFuture&)
		 */
		bool gyro_measures(Sensor3D& gyro)
		{
			GyroFuture future;
			if (gyro_measures(PARENT::make_proxy(future)) != 0) return false;
			return future.get(gyro);
		}

		/**
		 * Get latest chip temperature measurement (register map §4.18).
		 * The returned value is nternal raw value from the chip, it can be 
		 * converted to human-readable temperature with `convert_temp_to_centi_degrees()`.
		 * @warning Blocking API!
		 * 
		 * @return the latest raw temperature in degrees if no error occurred
		 * @retval -32768 if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 * 
		 * @sa temperature(TemperatureFuture&)
		 * @sa convert_temp_to_centi_degrees()
		 */
		int16_t temperature()
		{
			TemperatureFuture future;
			if (temperature(PARENT::make_proxy(future)) != 0) return INT16_MIN;
			int16_t temp;
			if (future.get(temp))
				return temp;
			else
				return INT16_MIN;
		}

		/**
		 * Get latest accelerometer measurements from the device (register map §4.17).
		 * @warning Blocking API!
		 * 
		 * @param accel a reference to a `Sensor3D` variable that will be filled
		 * with latest accelerometer measurements on 3 axis.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa accel_measures(AccelFuture&)
		 */
		bool accel_measures(Sensor3D& accel)
		{
			AccelFuture future;
			if (accel_measures(PARENT::make_proxy(future)) != 0) return false;
			return future.get(accel);
		}

		/**
		 * Get latest measurements of all device sensors (gyroscope, accelerometer,
		 * temperature).
		 * @warning Blocking API!
		 * 
		 * @param sensors a reference to an `AllSensors` variable that will be filled
		 * with all latest measurements.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa all_measures(AllMeasuresFuture&)
		 */
		bool all_measures(AllSensors& sensors)
		{
			AllMeasuresFuture future;
			if (all_measures(PARENT::make_proxy(future)) != 0) return false;
			return future.get(sensors);
		}

		/**
		 * Get the interrupt status (register map §4.16) after an interrupt has 
		 * occurred. After this method is called, the Interrupt Status register
		 * is cleared.
		 * @warning Blocking API!
		 * 
		 * @return the latest interrupt status as an `INTStatus` structure where
		 * each field maps to the interrupt that occurred; in case of an error,
		 * the returned status is fully cleared. In order to ensure the returned
		 * status can be inspected, you should first call `i2c::I2CManager.status()`.
		 * 
		 * @sa interrupt_status(InterruptStatusFuture&)
		 */
		INTStatus interrupt_status()
		{
			InterruptStatusFuture future;
			if (interrupt_status(PARENT::make_proxy(future)) != 0) return false;
			INTStatus status;
			future.get(status);
			return status;
		}

		/**
		 * Reset the FIFO buffer (parameter map §4.27).
		 * @warning Blocking API!
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa reset_fifo(ResetFifoFuture&)
		 */
		bool reset_fifo()
		{
			ResetFifoFuture future;
			if (reset_fifo(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get the number of bytes currently stored in the FIFO buffer (register 
		 * map §4.30). This number is a multiple of the size of sensor samples as
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @warning Blocking API!
		 * 
		 * @return the number of bytes currently present in the FIFO buffer; when not `0`,
		 * you should read the samples with `fifo_pop()`.
		 * @retval 0 if the FIFO buffer is empty or if the operation failed; 
		 * in order to ensure the returned count is cactually `0`, you should first
		 * call `i2c::I2CManager.status()`.
		 * 
		 * @sa fifo_pop()
		 * @sa fifo_count(FifoCountFuture&)
		 */
		uint16_t fifo_count()
		{
			FifoCountFuture future;
			if (fifo_count(PARENT::make_proxy(future)) != 0) return 0;
			uint16_t count = 0;
			if (!future.get(count)) return 0;
			return count;
		}

		/**
		 * Get one sample out of the FIFO buffer (register map §4.31).
		 * @warning You should first call `fifo_count()` to ensure the MPU6050
		 * FIFO queue contains a sample of the right size! Otherwise this method
		 * will not return any error but set arbitrary values to @p putput !
		 * @warning Blocking API!
		 * 
		 * @tparam T the type of sample to get from the FIFO buffer; must be one
		 * of `Sensor3D`, `int16_t` or `AllSensors`, based on the sensor samples
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @param output a reference to a `T`-type variable that will be filled
		 * with the required sample.
		 * @retval true if a sample has been read into @p output
		 * @retval false if no sample was ready or if the operation failed
		 * 
		 * @sa fifo_count()
		 * @sa fifo_pop(FifoPopFuture<T>&)
		 */
		template<typename T> bool fifo_pop(T& output)
		{
			FifoPopFuture<T> future;
			if (fifo_pop(PARENT::make_proxy(future)) != 0) return false;
			return future.get(output);
		}

	private:
		class Sensor3DFuture : public FUTURE<Sensor3D, uint8_t>
		{
			using PARENT = FUTURE<Sensor3D, uint8_t>;
		protected:
			Sensor3DFuture(uint8_t address) : PARENT{address} {}
			Sensor3DFuture(Sensor3DFuture&&) = default;
			Sensor3DFuture& operator=(Sensor3DFuture&&) = default;
		public:
			bool get(Sensor3D& result)
			{
				if (!PARENT::get(result)) return false;
				format_sensors(result);
				return true;
			}
		};

		class PowerManagement
		{
		public:
			constexpr PowerManagement()
				:	clock_select_{}, temp_disable_{}, reserved_{}, cycle_{}, sleep_{}, device_reset_{} {}
			explicit constexpr PowerManagement(ClockSelect clock_select, bool temp_disable = false,
							bool cycle = false, bool sleep = false, bool device_reset = false) 
				:	clock_select_{uint8_t(clock_select)},
					temp_disable_{temp_disable}, reserved_{}, 
					cycle_{cycle}, sleep_{sleep}, device_reset_{device_reset} {}
			constexpr PowerManagement(bool temp_disable, bool cycle, bool sleep, bool device_reset) 
				:	clock_select_{}, temp_disable_{temp_disable}, reserved_{}, 
					cycle_{cycle}, sleep_{sleep}, device_reset_{device_reset} {}

			ClockSelect clock_select() const
			{
				return static_cast<ClockSelect>(clock_select_);
			}

			bool temp_disable() const
			{
				return temp_disable_;
			}

			bool cycle() const
			{
				return cycle_;
			}

			bool sleep() const
			{
				return sleep_;
			}

			bool device_reset() const
			{
				return device_reset_;
			}

		private:
			uint8_t clock_select_ : 3;
			bool temp_disable_ : 1;
			uint8_t reserved_ : 1;
			bool cycle_ : 1;
			bool sleep_ : 1;
			bool device_reset_ : 1;
		};

		class PowerManagementFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		protected:
			PowerManagementFuture(PowerManagement power) : PARENT{{PWR_MGMT_1, uint8_t(power)}} {}
			PowerManagementFuture(PowerManagementFuture&&) = default;
			PowerManagementFuture& operator=(PowerManagementFuture&&) = default;
		};

		static constexpr uint8_t DEVICE_ADDRESS(AD0 ad0)
		{
			return uint8_t(uint8_t(0x68 | uint8_t(ad0)) << 1);
		}

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
		static constexpr const uint8_t FIFO_RESET = 0x04;
		static constexpr const uint8_t FIFO_ENABLE = 0x40;

		static constexpr const uint8_t PWR_MGMT_1 = 0x6B;
		static constexpr const uint8_t PWR_MGMT_2 = 0x6C;

		static constexpr const uint8_t FIFO_COUNT = 0x72;
		static constexpr const uint8_t FIFO_R_W = 0x74;

		static constexpr const uint8_t WHO_AM_I = 0x75;

		int write_power(PowerManagementFuture& future)
		{
			return this->launch_commands(future, {this->write()});
		}

		static void format_sensors(Sensor3D& sensors)
		{
			utils::swap_bytes(sensors.x);
			utils::swap_bytes(sensors.y);
			utils::swap_bytes(sensors.z);
		}
	};
}

#endif /* MPU6050_H */
/// @endcond
