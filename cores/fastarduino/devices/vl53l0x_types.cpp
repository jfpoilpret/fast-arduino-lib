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

#include "../flash.h"
#include "../streams.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x
{
	static const flash::FlashStorage* convert(DeviceError error)
	{
		switch (error)
		{
			case DeviceError::NONE:
			return F("NONE");

			case DeviceError::VCSEL_CONTINUITY_TEST_FAILURE:
			return F("VCSEL_CONTINUITY_TEST_FAILURE");

			case DeviceError::VCSEL_WATCHDOG_TEST_FAILURE:
			return F("VCSEL_WATCHDOG_TEST_FAILURE");

			case DeviceError::NO_VHV_VALUE_FOUND:
			return F("NO_VHV_VALUE_FOUND");

			case DeviceError::MSRC_NO_TARGET:
			return F("MSRC_NO_TARGET");

			case DeviceError::SNR_CHECK:
			return F("SNR_CHECK");

			case DeviceError::RANGE_PHASE_CHECK:
			return F("RANGE_PHASE_CHECK");

			case DeviceError::SIGMA_THRESHOLD_CHECK:
			return F("SIGMA_THRESHOLD_CHECK");

			case DeviceError::TCC:
			return F("TCC");

			case DeviceError::PHASE_CONSISTENCY:
			return F("PHASE_CONSISTENCY");

			case DeviceError::MIN_CLIP:
			return F("MIN_CLIP");

			case DeviceError::RANGE_COMPLETE:
			return F("RANGE_COMPLETE");

			case DeviceError::ALGO_UNDERFLOW:
			return F("ALGO_UNDERFLOW");

			case DeviceError::ALGO_OVERFLOW:
			return F("ALGO_OVERFLOW");

			case DeviceError::RANGE_IGNORE_THRESHOLD:
			return F("RANGE_IGNORE_THRESHOLD");

			case DeviceError::UNKNOWN:
			return F("UNKNOWN");
		}
	}

	streams::ostream& operator<<(streams::ostream& out, DeviceError error)
	{
		return out << convert(error) << streams::flush;
	}

	streams::ostream& operator<<(streams::ostream& out, DeviceStatus status)
	{
		return out << '(' << status.error() << ',' << status.data_ready() << ')' << streams::flush;
	}

	static const flash::FlashStorage* convert(PowerMode mode)
	{
		switch (mode)
		{
			case PowerMode::IDLE:
			return F("IDLE");

			case PowerMode::STANDBY:
			return F("STANDBY");
		}
	}

	streams::ostream& operator<<(streams::ostream& out, PowerMode mode)
	{
		return out << convert(mode) << streams::flush;
	}

	static const flash::FlashStorage* convert(GPIOFunction function)
	{
		switch (function)
		{
			case GPIOFunction::DISABLED: 
			return F("DISABLED");

			case GPIOFunction::LEVEL_LOW: 
			return F("LEVEL_LOW");

			case GPIOFunction::LEVEL_HIGH: 
			return F("LEVEL_HIGH");

			case GPIOFunction::OUT_OF_WINDOW: 
			return F("OUT_OF_WINDOW");

			case GPIOFunction::SAMPLE_READY: 
			return F("SAMPLE_READY");
		}
	}

	streams::ostream& operator<<(streams::ostream& out, GPIOFunction function)
	{
		return out << convert(function) << streams::flush;
	}

	streams::ostream& operator<<(streams::ostream& out, const GPIOSettings& settings)
	{
		return out	<< F("(GPIO function=")
					<< streams::dec << settings.function() << streams::flush
					<< F(", ") << (settings.high_polarity() ? F("HIGH") : F("LOW")) << F(" polarity") << streams::flush
					<< F(", low_threshold=") << streams::hex << settings.low_threshold()  << streams::flush
					<< F(", high_threshold=") << streams::hex << settings.high_threshold() << ')' << streams::flush;
	}

	static streams::ostream& with_without(streams::ostream& out, bool with, const flash::FlashStorage* label)
	{
		if (!with)
			out << F("no ");
		return out << label << streams::flush;
	}

	streams::ostream& operator<<(streams::ostream& out, SequenceSteps steps)
	{
		out << '(';
		with_without(out, steps.is_tcc(), F("TCC")) << ',';
		with_without(out, steps.is_dss(), F("DSS")) << ',';
		with_without(out, steps.is_msrc(), F("MSRC")) << ',';
		with_without(out, steps.is_pre_range(), F("PRE_RANGE")) << ',';
		return with_without(out, steps.is_final_range(), F("FINAL_RANGE")) << ')' << streams::flush;
	}

	streams::ostream& operator<<(streams::ostream& out, SPADInfo info)
	{
		return out	<< F("(aperture=") << info.is_aperture()
					<< F(", count=") << streams::dec << info.count() << ')' << streams::flush;
	}

	streams::ostream& operator<<(streams::ostream& out, const SequenceStepsTimeout& timeouts)
	{
		return out	<< F("(pre_range_vcsel_period_pclks=")
					<< streams::dec << timeouts.pre_range_vcsel_period_pclks() << streams::flush
					<< F(", final_range_vcsel_period_pclks=")
					<< timeouts.final_range_vcsel_period_pclks() << streams::flush
					<< F(", msrc_dss_tcc_mclks=")
					<< timeouts.msrc_dss_tcc_mclks() << streams::flush
					<< F(", pre_range_mclks=")
					<< timeouts.pre_range_mclks() << streams::flush
					<< F(", final_range_mclks(with pre-range)=")
					<< timeouts.final_range_mclks(true) << ')' << streams::flush
					<< F(", final_range_mclks(no pre-range)=")
					<< timeouts.final_range_mclks(false) << ')' << streams::flush;
	}
}

/// @endcond
