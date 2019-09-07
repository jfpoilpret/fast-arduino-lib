//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * API to handle WinBond flash memory chips through SPI interface.
 * Connection diagram:
 * 
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
 * when CS is low (active). I use a 10K resistor to pullup this pin to Vcc (3.3V).
 * 
 * @sa https://github.com/jfpoilpret/fast-arduino-lib/blob/master/refs/devices/WinBond-W25Q80BV.pdf
 */
#ifndef WINBOND_HH
#define WINBOND_HH

#include "../spi.h"
#include "../time.h"
#include "../utilities.h"

namespace devices
{
	/**
	 * SPI device driver for WinBond flash memory chips, like W25Q80BV (8 Mbit 
	 * flash).
	 * @tparam CS the output pin used for Chip Selection of the WinBond chip on
	 * the SPI bus.
	 */
	template<board::DigitalPin CS>
	class WinBond : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_LOW, spi::ClockRate::CLOCK_DIV_2>
	{
	public:
		/**
		 * Create a new device driver for a WinBond chip.
		 */
		WinBond() = default;

		/**
		 * This enum provides information about block protection (bits BP0-2,
		 * TB and SEC of Status register, §6.1.3, §6.1.4 & §6.1.5) in a more
		 * readable way.
		 * @sa Status::block_protect()
		 * @sa status()
		 */
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

		/**
		 * This enum provides information about the method of write protection
		 * of the Status register itself (bits SRP0-1 of Status register, §6.1.7).
		 * @sa Status::status_register_protect()
		 * @sa status()
		 */
		enum class StatusRegisterProtect : uint16_t
		{
			SOFTWARE_PROTECTION = 0x0000,
			HARDWARE_PROTECTION = 0x0080,
			POWER_SUPPLY_LOCKDOWN = 0x0100
		};

		/**
		 * This type maps WinBond Status register (§6.1) to more readable pieces.
		 * Note however that some bits (LB1-3, QE) are not mapped to understanble
		 * methods but can be tested directly on `value` field.
		 * @sa status()
		 */
		struct Status
		{
			bool busy() const
			{
				return value & bits::BV16(BUSY);
			}
			bool write_enable_latch() const
			{
				return value & bits::BV16(WEL);
			}
			BlockProtect block_protect() const
			{
				return static_cast<BlockProtect>(value & bits::BV16(BP0, BP1, BP2, TB, SEC));
			}
			bool complement_protect() const
			{
				return value & bits::BV16(CMP);
			}
			bool suspend_status() const
			{
				return value & bits::BV16(SUS);
			}
			StatusRegisterProtect status_register_protect() const
			{
				return static_cast<StatusRegisterProtect>(value & bits::BV16(SRP0, SRP1));
			}

			const uint16_t value;

		private:
			Status(uint8_t sr1, uint8_t sr2) : value(utils::as_uint16_t(sr2, sr1)) {}

			static constexpr const uint8_t BUSY = 0;
			static constexpr const uint8_t WEL = 1;
			static constexpr const uint8_t BP0 = 2;
			static constexpr const uint8_t BP1 = 3;
			static constexpr const uint8_t BP2 = 4;
			static constexpr const uint8_t TB = 5;
			static constexpr const uint8_t SEC = 6;
			static constexpr const uint8_t SRP0 = 7;
			static constexpr const uint8_t SRP1 = 8;
			static constexpr const uint8_t CMP = 14;
			static constexpr const uint8_t SUS = 15;

			friend class WinBond<CS>;
		};

		/**
		 * Get the value of the chip's Status register (§6.1, §6.2.8).
		 */
		Status status()
		{
			return Status(read(READ_STATUS_1), read(READ_STATUS_2));
		}

		/**
		 * Change the Status register (only writable bits, §6.2.9).
		 * @param status the new value for the Status register
		 */
		void set_status(uint16_t status);

		/**
		 * Wait until any erase or write operation is finished.
		 * This method continuously reads the Status register and check the BUSY 
		 * bit (§6.1.1). If the chip is busy, then the method yields time, i.e.
		 * put the MCU to sleep according to the default `board::SleepMode`.
		 * @param timeout_ms the maximum time, in milliseconds, to wait for the 
		 * chip to be ready
		 * @retval true if the chip is ready
		 * @retval false if the chip is still busy after @p timeout_ms delay
		 * 
		 * @sa time::yield()
		 * @sa power::Power::set_default_mode()
		 */
		bool wait_until_ready(uint16_t timeout_ms);

		/**
		 * Set the chip to low power mode (§6.2.29).
		 */
		void power_down()
		{
			send(POWER_DOWN);
		}

		/**
		 * Release power-down mode (§6.2.30).
		 */
		void power_up()
		{
			send(POWER_UP);
			time::delay_us(3);
		}

		/**
		 * Device information (§6.2.31)
		 * @sa read_device()
		 */
		struct Device
		{
			uint8_t manufacturer_ID;
			uint8_t device_ID;
		};

		/**
		 * Get device informaton §6.2.31).
		 */
		Device read_device();

		/**
		 * Get chip unique ID (§6.2.34).
		 */
		uint64_t read_unique_ID();

		/**
		 * Enable write mode for the chip (§6.2.5). This must becalled before
		 * every erase or write instruction.
		 */
		void enable_write()
		{
			send(WRITE_ENABLE);
		}

		/**
		 * Disable chip write mode (§6.2.7). This method is seldom used, as any
		 * erase or write instruction will automatically disable write mode.
		 */
		void disable_write()
		{
			send(WRITE_DISABLE);
		}

		/**
		 * Erase the sector (4KB) at @p address (§6.2.23).
		 * @param address address (24 bits) of the sector to erase
		 * @sa enable_write()
		 */
		void erase_sector(uint32_t address)
		{
			send(SECTOR_ERASE, address);
		}

