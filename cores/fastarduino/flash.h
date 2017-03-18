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

#ifndef FLASH_HH
#define	FLASH_HH

#include <avr/pgmspace.h>

namespace flash
{
	// Utilities to handle PROGMEM storage for strings
	class FlashStorage;

	template<typename T>
	T& read_flash(uint16_t address, T& item)
	{
		uint8_t* ptr = (uint8_t*) &item;
		for (size_t i = 0; i < sizeof(T); ++i)
			*ptr++ = pgm_read_byte(address++);
		return item;
	}

	template<typename T>
	T& read_flash(const T* address, T& item)
	{
		return read_flash((uint16_t) address, item);
	}
}

// Utilities to handle PROGMEM storage for strings
#define F(ptr) (__extension__({static const char __c[] PROGMEM = (ptr); (const flash::FlashStorage*) &__c[0];}))

#endif	/* FLASH_HH */
