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

/// @cond notdocumented

#ifndef VL53L0X_FUTURES_H
#define VL53L0X_FUTURES_H

#include "../array.h"
#include "../flash.h"
#include "../i2c.h"
#include "../future.h"
#include "../utilities.h"
#include "../i2c_handler.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"
#include "vl53l0x_internals.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x
{
	// Forward declaration
	template<typename MANAGER> class VL53L0X;
}

//TODO review all futures to add args checks when needed
namespace devices::vl53l0x_futures
{
	// Shortened aliases for various namespaces
	namespace internals = vl53l0x_internals;
	namespace regs = vl53l0x_registers;
	namespace actions = i2c::actions;

	// static utilities to support fixed point 9/7 bits used by VL53L0X chip
	class FixPoint9_7
	{
	public:
		static constexpr bool is_valid(float value)
		{
			return ((value >= 0.0) && (value < float(1 << INTEGRAL_BITS)));
		}

		static constexpr uint16_t convert(float value)
		{
			return is_valid(value) ? uint16_t(value * (1 << DECIMAL_BITS)) : 0U;
		}

		static constexpr float convert(uint16_t value)
		{
			return value / float(1 << DECIMAL_BITS);
		}

	private:
		static constexpr uint16_t INTEGRAL_BITS = 9;
		static constexpr uint16_t DECIMAL_BITS = 7;
	};

	// Utilities for timing budget computations
	class TimingBudgetUtilities
	{
	private:
		static constexpr const uint32_t MIN_TIMING_BUDGET    = 20000UL;
		static constexpr const uint16_t START_OVERHEAD       = 1910U;
		static constexpr const uint16_t END_OVERHEAD         = 960U;
		static constexpr const uint16_t MSRC_OVERHEAD        = 660U;
		static constexpr const uint16_t TCC_OVERHEAD         = 590U;
		static constexpr const uint16_t DSS_OVERHEAD         = 690U;
		static constexpr const uint16_t PRE_RANGE_OVERHEAD   = 660U;
		static constexpr const uint16_t FINAL_RANGE_OVERHEAD = 550U;

	public:
		static uint32_t calculate_measurement_timing_budget_us(
			const vl53l0x::SequenceSteps steps, const vl53l0x::SequenceStepsTimeout& timeouts)
		{
			// start and end overhead times always present
			uint32_t budget_us = START_OVERHEAD + END_OVERHEAD;

			if (steps.is_tcc())
				budget_us += timeouts.msrc_dss_tcc_us() + TCC_OVERHEAD;

			if (steps.is_dss())
				budget_us += 2 * (timeouts.msrc_dss_tcc_us() + DSS_OVERHEAD);
			else if (steps.is_msrc())
				budget_us += timeouts.msrc_dss_tcc_us() + MSRC_OVERHEAD;

			if (steps.is_pre_range())
				budget_us += timeouts.pre_range_us() + PRE_RANGE_OVERHEAD;

			if (steps.is_final_range())
				budget_us += timeouts.final_range_us(steps.is_pre_range()) + FINAL_RANGE_OVERHEAD;

			return budget_us;
		}

		static uint16_t calculate_final_range_timeout_mclks(
			const vl53l0x::SequenceSteps steps, const vl53l0x::SequenceStepsTimeout& timeouts, uint32_t budget_us)
		{
			// Requested budget must be be above minimum allowed
			if (budget_us < MIN_TIMING_BUDGET) return 0;
			// This calculation is useless if there is no final range step
			if (!steps.is_final_range()) return 0;

			// Calculate current used budget without final range
			uint32_t used_budget_us = 
				calculate_measurement_timing_budget_us(steps.no_final_range(), timeouts);

			// Now include final range and calculate difference
			used_budget_us += FINAL_RANGE_OVERHEAD;
			// Requested budget must be above calculated budget for all other steps
			if (used_budget_us > budget_us) return 0;

			// Calculate final range timeout in us
			const uint32_t final_range_timeout_us = budget_us - used_budget_us;

			// Deduce final range timeout in mclks
			uint32_t final_range_timeout_mclks = vl53l0x::TimeoutUtilities::calculate_timeout_mclks(
				final_range_timeout_us, timeouts.final_range_vcsel_period_pclks());
			if (steps.is_pre_range())
				final_range_timeout_mclks += timeouts.pre_range_mclks();
			
			return vl53l0x::TimeoutUtilities::encode_timeout(final_range_timeout_mclks);
		}
	};

