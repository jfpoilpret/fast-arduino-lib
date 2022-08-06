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

#include "i2c_debug.h"

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

	DEBUG operator|(DEBUG a, DEBUG b)
	{
		return DEBUG(uint8_t(a) | uint8_t(b));
	}
	/// @endcond
}
