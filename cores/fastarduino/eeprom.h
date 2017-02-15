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
REGISTER_ISR_METHOD_(eeprom::AsyncEEPROMManager, &eeprom::AsyncEEPROMManager::on_ready)

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
			EEAR = address;
			EEDR = value;
			synchronized
			{
				EECR |= _BV(EEMPE);
				EECR |= _BV(EEPE);
			}
		}

		inline static void read(uint16_t address, uint8_t& value)
		{
			wait_until_ready();
			EEAR = address;
			EECR |= _BV(EERE);
			value = EEDR;
		}

		inline static void wait_until_ready()
		{
			while (EECR & _BV(EEPE)) ;
		}
	};

	class WriteHandler
	{
	public:
		virtual void on_write_finished() = 0;
	};
//	class ReadHandler;

	//TODO Make it used for both read and write
	class AsyncEEPROMManager
	{
	protected:
		template<typename T>
		AsyncEEPROMManager(uint16_t address, const T* value, WriteHandler* handler)
		:_address{address}, _buffer{value}, _size{sizeof(T)}, _handler{handler}
		{
			register_handler(*this);
		}
		
		~AsyncEEPROMManager()
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
				EEAR = _address++;
				EEDR = *_buffer++;
				EECR |= _BV(EEMPE);
				EECR |= _BV(EEPE);
				--_size;
			}
			else
			{
				EECR &= ~_BV(EERIE);
				if (_handler) _handler->on_write_finished();
			}
		}

	private:
		uint16_t _address;
		uint8_t* _buffer;
		uint8_t _size;
		WriteHandler* _handler;
	};
	
	template<typename T>
	class EEPROMWriter: public AsyncEEPROMManager
	{
	public:
		EEPROMWriter(uint16_t address, const T& value, WriteHandler* handler = 0)
		:AsyncEEPROMManager{address, &_value, handler}, _value{value}
		{
			start();
		}

	private:
		T _value;
	};
}

#endif /* EEPROM_H */