	// This fake class gathers all futures specific to VL53L0X device
	template<typename MANAGER> struct Futures
	{
		// Ensure MANAGER is an accepted I2C Manager type
		static_assert(i2c::I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER_ must be a valid I2C Manager type");

		using DEVICE = vl53l0x::VL53L0X<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		template<typename OUT, typename IN> using FUTURE = typename MANAGER::template FUTURE<OUT, IN>;

		using FUTURE_OUTPUT_LISTENER = future::FutureOutputListener<ABSTRACT_FUTURE>;
		using FUTURE_STATUS_LISTENER = future::FutureStatusListener<ABSTRACT_FUTURE>;

		template<typename T> 
		using ReadRegisterFuture = i2c::ReadRegisterFuture<MANAGER, T, true>;
		template<typename T>
		using WriteRegisterFuture = i2c::WriteRegisterFuture<MANAGER, T, true>;

		template<uint8_t REGISTER, typename T = uint8_t>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, REGISTER, T, true>;
		template<uint8_t REGISTER, typename T = uint8_t>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, REGISTER, T, true>;

		using AbstractI2CFuturesGroup = i2c::AbstractI2CFuturesGroup<MANAGER>;
		using I2CFuturesGroup = i2c::I2CFuturesGroup<MANAGER>;
		using I2CSameFutureGroup = i2c::I2CSameFutureGroup<MANAGER>;
		using ComplexI2CFuturesGroup = i2c::ComplexI2CFuturesGroup<MANAGER>;

		template<uint8_t SIZE> using FUTURE_WRITE = i2c::FUTURE_WRITE<MANAGER, SIZE>;
		template<uint8_t SIZE> using FUTURE_READ = i2c::FUTURE_READ<MANAGER, SIZE>;
		using FUTURE_READ1 = i2c::FUTURE_READ1<MANAGER>;

		using GetSequenceStepsFuture = TReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, vl53l0x::SequenceSteps>;

		class DeviceStrobeWaitFuture : public AbstractI2CFuturesGroup
		{
			using PARENT = AbstractI2CFuturesGroup;
		public:
			DeviceStrobeWaitFuture(FUTURE_STATUS_LISTENER* listener = nullptr) : PARENT{listener}
			{
				PARENT::init({&write_, &read_}, PARENT::NO_LIMIT);
			}

			//TODO make protected and declare proper friends?
			bool start(DEVICE& device)
			{
				PARENT::set_device(device);
				return write_strobe(0x00, StrobeStep::STEP_INIT_STROBE);
			}

		private:
			enum class StrobeStep : uint8_t
			{
				STEP_INIT_STROBE = 0,
				STEP_READ_STROBE = 1,
				STEP_EXIT_STROBE = 2
			};

			bool write_strobe(uint8_t value, StrobeStep next_step)
			{
				write_.reset_(value);
				step_ = next_step;
				return PARENT::check_error(PARENT::launch_commands(write_, {PARENT::write(0, false, true)}));
			}

			void read_strobe()
			{
				read_.reset_();
				PARENT::check_error(PARENT::launch_commands(read_, {PARENT::write(), PARENT::read()}));
			}

			bool check_strobe()
			{
				uint8_t strobe = 0;
				read_.get(strobe);
				return (strobe != 0);
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				if (status != future::FutureStatus::READY) return;
				switch (step_)
				{
					case StrobeStep::STEP_READ_STROBE:
					if (check_strobe())
					{
						// Strobe is OK, go to last step
						write_strobe(0x01, StrobeStep::STEP_EXIT_STROBE);
						return;
					}
					if (++loop_ >= MAX_LOOP)
					{
						// Strobe is not OK after too many loops, abandon
						PARENT::check_error(errors::ETIME);
						return;
					}
					// Read strobe again
					// Intentional fallthrough
					
					case StrobeStep::STEP_INIT_STROBE:
					// Initial write strobe is finished, start loop reading probe
					step_ = StrobeStep::STEP_READ_STROBE;
					read_strobe();
					break;
					
					case StrobeStep::STEP_EXIT_STROBE:
					PARENT::set_future_finish_();
					break;
				}
			}

			static constexpr const uint16_t MAX_LOOP = 2000;

			StrobeStep step_ = StrobeStep::STEP_INIT_STROBE;
			uint16_t loop_ = 0;
			TWriteRegisterFuture<regs::REG_DEVICE_STROBE> write_{0x00};
			TReadRegisterFuture<regs::REG_DEVICE_STROBE> read_{};
		};

		//TODO do we still need that intermediate class?
		class AbstractGetVcselPulsePeriodFuture : public ReadRegisterFuture<uint8_t>
		{
			using PARENT = ReadRegisterFuture<uint8_t>;

		public:
			explicit AbstractGetVcselPulsePeriodFuture(uint8_t reg, FUTURE_STATUS_LISTENER* listener = nullptr)
				: PARENT{reg, listener} {}
			bool get(uint8_t& result)
			{
				if (!PARENT::get(result)) return false;
				result = (result + 1) << 1;
				return true;
			}
			AbstractGetVcselPulsePeriodFuture(AbstractGetVcselPulsePeriodFuture&&) = default;
			AbstractGetVcselPulsePeriodFuture& operator=(AbstractGetVcselPulsePeriodFuture&&) = default;
		};

		template<vl53l0x::VcselPeriodType TYPE>
		class GetVcselPulsePeriodFuture : public AbstractGetVcselPulsePeriodFuture
		{
			using PARENT = AbstractGetVcselPulsePeriodFuture;

		public:
			explicit GetVcselPulsePeriodFuture(FUTURE_STATUS_LISTENER* status_listener = nullptr)
			:	PARENT{uint8_t(TYPE), status_listener} {}
			GetVcselPulsePeriodFuture(GetVcselPulsePeriodFuture&&) = default;
			GetVcselPulsePeriodFuture& operator=(GetVcselPulsePeriodFuture&&) = default;
		};

		//TODO rework (Group of futures?)
		template<vl53l0x::VcselPeriodType TYPE>
		class SetVcselPulsePeriodFuture : public TWriteRegisterFuture<uint8_t(TYPE)>
		{
			using PARENT = TWriteRegisterFuture<uint8_t(TYPE)>;
			static uint8_t encode_period(uint8_t period)
			{
				return (period >> 1) - 1;
			}

		public:
			explicit SetVcselPulsePeriodFuture(uint8_t period_pclks) : PARENT{encode_period(period_pclks)} {}
			SetVcselPulsePeriodFuture(SetVcselPulsePeriodFuture<TYPE>&&) = default;
			SetVcselPulsePeriodFuture& operator=(SetVcselPulsePeriodFuture<TYPE>&&) = default;
			//TODO check compliance of period_clicks
		};

		class GetSignalRateLimitFuture : 
			public TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

		public:
			explicit GetSignalRateLimitFuture() : PARENT{} {}
			GetSignalRateLimitFuture(GetSignalRateLimitFuture&&) = default;
			GetSignalRateLimitFuture& operator=(GetSignalRateLimitFuture&&) = default;

			bool get(float& result)
			{
				uint16_t temp = 0;
				if (!PARENT::get(temp)) return false;
				result = FixPoint9_7::convert(temp);
				return true;
			}
		};

		class SetSignalRateLimitFuture : 
			public TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

		public:
			explicit SetSignalRateLimitFuture(float signal_rate)
				:	PARENT{FixPoint9_7::convert(signal_rate)} {}
			SetSignalRateLimitFuture(SetSignalRateLimitFuture&&) = default;
			SetSignalRateLimitFuture& operator=(SetSignalRateLimitFuture&&) = default;
		};

