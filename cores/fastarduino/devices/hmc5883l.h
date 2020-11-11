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
 * API to handle HMC5883L 3-axis digital compass I2C chip.
 * @sa https://github.com/jfpoilpret/fast-arduino-lib/blob/master/refs/devices/HMC5883L.pdf
 */
#ifndef HMC5883L_H
#define HMC5883L_H

#include <math.h>
#include "common_magneto.h"
#include "../i2c_device.h"
#include "../array.h"
#include "../utilities.h"

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
	class Status
	{
	public:
		Status() : ready_{}, lock_{}, reserved_{} {}
		bool ready() const
		{
			return ready_;
		}
		bool lock() const
		{
			return lock_;
		}

	private:
		bool ready_ : 1;
		bool lock_ : 1;
		uint8_t reserved_ : 6;
	};

	/**
	 * I2C device driver for the HMC5883L compass chip.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 * 
	 * The HMC5883L also has a DRDY pin that you can use to an EXT or PCI pin, 
	 * in order to be notified when sensor data is ready for reading; this is
	 * particularly useful in continuous mode, where you would try to avoid busy
	 * waits against HMC5883L status register. The following snippet (excerpt
	 * from `Magneto2` example) show this:
	 * @code
	 * // This handler gets notified when HMC5883L data is ready to read
	 * class DataReadyHandler
	 * {
	 * public:
	 *     DataReadyHandler() : ready_{false}
	 *     {
	 *         interrupt::register_handler(*this);
	 *     }
	 *     void reset()
	 *     {
	 *         ready_ = false;
	 *     }
	 *     bool ready() const
	 *     {
	 *         return ready_;
	 *     }
	 * private:
	 *     void data_ready()
	 *     {
	 *         ready_ = true;
	 *     }
	 *     volatile bool ready_;
	 *     DECL_INT_ISR_HANDLERS_FRIEND
	 * };
	 * 
	 * // EXT pin connected to HMC5883L DRDY pin
	 * static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D2_PD2_EXT0;
	 * // Register our handler with DRDY EXT pin interrupts
	 * REGISTER_INT_ISR_METHOD(0, DRDY, DataReadyHandler, &DataReadyHandler::data_ready)
	 * 
	 * int main()
	 * {
	 *     // Perform other necessary initializations here,
	 *     // including I2C Manager and HMC5883L device (named compass in hte following code)
	 *     ...
	 *     // Initialize DRDY notifications handler
	 *     DataReadyHandler handler;
	 *     interrupt::INTSignal<DRDY> signal{interrupt::InterruptTrigger::RISING_EDGE};
	 *     signal.enable();
	 * 
	 *     // Start compass in continuous mode
	 *     compass.begin(OperatingMode::CONTINUOUS,
	 *                   Gain::GAIN_1_9GA,
	 *                   DataOutput::RATE_0_75HZ,
	 *                   SamplesAveraged::EIGHT_SAMPLES);
	 *     while (true)
	 *     {
	 *         // Wait until data is ready (time::yield() will put MCU to sleep)
	 *         while (!handler.ready()) time::yield();
	 *         handler.reset();
	 *         // Read compass fields
	 *         Sensor3D fields;
	 *         compass.magnetic_fields(fields);
	 *         // Use compass fields in your program
	 *         ...
	 *     }
	 * }
	 * @endcode
	 */
	template<typename MANAGER>
	class HMC5883L : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		// Forward declarations needed by compiler
		class WriteRegisterFuture;
		template<typename T> class ReadRegisterFuture;

	public:
		/**
		 * Create a new device driver for a HMC5883L chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit HMC5883L(MANAGER& manager) : PARENT{manager, DEVICE_ADDRESS, i2c::I2C_FAST, false} {}

		// Asynchronous API
		//==================
		/**
		 * Create a future to be used by asynchronous method begin(BeginFuture&).
		 * This is used by `begin()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished, hence
		 * when you may use other methods such as `magnetic_fields()` to get sensors
		 * measurements from the device.
		 * 
		 * @param mode the `OperatingMode` to operate this chip
		 * @param gain the `Gain` to use to increase measured magnetic fields
		 * @param rate the `DataOutput` rate to use in `OperatingMode::CONTINUOUS` mode
		 * @param samples the `SamplesAveraged` to use for each measurement
		 * @param measurement the `MeasurementMode` to use on the chip sensors
		 * 
		 * @sa begin(BeginFuture&)
		 */
		class BeginFuture : public FUTURE<void, containers::array<uint8_t, 6>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 6>>;
		public:
			/// @cond notdocumented
			explicit BeginFuture(	OperatingMode mode = OperatingMode::SINGLE,
									Gain gain = Gain::GAIN_1_3GA,
									DataOutput rate = DataOutput::RATE_15HZ, 
									SamplesAveraged samples = SamplesAveraged::ONE_SAMPLE,
									MeasurementMode measurement = MeasurementMode::NORMAL)
				:	PARENT{{	CONFIG_REG_A, uint8_t(uint8_t(measurement) | uint8_t(rate) | uint8_t(samples)),
								CONFIG_REG_B, uint8_t(gain),
								MODE_REG, uint8_t(mode)}} {}
			BeginFuture(BeginFuture&&) = default;
			BeginFuture& operator=(BeginFuture&&) = default;

			Gain gain() const
			{
				return static_cast<Gain>(this->get_input()[3]);
			}
			/// @endcond
		};

		/**
		 * Start operation of this compass chip. Once this method has been called,
		 * you may use `magnetic_fields()` to find out the directions of the device.
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
		 * @sa begin(OperatingMode, Gain, DataOutput, SamplesAveraged, MeasurementMode)
		 * @sa errors
		 */
		int begin(PROXY<BeginFuture> future)
		{
			gain_ = GAIN(this->resolve(future).gain());
			// We split the transaction in 3 write commands (1 byte at CONFIG_REG_A, CONFIG_REG_B and MODE_REG)
			return this->launch_commands(future, {this->write(2), this->write(2), this->write(2)});
		}

		/**
		 * Create a future to be used by asynchronous method end(EndFuture&).
		 * This is used by `end()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa end(EndFuture&)
		 */
		class EndFuture : public WriteRegisterFuture
		{
		public:
			/// @cond notdocumented
			EndFuture() : WriteRegisterFuture{MODE_REG, uint8_t(OperatingMode::IDLE)} {}
			EndFuture(EndFuture&&) = default;
			EndFuture& operator=(EndFuture&&) = default;
			/// @endcond
		};

		/**
		 * Stop operation of this compass chip. You should not call `magnetic_fields()`
		 * after calling this method.
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
		 * @sa EndFuture
		 * @sa end()
		 * @sa begin(BeginFuture&)
		 * @sa errors
		 */
		int end(PROXY<EndFuture> future) INLINE
		{
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method status(StatusFuture&).
		 * This is used by `status()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa status(StatusFuture&)
		 */
		class StatusFuture : public ReadRegisterFuture<Status>
		{
		public:
			/// @cond notdocumented
			StatusFuture() : ReadRegisterFuture<Status>{STATUS_REG} {}
			StatusFuture(StatusFuture&&) = default;
			StatusFuture& operator=(StatusFuture&&) = default;
			/// @endcond
		};

		/**
		 * Get the curent chip status.
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
		 * @sa StatusFuture
		 * @sa status()
		 * @sa errors
		 */
		int status(PROXY<StatusFuture> future) INLINE
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method magnetic_fields(MagneticFieldsFuture&).
		 * This is used by `magnetic_fields()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @sa magnetic_fields(MagneticFieldsFuture&)
		 * @sa convert_fields_to_mGA()
		 */
		class MagneticFieldsFuture : public ReadRegisterFuture<Sensor3D>
		{
			using PARENT = ReadRegisterFuture<Sensor3D>;
		public:
			/// @cond notdocumented
			MagneticFieldsFuture() : PARENT{OUTPUT_REG_1} {}
			MagneticFieldsFuture(MagneticFieldsFuture&&) = default;
			MagneticFieldsFuture& operator=(MagneticFieldsFuture&&) = default;

			bool get(Sensor3D& fields)
			{
				if (!PARENT::get(fields)) return false;
				utils::swap_bytes(fields.x);
				utils::swap_bytes(fields.y);
				utils::swap_bytes(fields.z);
				return true;
			}
			/// @endcond
		};

		/**
		 * Read the magnetic fields (as raw values) on 3 axes (datasheet p15-16).
		 * In order to convert raw measurements to physical values, you should
		 * call `convert_fields_to_mGA()`.
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
		 * @sa MagneticFieldsFuture
		 * @sa convert_fields_to_mGA()
		 * @sa magnetic_fields(Sensor3D&)
		 * @sa errors
		 */
		int magnetic_fields(PROXY<MagneticFieldsFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		// Synchronous API
		//=================
		/**
		 * Start operation of this compass chip. Once this method has been called,
		 * you may use `magnetic_fields()` to find out the directions of the device.
		 * @warning Blocking API!
		 * 
		 * @param mode the `OperatingMode` to operate this chip
		 * @param gain the `Gain` to use to increase measured magnetic fields
		 * @param rate the `DataOutput` rate to use in `OperatingMode::CONTINUOUS` mode
		 * @param samples the `SamplesAveraged` to use for each measurement
		 * @param measurement the `MeasurementMode` to use on the chip sensors
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa end()
		 * @sa begin(BeginFuture&)
		 */
		bool begin(OperatingMode mode = OperatingMode::SINGLE, Gain gain = Gain::GAIN_1_3GA,
				   DataOutput rate = DataOutput::RATE_15HZ, SamplesAveraged samples = SamplesAveraged::ONE_SAMPLE,
				   MeasurementMode measurement = MeasurementMode::NORMAL)
		{
			BeginFuture future{mode, gain, rate, samples, measurement};
			if (begin(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Stop operation of this compass chip. You should not call `magnetic_fields()`
		 * after calling this method.
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
			if (end(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get the curent chip status.
		 * @warning Blocking API!
		 * 
		 * @sa Status
		 * @sa status(StatusFuture&)
		 */
		Status status() INLINE
		{
			StatusFuture future;
			if (status(PARENT::make_proxy(future)) != 0) return Status{};
			Status status;
			if (future.get(status))
				return status;
			else
				return Status{};
		}

		/**
		 * Read the magnetic fields (as raw values) on 3 axes (datasheet p15-16).
		 * In order to convert raw measurements to physical values, you should
		 * call `convert_fields_to_mGA()`.
		 * @warning Blocking API!
		 * 
		 * @param fields a reference to a `Sensor3D` variable that will be
		 * filled with values upon method return
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa convert_fields_to_mGA()
		 * @sa magnetic_fields(MagneticFieldsFuture&)
		 */
		bool magnetic_fields(Sensor3D& fields)
		{
			MagneticFieldsFuture future;
			if (magnetic_fields(PARENT::make_proxy(future)) != 0) return false;
			return future.get(fields);
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

		class WriteRegisterFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		protected:
			WriteRegisterFuture(uint8_t address, uint8_t value) : PARENT{{address, value}} {}
			WriteRegisterFuture(WriteRegisterFuture&&) = default;
			WriteRegisterFuture& operator=(WriteRegisterFuture&&) = default;
		};

		template<typename T>
		class ReadRegisterFuture : public FUTURE<T, uint8_t>
		{
			using PARENT = FUTURE<T, uint8_t>;
		protected:
			explicit ReadRegisterFuture(uint8_t address) : PARENT{{address}} {}
			ReadRegisterFuture(ReadRegisterFuture<T>&&) = default;
			ReadRegisterFuture<T>& operator=(ReadRegisterFuture<T>&&) = default;
		};

		void convert_field_to_mGa(int16_t& value)
		{
			value = value * 1000L / gain_;
		}

		static constexpr uint16_t GAIN(Gain gain)
		{
			if (gain == Gain::GAIN_0_88GA) return 1370;
			if (gain == Gain::GAIN_1_3GA) return 1090;
			if (gain == Gain::GAIN_1_9GA) return 820;
			if (gain == Gain::GAIN_2_5GA) return 660;
			if (gain == Gain::GAIN_4_0GA) return 440;
			if (gain == Gain::GAIN_4_7GA) return 390; 
			if (gain == Gain::GAIN_5_6GA) return 330;
			return 230;
		}

		uint16_t gain_;
	};
}

#endif /* HMC5883L_H */
/// @endcond
