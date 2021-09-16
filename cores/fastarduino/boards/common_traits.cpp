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

#include "common_traits.h"

// The following definitions seem necessary for linking some examples, not sure why
namespace board_traits
{
	using TIMER_PRESCALERS_TRAIT1 = TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_64_256_1024>;
	constexpr const TIMER_PRESCALERS_TRAIT1::TYPE TIMER_PRESCALERS_TRAIT1::ALL_PRESCALERS[];

	using TIMER_PRESCALERS_TRAIT2 = TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024>;
	constexpr const TIMER_PRESCALERS_TRAIT2::TYPE TIMER_PRESCALERS_TRAIT2::ALL_PRESCALERS[];

	using TIMER_PRESCALERS_TRAIT3 = TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_TO_16384>;
	constexpr const TIMER_PRESCALERS_TRAIT3::TYPE TIMER_PRESCALERS_TRAIT3::ALL_PRESCALERS[];
}
