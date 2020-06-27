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

#include "future_commons.h"

namespace future
{
	/// @cond notdocumented
	// Add utility ostream manipulator for FutureStatus
	static const flash::FlashStorage* convert(future::FutureStatus s)
	{
		switch (s)
		{
			case future::FutureStatus::INVALID:
			return F("INVALID");

			case future::FutureStatus::NOT_READY:
			return F("NOT_READY");

			case future::FutureStatus::READY:
			return F("READY");

			case future::FutureStatus::ERROR:
			return F("ERROR");
		}
	}

	streams::ostream& operator<<(streams::ostream& out, future::FutureStatus s)
	{
		return out << convert(s);
	}
	/// @endcond
}
