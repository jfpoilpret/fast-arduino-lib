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

#ifndef EEPROM_H
#define EEPROM_H

#include <avr/io.h>
#include <avr/interrupt.h>

#include "utilities.h"
#include "queue.h"

#define REGISTER_EEPROM_ISR()		\
REGISTER_ISR_METHOD_(EE_READY_vect, eeprom::QueuedWriter, &eeprom::QueuedWriter::on_ready)

namespace eeprom
{
	// Blocking EEPROM handler
	class EEPROM
	{
	public:
		template<typename T>
		static void read(uint16_t address, T& value)
		{
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i)
				read(address++, *v++);
		}

		inline static void read(uint16_t address, uint8_t& value)
		{
			wait_until_ready();
			EEAR = address;
			EECR = _BV(EERE);
			value = EEDR;
		}

		template<typename T>
		static void write(uint16_t address, const T& value)
		{
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i)
				write(address++, *v++);
		}

		inline static void write(uint16_t address, uint8_t value)
		{
			wait_until_ready();
			_write(address, value);
		}
		
		static constexpr const uint16_t size()
		{
			return E2END + 1;
		}
		
		inline static void wait_until_ready()
		{
			while (EECR & _BV(EEPE)) ;
		}
		
	protected:
		// In order to optimize write time we read current byte first, then compare it with new value
		// Then we choose between erase, write and erase+write based on comparison
		// This approach is detailed in ATmel note AVR103: Using the EEPROM Programming Modes 
		// http://www.atmel.com/images/doc2578.pdf
		inline static void _write(uint16_t address, uint8_t value)
		{
			EEAR = address;
			EECR = _BV(EERE);
			uint8_t old_value = EEDR;
			uint8_t diff = old_value ^ value;
			if (diff & value)
			{
				// Some bits need to be erased (ie set to 1)
				if (value == 0xFF)
					// Erase only
					EECR = EEPM0;
				else
					// Combine Erase/Write operation
					EECR = 0;
			}
			else
			{
				// No bit to be erased
				if (diff)
					// Some bits to be programmed (set to 0): Write only
					EECR = EEPM1;
				else
					// old value == new value => do nothing
					return;
			}
			EEDR = value;
			synchronized
			{
				EECR |= _BV(EEMPE);
				EECR |= _BV(EEPE);
			}
		}
	};
	
	//TODO possibility to add a callback when write is finished?
	class QueuedWriter: private EEPROM
	{
	public:
		template<uint16_t SIZE>
		QueuedWriter(uint8_t (&buffer)[SIZE]):_buffer{buffer}, _current{}, _done{true}
		{
			register_handler(*this);
		}
		
		template<typename T>
		bool write(uint16_t address, const T& value)
		{
			synchronized return _write(address, (uint8_t*) &value, sizeof(T));
		}
		
		bool write(uint16_t address, uint8_t value)
		{
			synchronized return _write(address, &value, 1);
		}
		
		void wait_until_done()
		{
			while (!_done) ;
		}
		
		void on_ready()
		{
			// Is there any item being currently written
			if (_current.size)
			{
				uint8_t value;
				_buffer._pull(value);
				EEPROM::_write(_current.address++, value);
				--_current.size;
				EECR |= _BV(EERIE);
			}
			else
			{
				// Is there any item left to be written
				if (_buffer._pull(_current.low_address))
				{
					// Yes, get it and start transmission
					_buffer._pull(_current.high_address);
					_buffer._pull(_current.size);
					EECR |= _BV(EERIE);
				}
				else
				{
					// All writes are finished
					_done = true;
					EECR = 0;
				}
			}
		}
		
	private:
		bool _write(uint16_t address, uint8_t* value, uint8_t size)
		{
			// First check if there is enough space in _buffer for this queued write
			if (_buffer._free() < size + sizeof(WriteItem))
				return false;
			_done = false;
			// Add new queued write to buffer
			_buffer._push(address & 0xFF);
			_buffer._push(address >> 8);
			_buffer._push(size);
			for (uint8_t i = 0; i < size; ++i)
				_buffer._push(*value++);
			
			// Start transmission if not done yet
			EECR = _BV(EERIE);
			return true;
		}

		struct WriteItem
		{
			WriteItem(): address{0}, size{0} {}
			union
			{
				uint16_t address;
				struct
				{
					uint8_t low_address;
					uint8_t high_address;
				};
			};
			uint8_t size;
		};
		
		Queue<uint8_t, uint8_t> _buffer;
		WriteItem _current;
		volatile bool _done;
	};
}

#endif /* EEPROM_H */
