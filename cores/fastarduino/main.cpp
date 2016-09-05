#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * Compiler warning on unused varable.
 */
#define UNUSED __attribute__((unused))


int main() __attribute__((weak, OS_main));
int main()
{
	return 0;
}

void exit(int status) __attribute__((weak));
void exit(int status UNUSED)
{
}

/**
 * Support for local static variables
 */
namespace __cxxabiv1
{
	typedef int __guard;

	extern "C" int __cxa_guard_acquire(__guard *g UNUSED)
	{
		return (0);
	}

	extern "C" void __cxa_guard_release(__guard *g UNUSED)
	{
	}

	extern "C" void __cxa_guard_abort(__guard *g UNUSED)
	{
	}

	extern "C" void __cxa_pure_virtual(void)
	{
	}

	void* __dso_handle = 0;

	extern "C" int __cxa_atexit(void (*destructor)(void*) UNUSED, void* arg UNUSED, void* dso UNUSED)
	{
		return 0;
	}

	extern "C" void __cxa_finalize(void* f UNUSED)
	{
	}
}
