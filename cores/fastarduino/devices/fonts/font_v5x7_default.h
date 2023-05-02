//   Copyright 2016-2023 Jean-Francois Poilpret
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

#ifndef FONT_V5X7_DEFAULT_HH
#define FONT_V5X7_DEFAULT_HH

#include "../font.h"

namespace devices::display
{
	class DefaultVerticalFont7x5 : public Font<true>
	{
	public:
		DefaultVerticalFont7x5() : Font{0x20, 0x7e, 5, 7, FONT} {}

	private:
		static const uint8_t FONT[] PROGMEM;
	};
}
#endif /* FONT_V5X7_DEFAULT_HH */

/// @endcond
