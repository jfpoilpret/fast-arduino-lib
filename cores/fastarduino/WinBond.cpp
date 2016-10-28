#include "WinBond.hh"

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
	transfer(0x05);
	//TODO add timing check (once RTT is available)
	while (true)
	{
		uint8_t status = transfer(0x00);
		if (!(status & 0x01))
		{
			ready = true;
			break;
		}
	}
	end_transfer();
	return ready;
}

WinBond::Device WinBond::read_device()
{
	Device device;
	_send(0x90, 0, (uint8_t*) &device, sizeof(device));
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

uint8_t WinBond::read_data(uint32_t address)
{
	uint8_t data;
	read_data(address, &data, 1);
	return data;
}

void WinBond::read_data(uint32_t address, uint8_t* data, uint16_t size)
{
	_send(0x03, address, data, size);
}

uint8_t WinBond::_read(uint8_t code)
{
	start_transfer();
	transfer(code);
	uint8_t result = transfer(0);
	end_transfer();
	return result;
}

void WinBond::_send(uint8_t code)
{
	start_transfer();
	transfer(code);
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
