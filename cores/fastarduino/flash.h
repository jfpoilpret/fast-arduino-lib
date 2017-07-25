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

/// @cond api

/**
 * @file 
 * Flash memory utilities.
 */
#ifndef FLASH_HH
#define	FLASH_HH

#include <avr/pgmspace.h>

/**
 * Defines API to handle flash memory storage.
 */
namespace flash
{
	/// @cond notdocumented
	// Utility to handle PROGMEM storage for strings
	class FlashStorage;
	/// @endcond

	/**
	 * Read flash memory content at given @p address into @p item.
	 * Note that type @p T should not have constructors as they will not be
	 * properly called by this method.
	 * 
	 * @tparam T type of @p item
	 * @param address the address to read from flash storage
	 * @param item the item to read from flah storage
	 * @return a reference to read @p item
	 */
	template<typename T>
	T& read_flash(uint16_t address, T& item)
	{
		uint8_t* ptr = (uint8_t*) &item;
		for (size_t i = 0; i < sizeof(T); ++i)
			*ptr++ = pgm_read_byte(address++);
		return item;
	}

	/**
	 * Read flash memory content at given @p address into @p item.
	 * Note that type @p T should not have constructors as they will not be
	 * properly called by this method.
	 * The item shgould properly be defined in flash storage, as in:
	 * @code
	 * struct Dummy
	 * {
	 *     uint16_t a;
	 *     uint8_t b;
	 *     bool c;
	 * };
	 * const Dummy sample1 PROGMEM = {54321, 123, true};
	 * @endcode
	 * 
	 * The address of the item in flash storage can then be directly obtained:
	 * @code
	 * Dummy value;
	 * flash::read_flash(&sample1, value);
	 * @endcode
	 * 
	 * @tparam T type of @p item
	 * @param address the address to read from flash storage
	 * @param item the item to read from flah storage
	 * @return a reference to read @p item
	 */
	template<typename T>
	T& read_flash(const T* address, T& item)
	{
		return read_flash((uint16_t) address, item);
	}
}

/**
 * Force string constant to be stored as flash storage.
 * Here is a classical usage here:
 * @code
 * char buffer[64];
 * streams::OutputBuffer out{buffer};
 * out << F("Hello, world!\n");
 * @endcode
 * 
 * @param ptr the string to be automatically stored on flash
 * @sa streams::OutputBuffer::puts(const flash::FlashStorage*)
 */
#define F(ptr) (__extension__({static const char __c[] PROGMEM = (ptr); (const flash::FlashStorage*) &__c[0];}))

#endif	/* FLASH_HH */
/// @endcond
