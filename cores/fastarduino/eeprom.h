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

#define REGISTER_EEPROM_ISR()		\
REGISTER_ISR_METHOD_(eeprom::AbstractEEPROMWriter, &eeprom::AbstractEEPROMWriter::on_ready)

namespace eeprom
{
	// Blocking EEPROM handler
	class EEPROM
	{
	public:
		template<typename T>
		static void write(uint16_t address, const T& value)
		{
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i)
				write(address++, *v++);
		}

		template<typename T>
		static void read(uint16_t address, T& value)
		{
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i)
				read(address++, *v++);
		}

		inline static void write(uint16_t address, uint8_t value)
		{
			wait_until_ready();
			_write(address, value);
		}
		
		inline static void read(uint16_t address, uint8_t& value)
		{
			wait_until_ready();
			EEAR = address;
			EECR = _BV(EERE);
			value = EEDR;
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

	class WriteHandler
	{
	public:
		virtual void on_write_finished() = 0;
	};

	class AbstractEEPROMWriter: public EEPROM
	{
	protected:
		template<typename T>
		AbstractEEPROMWriter(uint16_t address, const T* value, WriteHandler* handler)
		:_address{address}, _buffer{value}, _size{sizeof(T)}, _handler{handler}
		{
			register_handler(*this);
		}
		
		~AbstractEEPROMWriter()
		{
			wait_until_done();
		}
		
		inline void start()
		{
			EECR = _BV(EERIE);
		}

	public:
		inline void wait_until_done()
		{
			while (_size) ;
		}
		
		void on_ready()
		{
			if (_size)
			{
				_write(_address++, *_buffer++);
				--_size;
				EECR |= _BV(EERIE);
			}
			else
			{
				EECR = 0;
				if (_handler) _handler->on_write_finished();
			}
		}

	private:
		volatile uint16_t _address;
		volatile uint8_t* _buffer;
		volatile uint8_t _size;
		WriteHandler* _handler;
	};
	
	template<typename T>
	class EEPROMWriter: public AbstractEEPROMWriter
	{
	public:
		EEPROMWriter(uint16_t address, const T& value, WriteHandler* handler = 0)
		:AbstractEEPROMWriter{address, &_value, handler}, _value{value}
		{
			start();
		}

	private:
		T _value;
	};
}

#endif /* EEPROM_H */
