
#define UNUSED __attribute__((unused))

namespace __cxxabiv1
{
	typedef int __guard;

	void* __dso_handle = 0;

	extern "C"
	{
		// https://mentorembedded.github.io/cxx-abi/abi.html#guards
		// https://mentorembedded.github.io/cxx-abi/abi.html#once-ctor
		int __cxa_guard_acquire(__guard *g UNUSED)
		{
			return 0;
		}

		void __cxa_guard_release(__guard *g UNUSED)
		{
		}

		void __cxa_guard_abort(__guard *g UNUSED)
		{
		}

		// https://mentorembedded.github.io/cxx-abi/abi.html#pure-virtual
		void __cxa_pure_virtual(void)
		{
		}

		// https://mentorembedded.github.io/cxx-abi/abi.html#dso-dtor
		int __cxa_atexit(void (*destructor)(void*) UNUSED, void* arg UNUSED, void* dso UNUSED)
		{
			return 0;
		}

		void __cxa_finalize(void* f UNUSED)
		{
		}
	}
}
