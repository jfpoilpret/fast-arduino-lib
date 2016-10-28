#ifndef WINBOND_HH
#define WINBOND_HH

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
	WinBond(Board::DigitalPin cs);

	//TODO use bitfields for status

	//TODO check order of fields is right (potential endianness issue))
	struct Device
	{
		uint8_t manufacturer_ID;
		uint8_t device_ID;
	};
	
	uint16_t status();
	void set_status(uint16_t status);
	bool wait_until_ready(uint16_t timeout_ms);

	void power_down();
	void power_up();
	
	Device read_device();
	uint64_t read_unique_ID();
	
	void enable_write();
	void disable_write();

	void erase_sector(uint32_t address);
	void erase_block_32K(uint32_t address);
	void erase_block_64K(uint32_t address);
	void erase_chip();
	
	void write_page(uint32_t address, uint8_t* data, uint8_t size);
	
	uint8_t read_data(uint32_t address);
	void read_data(uint32_t address, uint8_t* data, uint16_t size);
	
private:
	void _send(uint8_t code);
	void _send(uint8_t code, uint32_t address);
	void _send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size);
};

#endif /* WINBOND_HH */

