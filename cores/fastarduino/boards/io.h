//   Copyright 2016-2018 Jean-Francois Poilpret
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

// This file wraps "avr/io.h" header, in order to override all _SFR_XXX() macros
// to allow usage of all defines (such as PINB) to work in constexpr evalutations.

#ifndef BOARDS_IO_HH
#define BOARDS_IO_HH

#include <avr/io.h>

// Override all _SFR_XXX() macros to simply return a const integer value
#ifdef _SFR_IO8
#undef _SFR_IO8
#endif
#define _SFR_IO8(x) ((x) + __SFR_OFFSET)

#ifdef _SFR_IO16
#undef _SFR_IO16
#endif
#define _SFR_IO16(x) ((x) + __SFR_OFFSET)

#ifdef _SFR_MEM8
#undef _SFR_MEM8
#endif
#define _SFR_MEM8(x) (x)

#ifdef _SFR_MEM16
#undef _SFR_MEM16
#endif
#define _SFR_MEM16(x) (x)

// Force SREG which is used by <util/atomic.h>
#ifdef SREG
#undef SREG
#define SREG (*((volatile uint8_t*) (0x3F + __SFR_OFFSET)))
#endif

#endif /* BOARDS_IO_HH */
