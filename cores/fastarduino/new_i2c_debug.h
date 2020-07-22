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
	//TODO DOC
	namespace debug
	{
	}
}

namespace i2c::debug
{
	/// @cond notdocumented
	// Add utility ostream manipulator for FutureStatus
	static const flash::FlashStorage* convert(i2c::DebugStatus s)
	{
		switch (s)
		{
			case i2c::DebugStatus::START:
			return F("ST ");

			case i2c::DebugStatus::REPEAT_START:
			return F("RS ");

			case i2c::DebugStatus::SLAW:
			return F("AW ");

			case i2c::DebugStatus::SLAR:
			return F("AR ");

			case i2c::DebugStatus::SEND:
			return F("S ");

			case i2c::DebugStatus::RECV:
			return F("R ");

			case i2c::DebugStatus::RECV_LAST:
			return F("RL ");

			case i2c::DebugStatus::STOP:
			return F("SP ");

			case i2c::DebugStatus::SEND_OK:
			return F("So ");

			case i2c::DebugStatus::SEND_ERROR:
			return F("Se ");

			case i2c::DebugStatus::RECV_OK:
			return F("Ro ");

			case i2c::DebugStatus::RECV_ERROR:
			return F("Re ");
		}
	}

	streams::ostream& operator<<(streams::ostream& out, i2c::DebugStatus s)
	{
		return out << convert(s);
	}
	/// @endcond

	//TODO DOC
	enum class DEBUG : uint8_t
	{
		DEBUG_STEPS = 0x01,
		DEBUG_SEND_OK = 0x02,
		DEBUG_SEND_ERR = 0x04,
		DEBUG_RECV_OK = 0x08,
		DEBUG_RECV_ERR = 0x10,
		DEBUG_ALL = 0xFF
	};

	//TODO DOC
	template<uint8_t SIZE> class I2CAsyncDebugger
	{
	public:
		I2CAsyncDebugger(DEBUG debug = DEBUG::DEBUG_ALL) : debug_{debug} {}

		void reset()
		{
			index_ = 0;
		}

		void trace(streams::ostream& out)
		{
			for (uint8_t i = 0; i < index_; ++i)
				out << status_[i] << streams::hex << data_[i] << ' ' << streams::flush;
			if (index_ >= SIZE)
				out << F("# OVF #");
			out << streams::endl;
			index_ = 0;
		}

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

	private:
		i2c::DebugStatus status_[SIZE];
		uint8_t data_[SIZE];
		uint8_t index_ = 0;
		DEBUG debug_;
	};

	//TODO DOC
	class I2CSyncDebugger
	{
	public:
		I2CSyncDebugger(streams::ostream& out, DEBUG debug = DEBUG::DEBUG_ALL) : out_{out}, debug_{debug} {}

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

	private:
		streams::ostream& out_;
		DEBUG debug_;
	};
}

#endif /* I2C_DEBUG_HH */
/// @endcond
