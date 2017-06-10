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

#ifndef I2C_HH
#define I2C_HH

#include <stdint.h>

//TODO register ISR macro?

//NOTE only Master operation is supported for the moment
//TODO is it useful to support interrupt-driven (async) mode? that would require static buffers for read and write!
namespace i2c
{
	// all static or singleton?
	class I2CManager
	{
	public:
		void begin(uint32_t frequency = 100000UL);
		void set_frequency(uint32_t frequency);
		void end();
		
		//TODO constexpr helper methods to compute TWBR and prescaler for given frequency
		//TODO ERRORS? constants in errors.h or dedicated enum class?
		//TODO should methods return the error or just abool and then an error() method would return the exact issue
	private:
		//TODO low-level methods to handle the bus, called by I2CDevice
		int start();
		int send_sla(uint8_t address);
		int send_data(uint8_t data);
		int receive_data(uint8_t& data);
		int stop();
		
		friend class I2CDevice;
	};
	
	class I2CDevice
	{
	public:
		I2CDevice(uint8_t address): _address{address & 0xF8} {}
		
		//TODO do we need address here? should already be a member of I2CDevice since construction!
		//TODO shouldn't these methods be all protected (ie used by actual devices with "business" methods)?
		int write(uint8_t address, const uint8_t* data, uint8_t size, bool dont_stop = false);
		template<T> int write(uint8_t address, const T& data, bool dont_stop = false);
		int read(uint8_t address, uint8_t* buffer, uint8_t size, bool dont_stop = false);
		template<T> int read(uint8_t address, T& data, bool dont_stop = false);
		
	private:
		const uint8_t _address;
	};
};

#endif /* I2C_HH */

