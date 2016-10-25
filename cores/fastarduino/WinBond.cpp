#include <util/delay.h>

#include "WinBond.hh"

WinBond::WinBond(Board::DigitalPin cs): SPIDevice{cs, SPI::ChipSelect::ACTIVE_LOW, SPI::ClockRate::CLOCK_DIV_2}
{
}

uint16_t WinBond::status()
{
	start_transfer();
	uint8_t status1 = transfer(0x05);
	end_transfer();
	start_transfer();
	uint8_t status2 = transfer(0x35);
	end_transfer();
	return status2 << 8 | status1;
}

void WinBond::set_status(uint16_t status)
{
	start_transfer();
	transfer(status);
	transfer(status >> 8);
	end_transfer();
}

bool WinBond::wait_until_ready(uint16_t timeout_ms)
{
	bool ready = false;
	start_transfer();
	//TODO add timing check (once RTT is available)
	while (true)
	{
		uint8_t status = transfer(0x05);
		if (!(status & 0x01))
		{
			ready = true;
			break;
		}
	}
	end_transfer();
	return ready;
}

void WinBond::power_down()
{
	_send(0xB9);
}

void WinBond::power_up()
{
	_send(0xB9);
	_delay_us(3);
}

WinBond::Device WinBond::read_device()
{
	Device device;
	_send(0x90, 0, (uint8_t*) &device, sizeof(device));
//	transfer(&device, sizeof(device), 0);
	return device;
}

uint64_t WinBond::read_unique_ID()
{
	uint8_t buffer[9];
	_send(0x4B, 0, buffer, 9);
	//FIXME check if we need to exchange bytes (endianness)
	uint64_t id = *((uint64_t*) &buffer[1]);
	return id;
}

void WinBond::enable_write()
{
	_send(0x06);
}

void WinBond::disable_write()
{
	_send(0x04);
}

void WinBond::erase_sector(uint32_t address)
{
	_send(0x20, address);
}

void WinBond::erase_block_32K(uint32_t address)
{
	_send(0x52, address);
}

void WinBond::erase_block_64K(uint32_t address)
{
	_send(0xD8, address);
}

void WinBond::erase_chip()
{
	_send(0xC7);
}

void WinBond::write_page(uint32_t address, uint8_t* data, uint8_t size)
{
	_send(0x02, address, data, (size == 0 ? 256 : size));
}

uint8_t WinBond::read_data(uint32_t address)
{
	uint8_t data;
	read_data(address, &data, 1);
	return data;
}

void WinBond::read_data(uint32_t address, uint8_t* data, uint16_t size)
{
	_send(0x03, address, data, size);
//	transfer(data, size, 0);
}

void WinBond::_send(uint8_t code)
{
	start_transfer();
	transfer(code);
	end_transfer();
}

void WinBond::_send(uint8_t code, uint32_t address)
{
	start_transfer();
	transfer(code);
	transfer(address >> 16);
	transfer(address >> 8);
	transfer(address);
	end_transfer();
}

void WinBond::_send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size)
{
	start_transfer();
	transfer(code);
	transfer(address >> 16);
	transfer(address >> 8);
	transfer(address);
	transfer(data, size);
	end_transfer();
}
