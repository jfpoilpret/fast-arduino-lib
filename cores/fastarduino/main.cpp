//   Copyright 2016-2017 Jean-Francois Poilpret
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

#define UNUSED __attribute__((unused))

int main() __attribute__((weak));
int main()
{
	return 0;
}

void exit(int status) __attribute__((weak));
void exit(int status UNUSED)
{
}

// Define ABI functions that may be required at link time under specific situations
namespace __cxxabiv1
{
	extern "C"
	{
		// This is required during link when a constructor may call a pure virtual method
		// If this is required, it is likely the faulty constructor is too complex (e.g. when using
		// multiple inheritance) for the compiler to detect that no pure virtual method will be actually 
		// called. Replacing multiple inheritance, when possible, with class member instead of private
		// inheritance, can help reduce code size
//		void __cxa_pure_virtual() {}
	}
}
