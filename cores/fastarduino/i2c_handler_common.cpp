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

#include "i2c_handler_common.h"

namespace i2c
{
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, const I2CCommandType& t)
	{
		if (t.is_none()) return out << F("NONE") << streams::flush;
		out << (t.is_write() ? F("WRITE") : F("READ"));
		if (t.is_stop())
			out << F("[STOP]");
		if (t.is_finish())
			out << F("[FINISH]");
		if (t.is_end())
			out << F("[END]");
		return out << streams::flush;
	}

	bool operator==(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ == b.value_);
	}

	bool operator!=(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ != b.value_);
	}
	/// @endcond
}