		class GetSPADInfoFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;
			
		public:
			GetSPADInfoFuture() : PARENT{uint16_t(internals::spad_info::BUFFER)}
			{
				PARENT::init({&write1_, &write2_, &read_, &strobe_}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

			bool get(vl53l0x::SPADInfo& info)
			{
				if (PARENT::await() != future::FutureStatus::READY)
					return false;
				info = vl53l0x::SPADInfo{info_};
				return true;
			}

		protected:
			bool start(DEVICE& device)
			{
				PARENT::set_device(device);
				device_ = &device;
				return next_future();
			}

		private:
			bool process_marker(uint8_t marker)
			{
				namespace data = internals::spad_info;
				// Check marker and act accordingly
				switch (marker)
				{
					case data::MARKER_OVERWRITE_REG_DEVICE_STROBE:
					read_.get(forced_value_);
					forced_value_ |= data::REG_DEVICE_STROBE_FORCED_VALUE;
					change_value_ = true;
					break;

					case data::MARKER_READ_SPAD_INFO:
					read_.get(info_);
					break;

					default:
					// Error: unexpected marker
					return PARENT::check_error(errors::EILSEQ);
				}
				return true;
			}

			bool process_include(uint8_t include)
			{
				if (include != internals::INCLUDE_DEVICE_STROBE_WAIT)
					// Error: unexpected include
					return PARENT::check_error(errors::EILSEQ);
				return strobe_.start(*device_);
			}
			
			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				read_.reset_(reg);
				return PARENT::check_error(
					PARENT::launch_commands(read_, {PARENT::write(), PARENT::read(0, false, stop)}));
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only two kinds of write here (1 or 2 bytes)
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					if (change_value_)
						value = forced_value_;
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					uint8_t val1 = this->next_byte();
					uint8_t val2 = this->next_byte();
					write2_.reset_({reg, val1, val2});
					return PARENT::check_error(
						PARENT::launch_commands(write2_, {PARENT::write(0, false, stop)}));
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				change_value_ = false;
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					if (!process_marker(this->next_byte()))
						return false;
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::INCLUDE)
					return process_include(this->next_byte());
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// It is impossible to fall here as all ProcessAction values have been tested before
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			// The device that uses this future
			DEVICE* device_ = nullptr;

			// SPAD info once read from device
			uint8_t info_ = 0;

			bool change_value_ = false;
			uint8_t forced_value_ = 0x00;

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ1 read_{0, this};
			DeviceStrobeWaitFuture strobe_{this};

			friend DEVICE;
			friend Futures;
		};

		class GetSequenceStepsTimeoutFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetSequenceStepsTimeoutFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(vl53l0x::SequenceStepsTimeout& timeouts)
			{
				if (this->await() != future::FutureStatus::READY)
					return false;
				uint8_t pre_range_vcsel_period_pclks = 0;
				readVcselPeriodPreRange_.get(pre_range_vcsel_period_pclks);
				uint8_t final_range_vcsel_period_pclks = 0;
				readVcselPeriodFinalRange_.get(final_range_vcsel_period_pclks);
				uint8_t msrc_dss_tcc_mclks = 0;
				readMsrcTimeout_.get(msrc_dss_tcc_mclks);
				uint16_t pre_range_mclks = 0;
				readPreRangeTimeout_.get(pre_range_mclks);
				uint16_t final_range_mclks = 0;
				readFinalRangeTimeout_.get(final_range_mclks);
				timeouts = vl53l0x::SequenceStepsTimeout{
					pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks,
					msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks};
				return true;
			}

		private:
			// Actual futures  embedded in this group
			TReadRegisterFuture<uint8_t(vl53l0x::VcselPeriodType::PRE_RANGE)> readVcselPeriodPreRange_{this};
			TReadRegisterFuture<uint8_t(vl53l0x::VcselPeriodType::FINAL_RANGE)> readVcselPeriodFinalRange_{this};
			TReadRegisterFuture<regs::REG_MSRC_CONFIG_TIMEOUT_MACROP> readMsrcTimeout_{this};
			TReadRegisterFuture<regs::REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t> readPreRangeTimeout_{this};
			TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t> readFinalRangeTimeout_{this};
			static constexpr uint8_t NUM_FUTURES = 5;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&readVcselPeriodPreRange_,
				&readVcselPeriodFinalRange_,
				&readMsrcTimeout_,
				&readPreRangeTimeout_,
				&readFinalRangeTimeout_
			};

			friend DEVICE;
			friend Futures;
		};

		class GetMeasurementTimingBudgetFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetMeasurementTimingBudgetFuture()  : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(uint32_t& budget_us)
			{
				if (this->await() != future::FutureStatus::READY)
					return false;
				// Get steps and timeouts
				vl53l0x::SequenceSteps steps{};
				getSequenceSteps_.get(steps);
				vl53l0x::SequenceStepsTimeout timeouts{};
				getSequenceTimeouts_.get(timeouts);
				// Calculate timing budget
				budget_us = 
					TimingBudgetUtilities::calculate_measurement_timing_budget_us(steps, timeouts);
				return true;
			}

		private:
			GetSequenceStepsFuture getSequenceSteps_{};
			GetSequenceStepsTimeoutFuture getSequenceTimeouts_{};

			static constexpr uint8_t NUM_FUTURES = 2;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&getSequenceSteps_,
				&getSequenceTimeouts_
			};

			friend DEVICE;
			friend Futures;
		};

		class SetMeasurementTimingBudgetFuture : public AbstractI2CFuturesGroup
		{
			using PARENT = AbstractI2CFuturesGroup;
		public:
			SetMeasurementTimingBudgetFuture(uint32_t budget_us, FUTURE_STATUS_LISTENER* listener = nullptr)
				:	PARENT{listener}, budget_us_{budget_us}
			{
				PARENT::init({&getSequenceSteps_, &getSequenceTimeouts_, &writeBudget_});
			}

		protected:
			bool start(DEVICE& device)
			{
				PARENT::set_device(device);
				return PARENT::check_error(PARENT::launch_commands(getSequenceSteps_, {PARENT::write()}));
			}

			void reset_(uint32_t budget_us)
			{
				budget_us_ = budget_us;
				PARENT::reset_(nullptr, 0, nullptr, 0);
			}

		private:
			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				if (status != future::FutureStatus::READY) return;
				if (&future == &getSequenceSteps_)
				{
					PARENT::check_error(PARENT::launch_commands(getSequenceTimeouts_, {PARENT::write()}));
				}
				else if (&future == &getSequenceTimeouts_)
				{
					// Calculate timing budget
					vl53l0x::SequenceSteps steps{};
					getSequenceSteps_.get(steps);
					vl53l0x::SequenceStepsTimeout timeouts{};
					getSequenceTimeouts_.get(timeouts);
					uint16_t budget = 
						TimingBudgetUtilities::calculate_final_range_timeout_mclks(steps, timeouts, budget_us_);
					writeBudget_.reset_(budget);
					PARENT::check_error(PARENT::launch_commands(writeBudget_, {PARENT::write(0, false, true)}));
				}
			}

			uint32_t budget_us_;
			GetSequenceStepsFuture getSequenceSteps_{};
			GetSequenceStepsTimeoutFuture getSequenceTimeouts_{};
			TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t> writeBudget_{0};

			friend DEVICE;
			friend Futures;
		};

		class GetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetGPIOSettingsFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(vl53l0x::GPIOSettings& settings)
			{
				if (this->await() != future::FutureStatus::READY)
					return false;
				vl53l0x::GPIOFunction function = vl53l0x::GPIOFunction::DISABLED;
				read_config_.get(function);
				uint8_t active_high = 0;
				read_GPIO_active_high_.get(active_high);
				uint16_t low_threshold = 0;
				read_low_threshold_.get(low_threshold);
				uint16_t high_threshold = 0;
				read_high_threshold_.get(high_threshold);
				settings = vl53l0x::GPIOSettings{function, bool(active_high & 0x10), low_threshold, high_threshold};
				return true;
			}

		private:
			TReadRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> read_config_{};
			TReadRegisterFuture<regs::REG_GPIO_HV_MUX_ACTIVE_HIGH> read_GPIO_active_high_{};
			TReadRegisterFuture<regs::REG_SYSTEM_THRESH_LOW, uint16_t> read_low_threshold_{};
			TReadRegisterFuture<regs::REG_SYSTEM_THRESH_HIGH, uint16_t> read_high_threshold_{};

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&read_config_,
				&read_GPIO_active_high_,
				&read_low_threshold_,
				&read_high_threshold_
			};

			friend DEVICE;
			friend Futures;
		};

		class SetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			SetGPIOSettingsFuture(const vl53l0x::GPIOSettings& settings)
				:	PARENT{futures_, NUM_FUTURES},
					write_config_{settings.function()},
					write_GPIO_active_high_{uint8_t(settings.high_polarity() ? 0x10 : 0x00)},
					write_low_threshold_{settings.low_threshold()},
					write_high_threshold_{settings.high_threshold()}
			{
				PARENT::init(futures_);
			}

		private:
			TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> write_config_;
			TWriteRegisterFuture<regs::REG_GPIO_HV_MUX_ACTIVE_HIGH> write_GPIO_active_high_;
			TWriteRegisterFuture<regs::REG_SYSTEM_THRESH_LOW, uint16_t> write_low_threshold_;
			TWriteRegisterFuture<regs::REG_SYSTEM_THRESH_HIGH, uint16_t> write_high_threshold_;

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&write_config_,
				&write_GPIO_active_high_,
				&write_low_threshold_,
				&write_high_threshold_
			};

			friend DEVICE;
			friend Futures;
		};

		//FIXME need to specify some value to clear interrupt! Not that simple?
		class ClearInterruptFuture : public TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>
		{
			using PARENT = TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>;
		public:
			ClearInterruptFuture() : PARENT{0x01} {}
		};

		//TODO additional arguments for init? eg sequence steps, limits...
		class InitDataFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;
			
		public:
			InitDataFuture() : PARENT{uint16_t(internals::init_data::BUFFER)}
			{
				PARENT::init({&write1_, &write2_, &read_}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

		protected:
			bool start(DEVICE& device)
			{
				PARENT::set_device(device);
				device_ = &device;
				return next_future();
			}

		private:
			void process_marker(uint8_t marker)
			{
				namespace data = internals::init_data;
				// Check marker and act accordingly
				switch (marker)
				{
					case data::MARKER_VHV_CONFIG:
					read_.get(forced_value_);
					forced_value_ |= data::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8;
					change_value_ = true;
					break;

					case data::MARKER_STOP_VARIABLE:
					read_.get(device_->stop_variable_);
					break;

					case data::MARKER_MSRC_CONFIG_CONTROL:
					read_.get(forced_value_);
					forced_value_ |= data::MSRC_CONFIG_CONTROL_INIT;
					change_value_ = true;
					break;

					default:
					//TODO ERROR
					break;
				}
			}

			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				read_.reset_(reg);
				return PARENT::check_error(
					PARENT::launch_commands(read_, {PARENT::write(), PARENT::read(0, false, stop)}));
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only two kinds of write here (1 or 2 bytes)
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					if (change_value_)
						value = forced_value_;
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					uint8_t val1 = this->next_byte();
					uint8_t val2 = this->next_byte();
					write2_.reset_({reg, val1, val2});
					return PARENT::check_error(
						PARENT::launch_commands(write2_, {PARENT::write(0, false, stop)}));
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				change_value_ = false;
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					process_marker(this->next_byte());
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// If we fall here , this is an error (no ProcessAction::INCLUDE in this group)
				//TODO error
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			// The device that uses this future
			DEVICE* device_ = nullptr;

			bool change_value_ = false;
			uint8_t forced_value_ = 0x00;

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ1 read_{0, this};

			friend DEVICE;
			friend Futures;
		};

		class LoadTuningSettingsFuture : public I2CSameFutureGroup
		{
			using PARENT = I2CSameFutureGroup;
		public:
			LoadTuningSettingsFuture(FUTURE_STATUS_LISTENER* status_listener = nullptr)
				: PARENT{	uint16_t(internals::load_tuning_settings::BUFFER), 
							internals::load_tuning_settings::BUFFER_SIZE, status_listener} {}

			friend DEVICE;
			friend Futures;
		};

		static constexpr const uint8_t NUM_REF_SPADS = 48;
		static constexpr const uint8_t SPADS_PER_BYTE = 8;
		static constexpr const uint8_t NUM_REF_SPADS_BYTES = NUM_REF_SPADS / SPADS_PER_BYTE;
		
		class InitStaticFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;

		public:
			InitStaticFuture() : PARENT{uint16_t(internals::init_static::BUFFER)}
			{
				PARENT::init({
					&write1_, &write6_, 
					&read1_, &read6_, 
					&getSpadInfo_, &setGPIOSettings_, &loadTuningSettings_,
					&getTimingBudget_, &setTimingBudget_
				}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

		protected:
			bool start(DEVICE& device)
			{
				PARENT::set_device(device);
				return next_future();
			}

			DEVICE& device()
			{
				return (DEVICE&) PARENT::device();
			}

		private:
			bool process_include(uint8_t include)
			{
				switch (include)
				{
					case internals::INCLUDE_GET_SPAD_INFO:
					return getSpadInfo_.start(device());

					case internals::INCLUDE_LOAD_TUNING_SETTINGS:
					return loadTuningSettings_.start(device());

					case internals::INCLUDE_SET_GPIO_SETTINGS:
					return setGPIOSettings_.start(device());

					case internals::INCLUDE_GET_MEASUREMENT_TIMING:
					return getTimingBudget_.start(device());

					case internals::INCLUDE_SET_MEASUREMENT_TIMING:
					{
						//FIXME reset_() does not work here probably
						uint32_t budget_us = 0;
						getTimingBudget_.get(budget_us);
						setTimingBudget_.reset_(budget_us);
					}
					return setTimingBudget_.start(device());

					default:
					// Error: unexpected include
					return PARENT::check_error(errors::EILSEQ);
				}
			}
			
			bool process_marker(uint8_t marker)
			{
				namespace data = internals::init_static;
				// Check marker and act accordingly
				if (marker == data::MARKER_GET_REFERENCE_SPADS)
				{
					// Get SPAD info
					vl53l0x::SPADInfo info{};
					getSpadInfo_.get(info);
					// Get NVM ref SPADs
					read6_.get(ref_spads_);
					// Calculate reference spads
					calculate_reference_SPADs(ref_spads_.data(), info);
				}
				else
				{
					// Error: unexpected marker
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				if (count == 1)
				{
					read1_.reset_(reg);
					return PARENT::check_error(
						PARENT::launch_commands(read1_, {PARENT::write(), PARENT::read(0, false, stop)}));
				}
				else if (count == 6)
				{
					read6_.reset_(reg);
					return PARENT::check_error(
						PARENT::launch_commands(read6_, {PARENT::write(), PARENT::read(0, false, stop)}));
				}
				else
				{
					// Error: unexpected read
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else if (count == 6)
				{
					// Skip 6 bytes
					for (uint8_t i = 0; i < 6; ++i)
						this->next_byte();
					// Replace with 6 bytes obtained before
					typename FUTURE_WRITE<6>::IN input{reg};
					input.template set<NUM_REF_SPADS_BYTES>(1, ref_spads_.data());
					write6_.reset_(input);
					return PARENT::check_error(
						PARENT::launch_commands(write6_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					// Error: unexpected read
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					if (!process_marker(this->next_byte()))
						return false;
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::INCLUDE)
					return process_include(this->next_byte());
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// It is impossible to fall here as all ProcessAction values have been tested before
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			containers::array<uint8_t, NUM_REF_SPADS_BYTES> ref_spads_{};

			// Futures included in this one
			GetSPADInfoFuture getSpadInfo_{};
			LoadTuningSettingsFuture loadTuningSettings_{};
			SetGPIOSettingsFuture setGPIOSettings_{vl53l0x::GPIOSettings::sample_ready()};
			GetMeasurementTimingBudgetFuture getTimingBudget_{};
			SetMeasurementTimingBudgetFuture setTimingBudget_{0};
			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{};
			FUTURE_WRITE<2> write2_{};
			FUTURE_WRITE<6> write6_{};
			FUTURE_READ1 read1_{};
			FUTURE_READ<6> read6_{};

			friend DEVICE;
			friend Futures;
		};

	private:
		static constexpr const uint8_t FIRST_APERTURE_SPAD = 12;

		static void calculate_reference_SPADs(uint8_t ref_spads[NUM_REF_SPADS_BYTES], vl53l0x::SPADInfo info)
		{
			const uint8_t count = info.count();
			const uint8_t first_spad = (info.is_aperture() ? FIRST_APERTURE_SPAD : 0);
			uint8_t enabled_spads = 0;
			uint8_t spad = 0;
			for (uint8_t i = 0; i < NUM_REF_SPADS_BYTES; ++i)
			{
				uint8_t& ref_spad = ref_spads[i];
				for (uint8_t j = 0; j < SPADS_PER_BYTE; ++j)
				{
					if ((spad < first_spad) || (enabled_spads == count))
						// Disable this SPAD as it should not be enabled
						ref_spad &= bits::CBV8(j);
					else if (ref_spad & bits::BV8(j))
						// Just count the current SPAD as enabled
						++enabled_spads;
					++spad;
				}
			}
		}
	};
}

#endif /* VL53L0X_FUTURES_H */
/// @endcond
