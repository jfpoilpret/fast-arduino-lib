// Important copyright notice: 
// Large parts of this file were copied from Mikael Patel's Cosa library, which copyright appears below

/**
 * @file main.cpp
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013-2015, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Arduino Che Cosa project.
 */
#include <avr/io.h>
#include <avr/interrupt.h>

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
