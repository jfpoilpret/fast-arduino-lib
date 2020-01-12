//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * Small utilities to emulate `virtual` methods without extra *vtabl*. This is not
 * as practical as true `virtual` methods but it is useful in simple situations.
 * These utilities are normally for FastArduino internal usage.
 */
#ifndef VIRTUAL_HH
#define VIRTUAL_HH

#include <stddef.h>
#include <stdint.h>

/**
 * Defines utility types for emulating `virtual` methods. These are dedicated to
 * FastArduino internal use (for code size and speed optimization).
 * @sa streams::ostreambuf
 */
namespace virtual_support
{
	/**
	 * Holder of a "virtual method".
	 * Typical use:
	 * @code
	 * class A {
	 * public:
	 *     A(VirtualMethod::METHOD callback, void* arg) : callback_{callback, arg} {}
	 * 
	 *     void call_back() {
	 *         callback_();
	 *     }
	 * private:
	 *     VirtualMethod callback_;
	 * }
	 * 
	 * class B {
	 * public:
	 *     B() : a{B::callback, this} {}
	 * private:
	 *     // this is the actual "virtual" method
	 *     void do_something() {
	 *         ...
	 *     }
	 * 
	 *     // this method is in hcarge of dispatching to the actual "virtual" method
	 *     static void callback(void* arg) {
	 *         ((B*) arg)->do_something();
	 *     }
	 * 
	 *     A a;
	 * }
	 * @endcode
	 */
	class VirtualMethod
	{
	public:
		VirtualMethod(const VirtualMethod&) = default;
		VirtualMethod& operator=(const VirtualMethod&) = default;
		
		/**
		 * The type of function that will get the call.
		 */
		using METHOD = void (*)(void*);

		/**
		 * Create a `VirtualMethod` with the given arguments.
		 * @param method the method that will receive the call and shall dispatch
		 * to the proper method of the proper object, which pointer is given by @p arg
		 * @param arg a pointer to the object that should receive the actual dispatch
		 */
		explicit VirtualMethod(METHOD method = nullptr, void* arg = nullptr): method_{method}, arg_{arg} {}

		/**
		 * Call dispatching method.
		 */
		void operator()() const
		{
			if (method_ != nullptr)
				method_(arg_);
		}
	
	private:
		const METHOD method_;
		void* const arg_;
	};

}

#endif /* VIRTUAL_HH */
/// @endcond
