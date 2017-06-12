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

#ifndef DS1307_H
#define DS1307_H

#include "../i2c.h"
#include "../utilities.h"

namespace devices
{
namespace rtc
{
	// Note we slightly adapted standard tm (time.h) to fit with DS1307 RTC
	struct tm
	{
		uint8_t tm_sec;		/**< seconds after the minute - [ 0 to 59 ] */
		uint8_t tm_min;		/**< minutes after the hour - [ 0 to 59 ] */
		uint8_t tm_hour;	/**< hours since midnight - [ 0 to 23 ] */
		uint8_t tm_wday;	/**< days since Sunday - [ 1 to 7 ] */
		uint8_t tm_mday;	/**< day of the month - [ 1 to 31 ] */
		uint8_t tm_mon;		/**< months since January - [ 1 to 12 ] */
		uint8_t tm_year;	/**< years since 2000 */
	};
	
	enum class SquareWaveFrequency: uint8_t
	{
		FREQ_1HZ = 0x00,
		FREQ_4096HZ = 0x01,
		FREQ_8192HZ = 0x02,
		FREQ_32768HZ = 0x03
	};
	
	class DS1307: public i2c::I2CDevice
	{
	public:
		DS1307(i2c::I2CManager& manager): I2CDevice(manager) {}

		//TODO API set datetime, get datetime, setup, read/write SRAM
		void setDateTime(const tm& datetime);
		void halt_clock();
		void getDateTime(tm& datetime);
		
		void enable_output(SquareWaveFrequency frequency = SquareWaveFrequency::FREQ_1HZ);
		void disable_output(bool output_value = false);
		
		void set_ram(uint8_t address, uint8_t data);
		uint8_t get_ram(uint8_t address);
		void set_ram(uint8_t address, const uint8_t* data, uint8_t size);
		void get_ram(uint8_t address, uint8_t* data, uint8_t size);
		template<typename T> void set_ram(uint8_t address, const T& data);
		template<typename T> void get_ram(uint8_t address, T& data);
		
	private:
		//TODO internal structures
		
	};
}
}

#endif /* DS1307_H */
