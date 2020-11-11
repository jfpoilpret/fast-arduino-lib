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

/// @cond notdocumented
#ifndef TESTS_ASSERTIONS_H
#define TESTS_ASSERTIONS_H

#include "../flash.h"
#include "../iomanip.h"
#include "../streams.h"

#define ASSERT(OUT, CONDITION) tests::assert_true(OUT, F("" #CONDITION ""), CONDITION)

namespace tests
{
	void assert_true(streams::ostream& out, const char* message, bool condition)
	{
		if (!condition)
			out << F("ASSERTION FAILED: ") << message << streams::endl;
	}

	void assert_true(streams::ostream& out, const flash::FlashStorage* message, bool condition)
	{
		if (!condition)
			out << F("ASSERTION FAILED: ") << message << streams::endl;
	}

	template<typename T1, typename T2>
	void assert_equals(streams::ostream& out, const char* var, T1 expected, T2 actual)
	{
		if (expected != actual)
			out << F("ASSERTION FAILED on ") << var 
				<< F(": expected = ") << expected << F(", actual=") << actual << streams::endl;
	}

	template<typename T1, typename T2>
	void assert_equals(streams::ostream& out, const flash::FlashStorage* var, T1 expected, T2 actual)
	{
		if (expected != actual)
			out << F("ASSERTION FAILED on ") << var 
				<< F(": expected = ") << expected << F(", actual=") << actual << streams::endl;
	}
}
#endif /* TESTS_ASSERTIONS_H */
/// @endcond
