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

//TODO 1. Check address and size: reject or limit address/size?
//TODO 2. Add callback
namespace eeprom
{
	// Blocking EEPROM handler
	class EEPROM
	{
	public:
		template<typename T>
		static bool read(uint16_t address, T& value)
		{
			if (!check(address, sizeof(T))) return false;
			uint8_t* v = (uint8_t*) &value;
			for (uint16_t i = 0; i < sizeof(T); ++i)
				_blocked_read(address++, *v++);
			return true;
		}

		template<typename T>
		static bool read(uint16_t address, T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			uint8_t* v = (uint8_t*) value;
			for (uint16_t i = 0; i < count * sizeof(T); ++i)
				_blocked_read(address++, *v++);
			return true;
		}

		inline static bool read(uint16_t address, uint8_t& value)
		{
			if (!check(address, 1)) return false;
			_blocked_read(address, value);
			return true;
		}

		template<typename T>
		static bool write(uint16_t address, const T& value)
		{
			if (!check(address, sizeof(T))) return false;
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i)
				_blocked_write(address++, *v++);
			return true;
		}

		template<typename T>
		static bool write(uint16_t address, const T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			uint8_t* v = (uint8_t*) value;
			for (uint8_t i = 0; i < count * sizeof(T); ++i)
				_blocked_write(address++, *v++);
			return true;
		}

		inline static bool write(uint16_t address, uint8_t value)
		{
			if (!check(address, 1)) return false;
			_blocked_write(address, value);
			return true;
		}
		
		static void erase()
		{
			for (uint16_t address = 0; address < size(); ++address)
			{
				wait_until_ready();
				_erase(address);
			}
		}
		
		static constexpr uint16_t size()
		{
			return E2END + 1;
		}
		
		inline static void wait_until_ready()
		{
			while (EECR & _BV(EEPE)) ;
		}
		
	protected:
		inline static bool check(uint16_t address, uint16_t size)
		{
			return size && (address <= E2END) && (size <= (E2END + 1)) && ((address + size) <= (E2END + 1));
		}
		
		inline static void _blocked_read(uint16_t address, uint8_t& value)
		{
			wait_until_ready();
			_read(address, value);
		}

		inline static void _read(uint16_t address, uint8_t& value)
		{
			EEAR = address;
			EECR = _BV(EERE);
			value = EEDR;
		}

		inline static void _blocked_write(uint16_t address, uint8_t value)
		{
			wait_until_ready();
			_write(address, value);
		}
		
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

		inline static bool _erase(uint16_t address)
		{
			EEAR = address;
			EECR = _BV(EERE);
			uint8_t value = EEDR;
			// Some bits need to be erased (ie set to 1)
			if (value == 0xFF) return false;
			EECR = EEPM0;
			EEDR = 0xFF;
			synchronized
			{
				EECR |= _BV(EEMPE);
				EECR |= _BV(EEPE);
			}
			return true;
		}
	};
	
	class QueuedWriter: private EEPROM
	{
	public:
		template<uint16_t SIZE>
		QueuedWriter(uint8_t (&buffer)[SIZE]):_buffer{buffer}, _current{}, _erase{false}, _done{true}
		{
			register_handler(*this);
		}
		
		template<typename T>
		bool write(uint16_t address, const T& value)
		{
			if (!check(address, sizeof(T))) return false;
			synchronized return _write(address, (uint8_t*) &value, sizeof(T));
		}
		
		template<typename T>
		bool write(uint16_t address, const T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			synchronized return _write(address, (uint8_t*) value, count * sizeof(T));
		}
		
		bool write(uint16_t address, uint8_t value)
		{
			if (!check(address, 1)) return false;
			synchronized return _write(address, &value, 1);
		}
		
		void erase()
		{
			// First remove all pending writes
			synchronized
			{
				_buffer._clear();
				_current.size = 0;
			}
			// Wait until current byte write is finished
			wait_until_done();
			synchronized
			{
				// Start erase
				_erase = true;
				_current.address = 0;
				_current.size = size();
				// Start transmission if not done yet
				EECR = _BV(EERIE);
			}
		}
		
		void wait_until_done()
		{
			while (!_done) ;
		}
		
		bool is_done()
		{
			return _done;
		}
		
		void on_ready()
		{
			if (_erase)
			{
				if (_current.size)
					erase_next();
				else
				{
					// All erases are finished
					_erase = false;
					// Mark all EEPROM work as finished if no write is pending in the queue
					if (_buffer._empty())
					{
						_done = true;
						EECR = 0;
					}
				}
			}
			else if (_current.size)
				// There is one item being currently written, write next byte
				write_next();
			else if (!_buffer._empty())
			{
				// Current item is finished writing but there is another item to be written in the queue
				// Get new item and start transmission of first byte
				_current = next_item();
				write_next();
			}
			else
			{
				// All writes are finished
				_done = true;
				EECR = 0;
			}
		}
		
	private:
		static const uint16_t ITEM_SIZE = 3;
		
		void write_next()
		{
			uint8_t value;
			_buffer._pull(value);
			EEPROM::_write(_current.address++, value);
			--_current.size;
			EECR |= _BV(EERIE);
		}
		
		void erase_next()
		{
			EEPROM::_erase(_current.address++);
			--_current.size;
			EECR |= _BV(EERIE);
		}
		
		bool _write(uint16_t address, uint8_t* value, uint16_t size)
		{
			// First check if there is enough space in _buffer for this queued write
			if ((_buffer._free() < size + ITEM_SIZE) || !size)
				return false;
			_done = false;
			// Add new queued write to buffer
			_buffer._push(WriteItem::value1(address, size));
			_buffer._push(WriteItem::value2(address, size));
			_buffer._push(WriteItem::value3(address, size));
			for (uint16_t i = 0; i < size; ++i)
				_buffer._push(*value++);
			
			// Start transmission if not done yet
			EECR = _BV(EERIE);
			return true;
		}
		
		struct WriteItem
		{
			WriteItem():address{0}, size{0} {}
			WriteItem(uint8_t value1, uint8_t value2, uint8_t value3)
				:	address{uint16_t(value1 << 4 | value2 >> 4)}, 
					size{uint16_t(is_zero((value2 & 0x0F) << 8 | value3, E2END + 1))} {}
			inline static uint8_t value1(uint16_t address, uint16_t size UNUSED)
			{
				return address >> 4;
			}
			inline static uint8_t value2(uint16_t address, uint16_t size)
			{
				return address << 4 | size >> 8;
			}
			inline static uint8_t value3(uint16_t address UNUSED, uint16_t size)
			{
				return size;
			}
			
			uint16_t address;
			uint16_t size;
		};
		
		WriteItem next_item()
		{
			uint8_t value1, value2, value3;
			_buffer._pull(value1);
			_buffer._pull(value2);
			_buffer._pull(value3);
			return WriteItem{value1, value2, value3};
		}

		Queue<uint8_t, uint8_t> _buffer;
		WriteItem _current;
		volatile bool _erase;
		volatile bool _done;
	};
}

#endif /* EEPROM_H */
