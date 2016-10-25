#ifndef WINBOND_HH
#define WINBOND_HH

#include "SPI.hh"

//TODO Wiring of WinBond IC
// pullups for CS

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
	
	// API
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

