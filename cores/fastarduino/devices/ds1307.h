//   Copyright 2016-2018 Jean-Francois Poilpret
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

#include "../i2c_device.h"
#include "../utilities.h"

namespace devices
{
	namespace rtc
	{
		enum class WeekDay : uint8_t
		{
			Sunday = 1,
			Monday,
			Tuesday,
			Wednesday,
			Thursday,
			Friday,
			Saturday
		};

		// Note we slightly adapted standard tm (time.h) to fit with DS1307 RTC
		// IMPORTANT! You are responsible to set the correct week day when setting RTC date!
		struct tm
		{
			uint8_t tm_sec;  /**< seconds after the minute - [ 0 to 59 ] */
			uint8_t tm_min;  /**< minutes after the hour - [ 0 to 59 ] */
			uint8_t tm_hour; /**< hours since midnight - [ 0 to 23 ] */
			WeekDay tm_wday; /**< days since Sunday - [ 1 to 7 ] */
			uint8_t tm_mday; /**< day of the month - [ 1 to 31 ] */
			uint8_t tm_mon;  /**< months since January - [ 1 to 12 ] */
			uint8_t tm_year; /**< years since 2000 */
		};

		enum class SquareWaveFrequency : uint8_t
		{
			FREQ_1HZ = 0x00,
			FREQ_4096HZ = 0x01,
			FREQ_8192HZ = 0x02,
			FREQ_32768HZ = 0x03
		};

		class DS1307 : public i2c::I2CDevice<i2c::I2CMode::Standard>
		{
		public:
			DS1307(MANAGER& manager) : I2CDevice(manager)
			{
			}

			bool set_datetime(tm& datetime)
			{
				// 1st convert datetime for DS1307 (BCD)
				datetime.tm_sec = utils::binary_to_bcd(datetime.tm_sec);
				datetime.tm_min = utils::binary_to_bcd(datetime.tm_min);
				datetime.tm_hour = utils::binary_to_bcd(datetime.tm_hour);
				datetime.tm_mday = utils::binary_to_bcd(datetime.tm_mday);
				datetime.tm_mon = utils::binary_to_bcd(datetime.tm_mon);
				datetime.tm_year = utils::binary_to_bcd(datetime.tm_year);
				// send register address to write to (0)
				// send datetime at address 0
				return write(DEVICE_ADDRESS, TIME_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					   write(DEVICE_ADDRESS, datetime, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}

			bool get_datetime(tm& datetime)
			{
				// send register address to read from (0)
				// read datetime at address 0
				if (write(DEVICE_ADDRESS, TIME_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					read(DEVICE_ADDRESS, datetime, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
				{
					// convert DS1307 output (BCD) to integer type
					datetime.tm_sec = utils::bcd_to_binary(datetime.tm_sec);
					datetime.tm_min = utils::bcd_to_binary(datetime.tm_min);
					datetime.tm_hour = utils::bcd_to_binary(datetime.tm_hour);
					datetime.tm_mday = utils::bcd_to_binary(datetime.tm_mday);
					datetime.tm_mon = utils::bcd_to_binary(datetime.tm_mon);
					datetime.tm_year = utils::bcd_to_binary(datetime.tm_year);
					return true;
				}
				return false;
			}

			bool halt_clock()
			{
				// just write 0x80 at address 0
				return write(DEVICE_ADDRESS, TIME_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					   write(DEVICE_ADDRESS, uint8_t(0x80), i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}

			bool enable_output(SquareWaveFrequency frequency = SquareWaveFrequency::FREQ_1HZ)
			{
				ControlRegister control;
				control.sqwe = 1;
				control.rs = uint8_t(frequency);
				return write(DEVICE_ADDRESS, CONTROL_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					   write(DEVICE_ADDRESS, control, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}
			bool disable_output(bool output_value = false)
			{
				ControlRegister control;
				control.out = output_value;
				return write(DEVICE_ADDRESS, CONTROL_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
					   write(DEVICE_ADDRESS, control, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
			}

			static constexpr uint8_t ram_size()
			{
				return RAM_SIZE;
			}
			bool set_ram(uint8_t address, uint8_t data)
			{
				address += RAM_START;
				if (address < RAM_END)
					return write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
						   write(DEVICE_ADDRESS, data, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
				else
					return false;
			}
			uint8_t get_ram(uint8_t address)
			{
				address += RAM_START;
				uint8_t data = 0;
				if (address < RAM_END)
				{
					write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP);
					read(DEVICE_ADDRESS, data, i2c::BusConditions::REPEAT_START_STOP);
				}
				return data;
			}

			bool set_ram(uint8_t address, const uint8_t* data, uint8_t size)
			{
				address += RAM_START;
				if (address + size <= RAM_END)
					return write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
						   write(DEVICE_ADDRESS, data, size, i2c::BusConditions::NO_START_STOP) == i2c::Status::OK;
				else
					return false;
			}
			bool get_ram(uint8_t address, uint8_t* data, uint8_t size)
			{
				address += RAM_START;
				if (address + size <= RAM_END)
					return write(DEVICE_ADDRESS, address, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK &&
						   read(DEVICE_ADDRESS, data, size, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK;
				else
					return false;
			}

			template<typename T> bool set_ram(uint8_t address, const T& data)
			{
				return set_ram(address, &data, sizeof(T));
			}
			template<typename T> bool get_ram(uint8_t address, T& data)
			{
				return get_ram(address, &data, sizeof(T));
			}

		private:
			static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
			static constexpr const uint8_t TIME_ADDRESS = 0x00;
			static constexpr const uint8_t CONTROL_ADDRESS = 0x07;
			static constexpr const uint8_t RAM_START = 0x08;
			static constexpr const uint8_t RAM_END = 0x40;
			static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;

			union ControlRegister
			{
				ControlRegister(uint8_t data = 0) : data{data}
				{
				}

				uint8_t data;
				struct
				{
					uint8_t rs : 2;
					uint8_t res1 : 2;
					uint8_t sqwe : 1;
					uint8_t res2 : 2;
					uint8_t out : 1;
				};
			};
		};
	}
}

#endif /* DS1307_H */
