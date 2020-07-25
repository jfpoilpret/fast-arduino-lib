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
 * I2C debugging utilities (useful when implementing support for new devices).
 */
#ifndef I2C_DEBUG_HH
#define I2C_DEBUG_HH

#include "flash.h"
#include "streams.h"
#include "new_i2c_handler_common.h"

namespace i2c
{
	/**
	 * Defines API to ease I2C devices debugging.
	 * You would normally not use API in this namespace unless you develop specific
	 * support for I2C devices.
	 */
	namespace debug
	{
	}
}

namespace i2c::debug
{
	/// @cond notdocumented
	// Add utility ostream insert operator for FutureStatus
	streams::ostream& operator<<(streams::ostream& out, i2c::DebugStatus s);
	/// @endcond

	/**
	 * Indicate what in I2C protocol shall be debugged.
	 * Values can or'ed together, e.g. `DEBUG::DEBUG_SEND_OK | DEBUG::DEBUG_SEND_ERR`.
	 * @sa I2CDebugRecorder
	 * @sa I2CLiveDebugger
	 */
	enum class DEBUG : uint8_t
	{
		/**
		 * Debug all individual steps of I2C protocol:
		 * - START condition
		 * - REPEAT START condition
		 * - SLAW slave address for writing
		 * - SLAR slave address for reading
		 * - SEND send byte to slave
		 * - RECV receive byte from slave
		 * - RECV_LAST receive last byte from slave
		 * - STOP condition
		 */
		DEBUG_STEPS = 0x01,
		/** Debug successfully written bytes. */
		DEBUG_SEND_OK = 0x02,
		/** Debug written bytes not acknowledged by slave. */
		DEBUG_SEND_ERR = 0x04,
		/** Debug successfully received bytes. */
		DEBUG_RECV_OK = 0x08,
		/** Debug error during receiving bytes. */
		DEBUG_RECV_ERR = 0x10,
		/** Debug everything. */
		DEBUG_ALL = 0xFF
	};

	/// @cond notdocumented
	DEBUG operator|(DEBUG a, DEBUG b)
	{
		return DEBUG(uint8_t(a) | uint8_t(b));
	}
	/// @endcond

	/**
	 * Class recording I2C debug notifications for later output.
	 * 
	 * @tparam SIZE the maximum number of notifications to record (each notification
	 * is 2 bytes)
	 * 
	 * @sa i2c::DebugStatus
	 */
	template<uint8_t SIZE> class I2CDebugRecorder
	{
	public:
		/**
		 * Create an I2CDebugRecorder that can record I2C notifications determined
		 * by @p debug list.
		 * The number of recorded notifications is limited by @p SIZE. Once @p SIZE
		 * notifications have been recorded by this I2CDebugRecorder, any additional
		 * notification will be trashed.
		 * To be effective, this must be attached to an I2CManager (at construction time).
		 * Recorded notifications can be output to a `streams::ostream` with `trace()`.
		 * 
		 * @param debug the list fo notifications to be recorded
		 * 
		 * @sa trace()
		 * @sa reset()
		 */
		I2CDebugRecorder(DEBUG debug = DEBUG::DEBUG_ALL) : debug_{debug} {}

		/**
		 * Clear all recorded notifications.
		 * @sa trace()
		 */
		void reset()
		{
			index_ = 0;
		}

		/**
		 * Output all recorded I2C notifications to @p out then clear all records.
		 * @param out the `streams::ostream` to output traces to
		 * @sa reset()
		 */
		void trace(streams::ostream& out)
		{
			for (uint8_t i = 0; i < index_; ++i)
				out << status_[i] << streams::hex << data_[i] << ' ' << streams::flush;
			if (index_ >= SIZE)
				out << F("# OVF #");
			out << streams::endl;
			index_ = 0;
		}

		/// @cond notdocumented
		void operator()(i2c::DebugStatus status, uint8_t data)
		{
			if (index_ >= SIZE) return;
			switch (status)
			{
				case i2c::DebugStatus::START:
				case i2c::DebugStatus::REPEAT_START:
				case i2c::DebugStatus::STOP:
				case i2c::DebugStatus::SLAW:
				case i2c::DebugStatus::SLAR:
				case i2c::DebugStatus::SEND:
				case i2c::DebugStatus::RECV:
				case i2c::DebugStatus::RECV_LAST:
				if (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_STEPS))
				{
					status_[index_] = status;
					data_[index_++] = data;
				}
				break;

				case i2c::DebugStatus::SEND_OK:
				if (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_SEND_OK))
				{
					status_[index_] = status;
					data_[index_++] = data;
				}
				break;

				case i2c::DebugStatus::SEND_ERROR:
				if (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_SEND_ERR))
				{
					status_[index_] = status;
					data_[index_++] = data;
				}
				break;

				case i2c::DebugStatus::RECV_OK:
				if (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_RECV_OK))
				{
					status_[index_] = status;
					data_[index_++] = data;
				}
				break;

				case i2c::DebugStatus::RECV_ERROR:
				if (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_RECV_ERR))
				{
					status_[index_] = status;
					data_[index_++] = data;
				}
				break;
			}
		}
		/// @endcond

	private:
		i2c::DebugStatus status_[SIZE];
		uint8_t data_[SIZE];
		uint8_t index_ = 0;
		DEBUG debug_;
	};

	/**
	 * Class tracing I2C debug notifications live to @p out.
	 * @warning Do not use this with asynchronous (ISR-based) I2CManagers! if you
	 * use an asynchronous I2CManager, then use I2CDebugRecorder instead.
	 * 
	 * @sa i2c::DebugStatus
	 * @sa I2CDebugRecorder
	 */
	class I2CLiveDebugger
	{
	public:
		/**
		 * Create an I2CLiveDebugger that can trace live I2C notifications determined
		 * by @p debug list. I2C notifications are output to @p out.
		 * 
		 * @param out the `streams::ostream` to output traces to
		 * @param debug the list fo notifications to trace
		 */
		I2CLiveDebugger(streams::ostream& out, DEBUG debug = DEBUG::DEBUG_ALL) : out_{out}, debug_{debug} {}

		/// @cond notdocumented
		void operator()(i2c::DebugStatus status, uint8_t data)
		{
			bool display = false;
			switch (status)
			{
				case i2c::DebugStatus::START:
				case i2c::DebugStatus::REPEAT_START:
				case i2c::DebugStatus::STOP:
				case i2c::DebugStatus::SLAW:
				case i2c::DebugStatus::SLAR:
				case i2c::DebugStatus::SEND:
				case i2c::DebugStatus::RECV:
				case i2c::DebugStatus::RECV_LAST:
				display = (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_STEPS));
				break;

				case i2c::DebugStatus::SEND_OK:
				display = (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_SEND_OK));
				break;

				case i2c::DebugStatus::SEND_ERROR:
				display = (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_SEND_ERR));
				break;

				case i2c::DebugStatus::RECV_OK:
				display = (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_RECV_OK));
				break;

				case i2c::DebugStatus::RECV_ERROR:
				display = (uint8_t(debug_) & uint8_t(DEBUG::DEBUG_RECV_ERR));
				break;
			}
			if (display)
				out_ << status << streams::hex << data << ' ' << streams::flush;
		}
		/// @endcond

	private:
		streams::ostream& out_;
		DEBUG debug_;
	};
}

#endif /* I2C_DEBUG_HH */
/// @endcond
