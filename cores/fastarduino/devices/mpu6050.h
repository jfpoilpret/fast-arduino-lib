//   Copyright 2016-2021 Jean-Francois Poilpret
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
#include "../functors.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"
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
	 * This can be expressed either from the gyroscope viewpoint or from the
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
			:	data_{
					enable(accel, ACCEL_FIFO_EN) | enable(temperature, TEMP_FIFO_EN) |
					enable(gyro_x, XG_FIFO_EN) | enable(gyro_y, YG_FIFO_EN) | enable(gyro_z, ZG_FIFO_EN)} {}

		/** If `true`, accelerometer measures on 3 axes will be loaded to FIFO buffer. */
		bool accel() const
		{
			return data_ & ACCEL_FIFO_EN;
		}

		/** If `true`, gyroscope measures on X axis will be loaded to FIFO buffer. */
		bool gyro_x() const
		{
			return data_ & XG_FIFO_EN;
		}

		/** If `true`, gyroscope measures on Y axis will be loaded to FIFO buffer. */
		bool gyro_y() const
		{
			return data_ & YG_FIFO_EN;
		}

		/** If `true`, gyroscope measures on Z axis will be loaded to FIFO buffer. */
		bool gyro_z() const
		{
			return data_ & ZG_FIFO_EN;
		}

		/** If `true`, chip temperature will be loaded to FIFO buffer. */
		bool temperature() const
		{
			return data_ & TEMP_FIFO_EN;
		}

	private:
		static constexpr uint8_t TEMP_FIFO_EN = bits::BV8(7);
		static constexpr uint8_t XG_FIFO_EN = bits::BV8(6);
		static constexpr uint8_t YG_FIFO_EN = bits::BV8(5);
		static constexpr uint8_t ZG_FIFO_EN = bits::BV8(4);
		static constexpr uint8_t ACCEL_FIFO_EN = bits::BV8(3);

		static constexpr uint8_t enable(bool flag, uint8_t mask)
		{
			return (flag ? mask : 0);
		}
		
		uint8_t data_;
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
			: data_{enable(data_ready, DATA_RDY_INT) | enable(overflow, FIFO_OFLOW_INT)} {}

		/** If `true`, the Data Ready interrupt is enabled. */
		bool data_ready() const
		{
			return data_ & DATA_RDY_INT;
		}

		/** If `true`, a FIFO buffer overflow will generate an interrupt. */
		bool overflow() const
		{
			return data_ & FIFO_OFLOW_INT;
		}

	private:
		static constexpr uint8_t FIFO_OFLOW_INT = bits::BV8(4);
		static constexpr uint8_t DATA_RDY_INT = bits::BV8(0);

		static constexpr uint8_t enable(bool flag, uint8_t mask)
		{
			return (flag ? mask : 0);
		}

		uint8_t data_;
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
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class MPU6050 : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		// Forward declarations needed by compiler
		template<uint8_t REGISTER, typename T = uint8_t, typename FUNCTOR = functor::ChangeEndianness<T>>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, REGISTER, T, FUNCTOR>;
		template<uint8_t REGISTER, typename T = uint8_t, typename FUNCTOR = functor::ChangeEndianness<T>>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, REGISTER, T, FUNCTOR>;

		template<uint8_t REGISTER>
		using Sensor3DFuture = TReadRegisterFuture<REGISTER, Sensor3D>;

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

		class PowerManagement;
		using PowerManagementFuture = TWriteRegisterFuture<PWR_MGMT_1, PowerManagement>;

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
			: PARENT{manager, DEVICE_ADDRESS(ad0), i2c::I2C_FAST, true} {}

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
			/// @cond notdocumented
			explicit BeginFuture(	GyroRange gyro_range = GyroRange::RANGE_250,
									AccelRange accel_range = AccelRange::RANGE_2G,
									DLPF low_pass_filter = DLPF::ACCEL_BW_260HZ,
									ClockSelect clock_select = ClockSelect::INTERNAL_8MHZ)
				: PARENT{{	CONFIG, uint8_t(low_pass_filter), uint8_t(gyro_range), uint8_t(accel_range),
							PWR_MGMT_1, utils::as_uint8_t(PowerManagement{clock_select})}} {}
			BeginFuture(BeginFuture&&) = default;
			BeginFuture& operator=(BeginFuture&&) = default;
			/// @endcond
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
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
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
		class FifoBeginFuture : public FUTURE<void, containers::array<uint8_t, 15>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 15>>;
		public:
			/// @cond notdocumented
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
							USER_CTRL, FIFO_ENABLE | FIFO_RESET}} {}
			FifoBeginFuture(FifoBeginFuture&&) = default;
			FifoBeginFuture& operator=(FifoBeginFuture&&) = default;
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
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
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
			return this->launch_commands(future, {	
				// CONFIG, GYRO_CONFIG, ACCEL_CONFIG
				this->write(4),
				// PWR_MGMT_1
				this->write(2),
				// SMPRT_DIV
				this->write(2),
				// FIFO_EN
				this->write(2),
				// INT_PIN_CFG
				this->write(3),
				// USER_CONTROL
				this->write(2),
			});
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
			/// @cond notdocumented
			EndFuture() : PowerManagementFuture{PowerManagement{false, false, true, false}} {}
			EndFuture(EndFuture&&) = default;
			EndFuture& operator=(EndFuture&&) = default;
			/// @endcond
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
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
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
			return this->async_write(future);
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
			/// @cond notdocumented
			ResetFuture() : PowerManagementFuture{PowerManagement{false, false, false, true}} {}
			ResetFuture(ResetFuture&&) = default;
			ResetFuture& operator=(ResetFuture&&) = default;
			/// @endcond
		};

		/**
		 * Reset the chip (register map §4.28).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `ResetFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa reset()
		 * @sa errors
		 */
		int reset(PROXY<ResetFuture> future) INLINE
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method gyro_measures(GyroFuture&).
		 * This is used by `gyro_measures()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa gyro_measures(GyroFuture&)
		 */
		using GyroFuture = Sensor3DFuture<GYRO_XOUT>;

		/**
		 * Get latest gyroscope measurements from the device (register map §4.19).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `GyroFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa gyro_measures(Sensor3D&)
		 * @sa errors
		 */
		int gyro_measures(PROXY<GyroFuture> future)
		{
			return this->async_read(future);
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
		using TemperatureFuture = TReadRegisterFuture<TEMP_OUT, int16_t>;

		/**
		 * Get latest chip temperature measurement (register map §4.18).
		 * @warning Asynchronous API!
		 * 
		 * @param future a `TemperatureFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
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
			return this->async_read(future);
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
		using AccelFuture = Sensor3DFuture<ACCEL_XOUT>;

		/**
		 * Get latest accelerometer measurements from the device (register map §4.17).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `AccelFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa accel_measures(Sensor3D&)
		 * @sa errors
		 */
		int accel_measures(PROXY<AccelFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method all_measures(AllMeasuresFuture&).
		 * This is used by `all_measures()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa all_measures(AllMeasuresFuture&)
		 */
		using AllMeasuresFuture = 
			TReadRegisterFuture<ACCEL_XOUT, AllSensors, functor::ChangeEndianness<AllSensors, int16_t>>;

		/**
		 * Get latest measurements of all device sensors (gyroscope, accelerometer,
		 * temperature).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `AllMeasuresFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa all_measures(AllSensors&)
		 * @sa errors
		 */
		int all_measures(PROXY<AllMeasuresFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method interrupt_status(InterruptStatusFuture&).
		 * This is used by `interrupt_status()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa interrupt_status(InterruptStatusFuture&)
		 */
		using InterruptStatusFuture = TReadRegisterFuture<INT_STATUS, INTStatus>;

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
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa interrupt_status()
		 * @sa errors
		 */
		int interrupt_status(PROXY<InterruptStatusFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method reset_fifo(ResetFifoFuture&).
		 * This is used by `reset_fifo()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa reset_fifo(ResetFifoFuture&)
		 */
		using ResetFifoFuture = 
			TWriteRegisterFuture<USER_CTRL, uint8_t, functor::Constant<uint8_t, FIFO_ENABLE | FIFO_RESET>>;

		/**
		 * Reset the FIFO buffer (parameter map §4.27).
		 * @warning Asynchronous API!
		 * 
		 * @param future an `ResetFifoFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa reset_fifo()
		 * @sa errors
		 */
		int reset_fifo(PROXY<ResetFifoFuture> future)
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method fifo_count(FifoCountFuture&).
		 * This is used by `fifo_count()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa fifo_count(FifoCountFuture&)
		 */
		using FifoCountFuture = TReadRegisterFuture<FIFO_COUNT, uint16_t>;

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
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa fifo_pop()
		 * @sa fifo_count()
		 * @sa errors
		 */
		int fifo_count(PROXY<FifoCountFuture> future)
		{
			return this->async_read(future);
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
		using FifoPopFuture = TReadRegisterFuture<FIFO_R_W, T, functor::ChangeEndianness<T, int16_t>>;

		/**
		 * Get one sample out of the FIFO buffer (register map §4.31).
		 * @warning You should first call `fifo_count()` to ensure the MPU6050
		 * FIFO queue contains a sample of the right size! Otherwise this method
		 * will not return any error but set arbitrary values to @p output !
		 * @warning Asynchronous API!
		 * 
		 * @tparam T the type of sample to get from the FIFO buffer; must be one
		 * of `Sensor3D`, `int16_t` or `AllSensors`, based on the sensor samples
		 * selected by `FIFOEnable` in
		 * `begin(FIFOEnable, INTEnable, uint8_t, GyroRange, AccelRange, DLPF, ClockSelect)`.
		 * @param future an `FifoPopFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa fifo_count()
		 * @sa fifo_pop(T&)
		 * @sa errors
		 */
		template<typename T> int fifo_pop(PROXY<FifoPopFuture<T>> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method fifo_push(FifoPushFuture&).
		 * This is used by `fifo_push(FifoPushFuture&)` to asynchronously launch the 
		 * I2C transaction, and it shall be used by the caller to determine when the
		 * I2C transaction is finished.
		 * 
		 * @param data the byte value to be pushed
		 * 
		 * @sa fifo_push(FifoPushFuture&)
		 */
		using FifoPushFuture = TWriteRegisterFuture<FIFO_R_W>;

		/**
		 * Push one byte to the FIFO buffer (register map §4.31).
		 * @warning You should first call `fifo_count()` to ensure the MPU6050
		 * FIFO queue contains a sample of the right size! Otherwise this method
		 * will not return any error but set arbitrary values to @p output !
		 * @warning Asynchronous API!
		 * 
		 * @param future an `FifoPushFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa fifo_count()
		 * @sa fifo_push(uint8_t)
		 * @sa errors
		 */
		int fifo_push(PROXY<FifoPushFuture> future)
		{
			return this->async_write(future);
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
			return this->template sync_write<EndFuture>();
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
			return this->template sync_write<ResetFuture>();
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
			return this->template sync_read<GyroFuture>(gyro);
		}

		/**
		 * Get latest chip temperature measurement (register map §4.18).
		 * The returned value is nternal raw value from the chip, it can be 
		 * converted to human-readable temperature with `convert_temp_to_centi_degrees()`.
		 * @warning Blocking API!
		 * 
		 * @return the latest raw temperature in degrees if no error occurred
		 * @retval -32768 if the operation failed
		 * 
		 * @sa temperature(TemperatureFuture&)
		 * @sa convert_temp_to_centi_degrees()
		 */
		int16_t temperature()
		{
			int16_t temp = INT16_MIN;
			this->template sync_read<TemperatureFuture>(temp);
			return temp;
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
			return this->template sync_read<AccelFuture>(accel);
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
			return this->template sync_read<AllMeasuresFuture>(sensors);
		}

		/**
		 * Get the interrupt status (register map §4.16) after an interrupt has 
		 * occurred. After this method is called, the Interrupt Status register
		 * is cleared.
		 * @warning Blocking API!
		 * 
		 * @return the latest interrupt status as an `INTStatus` structure where
		 * each field maps to the interrupt that occurred; in case of an error,
		 * the returned status is fully cleared.
		 * 
		 * @sa interrupt_status(InterruptStatusFuture&)
		 */
		INTStatus interrupt_status()
		{
			INTStatus status;
			this->template sync_read<InterruptStatusFuture>(status);
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
			return this->template sync_write<ResetFifoFuture>();
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
		 * @retval 0 if the FIFO buffer is empty or if the operation failed
		 * 
		 * @sa fifo_pop()
		 * @sa fifo_count(FifoCountFuture&)
		 */
		uint16_t fifo_count()
		{
			uint16_t count = 0;
			this->template sync_read<FifoCountFuture>(count);
			return count;
		}

		/**
		 * Get one sample out of the FIFO buffer (register map §4.31).
		 * @warning You should first call `fifo_count()` to ensure the MPU6050
		 * FIFO queue contains a sample of the right size! Otherwise this method
		 * will not return any error but set arbitrary values to @p output !
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
			return this->template sync_read<FifoPopFuture<T>>(output);
		}

		/**
		 * Push one byte to the FIFO buffer (register map §4.31).
		 * @warning Blocking API!
		 * 
		 * @param data the byte value to be pushed
		 * @retval true if @p data has been correctly pushed to the FIFO
		 * @retval false if the operation failed
		 * 
		 * @sa fifo_push(FifoPushFuture&)
		 */
		bool fifo_push(uint8_t data)
		{
			return this->template sync_write<FifoPushFuture>(data);
		}

	private:
		class PowerManagement
		{
		public:
			constexpr PowerManagement() : value_{} {}
			explicit constexpr PowerManagement(ClockSelect clock_select, bool temp_disable = false,
							bool cycle = false, bool sleep = false, bool device_reset = false) 
				:	value_{	uint8_t(uint8_t(clock_select) | 
							(temp_disable ? TEMP_DIS_MASK : 0) |
							(cycle ? CYCLE_MASK : 0) |
							(sleep ? SLEEP_MASK : 0) |
							(device_reset ? RESET_MASK : 0))} {}
			constexpr PowerManagement(bool temp_disable, bool cycle, bool sleep, bool device_reset) 
				:	value_{	(temp_disable ? TEMP_DIS_MASK : 0) |
							(cycle ? CYCLE_MASK : 0) |
							(sleep ? SLEEP_MASK : 0) |
							(device_reset ? RESET_MASK : 0)} {}

			ClockSelect clock_select() const
			{
				return static_cast<ClockSelect>(value_ & CLOCK_SEL_MASK);
			}

			bool temp_disable() const
			{
				return value_ & TEMP_DIS_MASK;
			}

			bool cycle() const
			{
				return value_ & CYCLE_MASK;
			}

			bool sleep() const
			{
				return value_ & SLEEP_MASK;
			}

			bool device_reset() const
			{
				return value_ & RESET_MASK;
			}

		private:
			static constexpr uint8_t CLOCK_SEL_MASK = bits::BV8(0, 1, 2);
			static constexpr uint8_t TEMP_DIS_MASK = bits::BV8(3);
			static constexpr uint8_t CYCLE_MASK = bits::BV8(5);
			static constexpr uint8_t SLEEP_MASK = bits::BV8(6);
			static constexpr uint8_t RESET_MASK = bits::BV8(7);
			uint8_t value_;
		};

		static constexpr uint8_t DEVICE_ADDRESS(AD0 ad0)
		{
			return uint8_t(uint8_t(0x68 | uint8_t(ad0)) << 1);
		}
	};
}

#endif /* MPU6050_H */
/// @endcond
