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

#ifndef WINBOND_HH
#define WINBOND_HH

#include "../spi.h"
#include "../time.h"

namespace devices
{
	/*
	 *                 W25Q80BV
	 *                +----U----+
	 * (/CS)--------1-|/CS   VCC|-8---------(VCC)
	 * (MISO)-------2-|DO  /HOLD|-7--VVVV---(VCC)
	 *            --3-|/WP   CLK|-6---------(CLK)
	 * (GND)--------4-|GND    DI|-5---------(MOSI)
	 *                +---------+
	 * 
	 * Note that WinBond IC works on Vcc = 3.3V (not 5V) and any inputs should be limited to 3.3V,
	 * hence, when working with 5V MCU, use level converters at least for DI, CLK and CS pins.
	 * This library operates WinBond IC in single SPI mode only (WinBond supports dual and quad modes);
	 * in this mode, the /HOLD pin should not be left dangling as it may trigger transmission errors
	 * when CS is low (active). I use a 10K resistor to pullup this pin to Vcc (3.3V)
	 */

	// Tested with W25Q80BV (8 Mbit)
	template<board::DigitalPin CS>
	class WinBond : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_LOW, spi::ClockRate::CLOCK_DIV_2>
	{
	public:
		WinBond()
		{
		}

		// This type maps to status SEC/TB/BP2/BP1/BP0
		enum class BlockProtect : uint16_t
		{
			BLOCK_NONE = 0x00,
			BLOCK_UPPER_64KB = 0x01 << 2,
			BLOCK_UPPER_128KB = 0x02 << 2,
			BLOCK_UPPER_256KB = 0x03 << 2,
			BLOCK_UPPER_512KB = 0x04 << 2,

			BLOCK_LOWER_64KB = 0x09 << 2,
			BLOCK_LOWER_128KB = 0x0A << 2,
			BLOCK_LOWER_256KB = 0x0B << 2,
			BLOCK_LOWER_512KB = 0x0C << 2,
			BLOCK_ALL = 0x07 << 2,

			BLOCK_UPPER_4KB = 0x11 << 2,
			BLOCK_UPPER_8KB = 0x12 << 2,
			BLOCK_UPPER_16KB = 0x13 << 2,
			BLOCK_UPPER_32KB = 0x14 << 2,

			BLOCK_LOWER_4KB = 0x19 << 2,
			BLOCK_LOWER_8KB = 0x1A << 2,
			BLOCK_LOWER_16KB = 0x1B << 2,
			BLOCK_LOWER_32KB = 0x1C << 2
		};
		enum class StatusRegisterProtect : uint16_t
		{
			SOFTWARE_PROTECTION = 0x0000,
			HARDWARE_PROTECTION = 0x0080,
			POWER_SUPPLY_LOCKDOWN = 0x0100
		};
		struct Status
		{
			inline bool busy() const
			{
				return value & 0x0001;
			}
			inline bool write_enable_latch() const
			{
				return value & 0x0002;
			}
			inline BlockProtect block_protect() const
			{
				return static_cast<BlockProtect>(value & 0x007A);
			}
			inline bool complement_protect() const
			{
				return value & 0x4000;
			}
			inline bool suspend_status() const
			{
				return value & 0x8000;
			}
			inline StatusRegisterProtect status_register_protect() const
			{
				return static_cast<StatusRegisterProtect>(value & 0x0180);
			}

			const uint16_t value;

		private:
			inline Status(uint8_t sr1, uint8_t sr2) : value(sr2 << 8 | sr1)
			{
			}

			friend class WinBond<CS>;
		};

		inline Status status()
		{
			return Status(read(0x05), read(0x35));
		}
		void set_status(uint16_t status);
		bool wait_until_ready(uint16_t timeout_ms);

		inline void power_down()
		{
			send(0xB9);
		}
		inline void power_up()
		{
			send(0xAB);
			time::delay_us(3);
		}

		struct Device
		{
			uint8_t manufacturer_ID;
			uint8_t device_ID;
		};
		Device read_device();
		uint64_t read_unique_ID();

		inline void enable_write()
		{
			send(0x06);
		}
		inline void disable_write()
		{
			send(0x04);
		}

		inline void erase_sector(uint32_t address)
		{
			send(0x20, address);
		}
		inline void erase_block_32K(uint32_t address)
		{
			send(0x52, address);
		}
		inline void erase_block_64K(uint32_t address)
		{
			send(0xD8, address);
		}
		inline void erase_chip()
		{
			send(0xC7);
		}

		inline void write_page(uint32_t address, uint8_t* data, uint8_t size)
		{
			send(0x02, address, data, (size == 0 ? 256 : size));
		}

		uint8_t read_data(uint32_t address);
		void read_data(uint32_t address, uint8_t* data, uint16_t size);

	private:
		uint8_t read(uint8_t code);
		void send(uint8_t code);
		inline void send(uint8_t code, uint32_t address)
		{
			send(code, address, 0, 0);
		}
		void send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size);
	};

	template<board::DigitalPin CS> void WinBond<CS>::set_status(uint16_t status)
	{
		this->start_transfer();
		this->transfer(status);
		this->transfer(status >> 8);
		this->end_transfer();
	}

	template<board::DigitalPin CS> bool WinBond<CS>::wait_until_ready(uint16_t timeout_ms)
	{
		bool ready = false;
		this->start_transfer();
		this->transfer(0x05);
		//TODO add timing check (once RTT is available)
		while (true)
		{
			uint8_t status = this->transfer(0x00);
			if (!(status & 0x01))
			{
				ready = true;
				break;
			}
		}
		this->end_transfer();
		return ready;
	}

	template<board::DigitalPin CS> typename WinBond<CS>::Device WinBond<CS>::read_device()
	{
		Device device;
		send(0x90, 0, (uint8_t*) &device, sizeof(device));
		return device;
	}

	template<board::DigitalPin CS> uint64_t WinBond<CS>::read_unique_ID()
	{
		uint8_t buffer[9];
		send(0x4B, 0, buffer, 9);
		//FIXME check if we need to exchange bytes (endianness)
		uint64_t id = *((uint64_t*) &buffer[1]);
		return id;
	}

	template<board::DigitalPin CS> uint8_t WinBond<CS>::read_data(uint32_t address)
	{
		uint8_t data;
		read_data(address, &data, 1);
		return data;
	}

	template<board::DigitalPin CS> void WinBond<CS>::read_data(uint32_t address, uint8_t* data, uint16_t size)
	{
		send(0x03, address, data, size);
	}

	template<board::DigitalPin CS> uint8_t WinBond<CS>::read(uint8_t code)
	{
		this->start_transfer();
		this->transfer(code);
		uint8_t result = this->transfer(0);
		this->end_transfer();
		return result;
	}

	template<board::DigitalPin CS> void WinBond<CS>::send(uint8_t code)
	{
		this->start_transfer();
		this->transfer(code);
		this->end_transfer();
	}

	template<board::DigitalPin CS> void WinBond<CS>::send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size)
	{
		this->start_transfer();
		this->transfer(code);
		this->transfer(address >> 16);
		this->transfer(address >> 8);
		this->transfer(address);
		this->transfer(data, size);
		this->end_transfer();
	}
}

#endif /* WINBOND_HH */
