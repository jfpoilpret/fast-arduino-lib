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

#include "defines.h"

// Interesting and complete description here (C++ wise, not AVR specific)
// https://itanium-cxx-abi.github.io/cxx-abi/abi.html
// Default GCC 11.1.0 ABI header
// https://gcc.gnu.org/onlinedocs/gcc-11.1.0/libstdc++/api/a00026.html
namespace __cxxabiv1
{
	typedef int __guard;

	void* __dso_handle = nullptr;

	extern "C"
	{
		// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#guards
		// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#once-ctor
		int __cxa_guard_acquire(__guard* g UNUSED)
		{
			return 0;
		}

		void __cxa_guard_release(__guard* g UNUSED) {}

		void __cxa_guard_abort(__guard* g UNUSED) {}

		// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#pure-virtual
		void __cxa_pure_virtual(void) {}

		// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor
		int __cxa_atexit(void (*destructor)(void*) UNUSED, void* arg UNUSED, void* dso UNUSED)
		{
			return 0;
		}

		void __cxa_finalize(void* f UNUSED) {}
	}
}
