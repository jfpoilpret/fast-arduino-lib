//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * I2C status hook utilities.
 */
#ifndef I2C_STATUS_HH
#define I2C_STATUS_HH

#include "flash.h"
#include "ios.h"
#include "i2c_handler_common.h"

namespace i2c
{
	/**
	 * Defines API to ease I2C manager status tracing and debugging.
	 * @sa i2c::Status
	 */
	namespace status
	{
	}
}

namespace i2c::status
{
	/**
	 * Indicate when status should be traced.
	 * @sa I2CStatusRecorder
	 * @sa I2CStatusDebugger
	 */
	enum class STATUS : uint8_t
	{
		/** Trace only status that differ (between expected and actual). */
		TRACE_ERROR = 0x01,
		/** Trace everything. */
		TRACE_ALL = 0xFF
	};

	/**
	 * Class recording I2C status notifications for later output.
	 * 
	 * @tparam SIZE the maximum number of notifications to record (each notification
	 * is 2 bytes)
	 */
	template<uint8_t SIZE> class I2CStatusRecorder
	{
	public:
		/**
		 * Create an I2CStatusRecorder that can record I2C status notifications
		 * determined by @p trace. list.
		 * The number of recorded notifications is limited by @p SIZE. Once @p SIZE
		 * notifications have been recorded by this I2CStatusRecorder, any additional
		 * notification will be trashed.
		 * To be effective, this must be attached to an I2C Manager (at construction time).
		 * Recorded notifications can be output to a `streams::ostream` with `trace()`.
		 * 
		 * @param trace the list of notifications to be recorded
		 * 
		 * @sa trace()
		 * @sa reset()
		 */
		explicit I2CStatusRecorder(STATUS trace = STATUS::TRACE_ALL) : trace_{trace} {}

		/**
		 * Clear all recorded notifications.
		 * @sa trace()
		 */
		void reset()
		{
			index_ = 0;
		}

		/**
		 * Output all recorded I2C status notifications to @p out then clear all records.
		 * @param out the `streams::ostream` to output traces to
		 * @param hex_status if `true` (the default), status values will be displayed in 
		 * hexadecimal, otherwise status name will be displayed 
		 * @sa reset()
		 */
		template<typename OSTREAM>
		void trace(OSTREAM& out, bool hex_status = true)
		{
			if (hex_status)
				for (uint8_t i = 0; i < index_; ++i)
				{
					out.setf(streams::ios::hex, streams::ios::basefield);
					out << uint8_t(expected_[i]) << ' ';
					out.setf(streams::ios::hex, streams::ios::basefield);
					out << uint8_t(actual_[i]) << '\n';
				}
			else
				for (uint8_t i = 0; i < index_; ++i)
					out << expected_[i] << ' ' << actual_[i] << '\n';
			if (index_ >= SIZE)
				out << F("# OVF #") << '\n';
			index_ = 0;
		}

		/// @cond notdocumented
		void operator()(Status expected, Status actual)
		{
			if (index_ >= SIZE) return;
			if ((expected != actual) || (trace_ == STATUS::TRACE_ALL))
			{
				expected_[index_] = expected;
				actual_[index_++] = actual;
			}
		}
		/// @endcond

	private:
		Status expected_[SIZE];
		Status actual_[SIZE];
		uint8_t index_ = 0;
		STATUS trace_;
	};

	/**
	 * Class tracing I2C status notifications live to @p out.
	 * @warning Do not use this with asynchronous (ISR-based) I2C Managers! if you
	 * use an asynchronous I2C Manager, then use I2CStatusRecorder instead.
	 * 
	 * @sa I2CStatusRecorder
	 */
	template<typename OSTREAM>
	class I2CStatusLiveLogger
	{
	public:
		/**
		 * Create an I2CStatusLiveLogger that can trace live I2C notifications determined
		 * by @p trace. list. I2C notifications are output to @p out.
		 * 
		 * @param out the `streams::ostream` to output traces to
		 * @param trace the list of notifications to trace
		 * @param hex_status if `true` (the default), status values will be displayed in 
		 * hexadecimal, otherwise status name will be displayed 
		 */
		I2CStatusLiveLogger(OSTREAM& out, STATUS trace = STATUS::TRACE_ALL, bool hex_status = true)
		: out_{out}, trace_{trace}, hex_status_{hex_status} {}

		/// @cond notdocumented
		void operator()(Status expected, Status actual)
		{
			if ((expected != actual) || (trace_ == STATUS::TRACE_ALL))
			{
				if (hex_status_)
				{
					out_.setf(streams::ios::hex, streams::ios::basefield);
					out_ << uint8_t(expected) << ' ';
					out_.setf(streams::ios::hex, streams::ios::basefield);
					out_ << uint8_t(actual) << '\n';
				}
				else
					out_ << expected << ' ' << actual << '\n';
			}
		}
		/// @endcond

	private:
		OSTREAM& out_;
		STATUS trace_;
		bool hex_status_;
	};

	/**
	 * Class holding the latest I2C status.
	 * 
	 * @sa Status
	 */
	class I2CLatestStatusHolder
	{
	public:
		/**
		 * Create an I2CLatestStatusHolder that can hold latest I2C status.
		 */
		I2CLatestStatusHolder() = default;

		/**
		 * Return the latest I2C actual status.
		 */
		Status latest_status() const
		{
			return actual_;
		}

		/**
		 * Return the latest I2C expected status (may be different than actual).
		 */
		Status latest_expected_status() const
		{
			return expected_;
		}

		/// @cond notdocumented
		void operator()(Status expected, Status actual)
		{
			expected_ = expected;
			actual_ = actual;
		}
		/// @endcond

	private:
		Status actual_ = Status::OK;
		Status expected_ = Status::OK;
	};
}

#endif /* I2C_STATUS_HH */
/// @endcond
