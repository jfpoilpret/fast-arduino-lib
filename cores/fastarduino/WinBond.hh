#ifndef WINBOND_HH
#define WINBOND_HH

#include <util/delay.h>
#include "SPI.hh"

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
class WinBond: public SPI::SPIDevice
{
public:
	WinBond(Board::DigitalPin cs): SPIDevice{cs, SPI::ChipSelect::ACTIVE_LOW, SPI::ClockRate::CLOCK_DIV_2} {}

	// This type maps to status SEC/TB/BP2/BP1/BP0
	enum class BlockProtect:uint16_t
	{
		BLOCK_NONE			= 0x00,
		BLOCK_UPPER_64KB	= 0x01 << 2,
		BLOCK_UPPER_128KB	= 0x02 << 2,
		BLOCK_UPPER_256KB	= 0x03 << 2,
		BLOCK_UPPER_512KB	= 0x04 << 2,
		
		BLOCK_LOWER_64KB	= 0x09 << 2,
		BLOCK_LOWER_128KB	= 0x0A << 2,
		BLOCK_LOWER_256KB	= 0x0B << 2,
		BLOCK_LOWER_512KB	= 0x0C << 2,
		BLOCK_ALL			= 0x07 << 2,
		
		BLOCK_UPPER_4KB		= 0x11 << 2,
		BLOCK_UPPER_8KB		= 0x12 << 2,
		BLOCK_UPPER_16KB	= 0x13 << 2,
		BLOCK_UPPER_32KB	= 0x14 << 2,
		
		BLOCK_LOWER_4KB		= 0x19 << 2,
		BLOCK_LOWER_8KB		= 0x1A << 2,
		BLOCK_LOWER_16KB	= 0x1B << 2,
		BLOCK_LOWER_32KB	= 0x1C << 2
	};
	enum class StatusRegisterProtect: uint16_t
	{
		SOFTWARE_PROTECTION		= 0x0000,
		HARDWARE_PROTECTION		= 0x0080,
		POWER_SUPPLY_LOCKDOWN	= 0x0100
	};
	struct Status
	{
		inline bool busy() const { return value & 0x0001; }
		inline bool write_enable_latch() const { return value & 0x0002; }
		inline BlockProtect block_protect() const { return static_cast<BlockProtect>(value & 0x007A); }
		inline bool complement_protect() const { return value & 0x4000; }
		inline bool suspend_status() const { return value & 0x8000; }
		inline StatusRegisterProtect status_register_protect() const { return static_cast<StatusRegisterProtect>(value & 0x0180); }

		const uint16_t value;
		
	private:
		inline  Status(uint8_t sr1, uint8_t sr2):value(sr2 << 8 | sr1){}
		
		friend class WinBond;
	};
	
	inline Status status()
	{
		return Status(_read(0x05), _read(0x35));
	}
	void set_status(uint16_t status);
	bool wait_until_ready(uint16_t timeout_ms);

	inline void power_down()
	{
		_send(0xB9);
	}
	inline void power_up()
	{
		_send(0xAB);
		_delay_us(3);
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
		_send(0x06);
	}
	inline void disable_write()
	{
		_send(0x04);
	}

	inline void erase_sector(uint32_t address)
	{
		_send(0x20, address);
	}
	inline void erase_block_32K(uint32_t address)
	{
		_send(0x52, address);
	}
	inline void erase_block_64K(uint32_t address)
	{
		_send(0xD8, address);
	}
	inline void erase_chip()
	{
		_send(0xC7);
	}
	
	inline void write_page(uint32_t address, uint8_t* data, uint8_t size)
	{
		_send(0x02, address, data, (size == 0 ? 256 : size));
	}
	
	uint8_t read_data(uint32_t address);
	void read_data(uint32_t address, uint8_t* data, uint16_t size);
	
private:
	uint8_t _read(uint8_t code);
	void _send(uint8_t code);
	inline void _send(uint8_t code, uint32_t address)
	{
		_send(code, address, 0, 0);
	}
	void _send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size);
};

#endif /* WINBOND_HH */

