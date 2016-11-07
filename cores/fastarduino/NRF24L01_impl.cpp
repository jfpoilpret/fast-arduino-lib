#include "NRF24L01_impl.hh"
#include "errors.hh"

NRF24L01Impl::NRF24L01Impl(
		uint16_t net, uint8_t dev,
		Board::DigitalPin csn, Board::DigitalPin ce)
	:	SPIDevice{csn},
		_ce{ce, PinMode::OUTPUT, false},
		_addr{net, dev},
		_channel{DEFAULT_CHANNEL},
		_dest{},
		_status{0},
		_state{State::POWER_DOWN_STATE},
		_trans{},
		_retrans{},
		_drops{}
{
}

uint8_t NRF24L01Impl::read(uint8_t cmd)
{
	start_transfer();
	_status = transfer(cmd);
	uint8_t result = transfer(uint8_t(Command::NOP));
	end_transfer();
	return result;
}

void NRF24L01Impl::read(uint8_t cmd, void* buf, size_t size)
{
	start_transfer();
	_status = transfer(cmd);
	transfer((uint8_t*) buf, size, uint8_t(Command::NOP));
	end_transfer();
}

void NRF24L01Impl::write(uint8_t cmd)
{
	start_transfer();
	_status = transfer(cmd);
	end_transfer();
}

void NRF24L01Impl::write(uint8_t cmd, uint8_t data)
{
	start_transfer();
	_status = transfer(cmd);
	transfer(data);
	end_transfer();
}

void NRF24L01Impl::write(uint8_t cmd, const void* buf, size_t size)
{
	start_transfer();
	_status = transfer(cmd);
	transfer((uint8_t*) buf, size);
	end_transfer();
}

NRF24L01Internals::status_t NRF24L01Impl::read_status() 
{
	start_transfer();
	_status = transfer(uint8_t(Command::NOP));
	end_transfer();
	return _status;
}

int NRF24L01Impl::read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t size)
{
	// Check for payload error from device (Tab. 20, pp. 51, R_RX_PL_WID)
	uint8_t count = read(Command::R_RX_PL_WID) - 2;
	if ((count > PAYLOAD_MAX) || (count > size))
	{
		write(Command::FLUSH_RX);
		return EMSGSIZE;
	}
	
	// Data is available, check if this a broadcast or not
	_dest = (read_status().rx_p_no == 1 ? _addr.device : BROADCAST);
	
	// Read the source address, port and payload
	start_transfer();
	_status = transfer(uint8_t(Command::R_RX_PAYLOAD));
	src = transfer(0);
	port = transfer(0);
	transfer((uint8_t*) buf, count, uint8_t(Command::NOP));
	end_transfer();
	return count;
}