		/**
		 * Erase the block (32KB) at @p address (§6.2.24).
		 * @param address address (24 bits) of the sector to erase
		 * @sa enable_write()
		 */
		void erase_block_32K(uint32_t address)
		{
			send(BLOCK_32K_ERASE, address);
		}

		/**
		 * Erase the sector (64KB) at @p address (§6.2.25).
		 * @param address address (24 bits) of the sector to erase
		 * @sa enable_write()
		 */
		void erase_block_64K(uint32_t address)
		{
			send(BLOCK_64K_ERASE, address);
		}

		/**
		 * Erase the whole chip memory (§6.2.26).
		 * @sa enable_write()
		 */
		void erase_chip()
		{
			send(CHIP_ERASE);
		}

		/**
		 * Write data (max 256 bytes) to a page (§6.2.21).
		 * @param address address (24 bits) of the first flash byte to write
		 * @param data the data to be written to the flash page at @p address;
		 * @p data may be overwritten by this operation.
		 * @param size the number of bytes to write; if `0`, then 256 bytes
		 * (one full page) will be written.
		 */
		void write_page(uint32_t address, uint8_t* data, uint8_t size)
		{
			send(PAGE_PROGRAM, address, data, ((size == 0) ? (1U + UINT8_MAX) : size));
		}

		/**
		 * Read one byte of flash memory (§6.2.10).
		 * @param address address (24 bits) of the flash byte to read
		 * @return the value read from flash memory
		 */
		uint8_t read_data(uint32_t address);

		/**
		 * Read several bytes of flash memory (§6.2.10).
		 * @param address address (24 bits) of the first flash byte to read
		 * @param data the buffer that shall receive the value of all read bytes;
		 * this must have been allocated at leat @p size bytes
		 * @param size the number of bytes to read from flash memory
		 */
		void read_data(uint32_t address, uint8_t* data, uint16_t size);

	private:
		uint8_t read(uint8_t code);
		void send(uint8_t code);
		void send(uint8_t code, uint32_t address)
		{
			send(code, address, 0, 0);
		}
		void send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size);

		// Instructions
		static constexpr const uint8_t WRITE_STATUS = 0x01;
		static constexpr const uint8_t PAGE_PROGRAM = 0x02;
		static constexpr const uint8_t READ_DATA = 0x03;
		static constexpr const uint8_t WRITE_DISABLE = 0x04;
		static constexpr const uint8_t READ_STATUS_1 = 0x05;
		static constexpr const uint8_t WRITE_ENABLE = 0x06;
		static constexpr const uint8_t FAST_READ = 0x0B;
		static constexpr const uint8_t SECTOR_ERASE = 0x20;
		static constexpr const uint8_t READ_STATUS_2 = 0x35;
		static constexpr const uint8_t READ_UNIQUE_ID = 0x4B;
		static constexpr const uint8_t BLOCK_32K_ERASE = 0x52;
		static constexpr const uint8_t DEVICE_ID = 0x90;
		static constexpr const uint8_t POWER_UP = 0xAB;
		static constexpr const uint8_t POWER_DOWN = 0xB9;
		static constexpr const uint8_t CHIP_ERASE = 0xC7;
		static constexpr const uint8_t BLOCK_64K_ERASE = 0xD8;
		
	};

	template<board::DigitalPin CS> void WinBond<CS>::set_status(uint16_t status)
	{
		this->start_transfer();
		this->transfer(bits::LOW_BYTE(status));
		this->transfer(bits::HIGH_BYTE(status));
		this->end_transfer();
	}

	template<board::DigitalPin CS> bool WinBond<CS>::wait_until_ready(uint16_t timeout_ms)
	{
		bool ready = false;
		this->start_transfer();
		this->transfer(READ_STATUS_1);
		uint32_t start = time::millis();
		while (true)
		{
			uint8_t status = this->transfer(0x00);
			if ((status & bits::BV8(Status::BUSY)) == 0)
			{
				ready = true;
				break;
			}
			if ((timeout_ms != 0) && (time::since(start) > timeout_ms)) break;
			time::yield();
		}
		this->end_transfer();
		return ready;
	}

	template<board::DigitalPin CS> typename WinBond<CS>::Device WinBond<CS>::read_device()
	{
		Device device;
		send(DEVICE_ID, 0, (uint8_t*) &device, sizeof(device));
		return device;
	}

	template<board::DigitalPin CS> uint64_t WinBond<CS>::read_unique_ID()
	{
		// Since the Read ID instruction must be followed by 4 dummy bytes before
		// returning the 8 bytes ID, we must use a 9-bytes buffer and skip its
		// first byte (the 3 other dummy bytes are already sent by send() as the
		// 0 address)
		struct PAYLOAD
		{
			uint8_t dummy;
			uint64_t id;
		};
		PAYLOAD buffer;
		send(READ_UNIQUE_ID, 0, (uint8_t*) &buffer, sizeof buffer);
		// WinBond ID is big-endian (high byte first) but AVR is little-endian
		// hence we need to convert result (using GCC builtin utility)
		return __builtin_bswap64(buffer.id);
	}

	template<board::DigitalPin CS> uint8_t WinBond<CS>::read_data(uint32_t address)
	{
		uint8_t data;
		read_data(address, &data, 1);
		return data;
	}

	template<board::DigitalPin CS> void WinBond<CS>::read_data(uint32_t address, uint8_t* data, uint16_t size)
	{
		send(READ_DATA, address, data, size);
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
		this->transfer(bits::HIGH_BYTE(address));
		this->transfer(bits::LOW_BYTE(address));
		this->transfer(data, size);
		this->end_transfer();
	}
}

#endif /* WINBOND_HH */
/// @endcond
