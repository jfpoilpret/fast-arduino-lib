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

/// @cond api

/**
 * @file 
 * Flash memory utilities.
 */
#ifndef FLASH_HH
#define FLASH_HH

#include "boards/io.h"
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
	 * Read flash memory content at given @p address into @p buffer.
	 * Note that type @p T should not have constructors as they will not be
	 * properly called by this method.
	 * 
	 * @tparam T type of items contained in @p buffer
	 * @param address the address to read from flash storage
	 * @param buffer the array (already allocated by caller) to be copied by 
	 * flash storage content
	 * @param size the number of items (of type @p T) to read from flash storage
	 * @return a pointer to @p buffer
	 */
	template<typename T = uint8_t>
	T* read_flash(uint16_t address, T* buffer, uint8_t size)
	{
		uint8_t* ptr = (uint8_t*) buffer;
		for (size_t i = 0; i < (size * sizeof(T)); ++i) *ptr++ = pgm_read_byte(address++);
		return buffer;
	}

	/**
	 * Read flash memory content at given @p address into @p item.
	 * Note that type @p T should not have constructors as they will not be
	 * properly called by this method.
	 * 
	 * @tparam T type of @p item
	 * @param address the address to read from flash storage
	 * @param item the item to read from flash storage
	 * @return a reference to read @p item
	 */
	template<typename T> T& read_flash(uint16_t address, T& item)
	{
		uint8_t* ptr = (uint8_t*) &item;
		for (size_t i = 0; i < sizeof(T); ++i) *ptr++ = pgm_read_byte(address++);
		return item;
	}

	/**
	 * Read flash memory content at given @p address into @p item.
	 * Note that type @p T should not have constructors as they will not be
	 * properly called by this method.
	 * The item should properly be defined in flash storage, as in:
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
	 * @param item the item to read from flash storage
	 * @return a reference to read @p item
	 */
	template<typename T> T& read_flash(const T* address, T& item)
	{
		return read_flash((uint16_t) address, item);
	}

	/**
	 * Functor reading items from an address in AVR Flash memory (PROGMEM).
	 * 
	 * @tparam T the typ of items to read from Flash
	 */
	template<typename T> class FlashReader
	{
	public:
		/**
		 * Construct a FlashReader functor reading Flash from @p flash_buffer address.
		 * 
		 * @param flash_buffer a pointer to the first item in Flash memory
		 */
		explicit FlashReader(const T* flash_buffer) : address_{uint16_t(flash_buffer)} {}

		/**
		 * Get the enxt item read from memory.
		 * 
		 * @return T the next read item from Flash
		 */
		T operator()()
		{
			T item;
			read_flash(address_, item);
			address_ += sizeof(T);
			return item;
		}

	private:
		uint16_t address_;
	};
}

/**
 * Force string constant to be stored as flash storage.
 * Here is a classical usage here:
 * @code
 * char buffer[64];
 * streams::ostreambuf out{buffer};
 * out << F("Hello, world!\n");
 * @endcode
 * 
 * @param ptr the string to be automatically stored on flash
 * @sa streams::ostreambuf::sputn(const flash::FlashStorage*)
 */
#define F(ptr)                                   \
	(__extension__({                             \
		static const char __c[] PROGMEM = (ptr); \
		(const flash::FlashStorage*) &__c[0];    \
	}))

#endif /* FLASH_HH */
/// @endcond
