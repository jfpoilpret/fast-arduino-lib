#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * Compiler warning on unused varable.
 */
#define UNUSED(x) (void) (x)


int main(void) __attribute__((weak));
int main(void)
{
	return 0;
}

void exit(int status) __attribute__((weak));
void exit(int status)
{
  UNUSED(status);
}

/**
 * Support for local static variables
 */
namespace __cxxabiv1
{
	typedef int __guard;

	extern "C" int __cxa_guard_acquire(__guard *g)
	{
		UNUSED(g);
		return (0);
	}

	extern "C" void __cxa_guard_release(__guard *g)
	{
		UNUSED(g);
	}

	extern "C" void __cxa_guard_abort(__guard *g)
	{
		UNUSED(g);
	}

	extern "C" void __cxa_pure_virtual(void)
	{
	}

	void* __dso_handle = 0;

	extern "C" int __cxa_atexit(void (*destructor)(void*), void* arg, void* dso)
	{
		UNUSED(destructor);
		UNUSED(arg);
		UNUSED(dso);
		return 0;
	}

	extern "C" void __cxa_finalize(void* f)
	{
		UNUSED(f);
	}
}
