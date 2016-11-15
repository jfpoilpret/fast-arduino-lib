#include "NRF24L01_impl.hh"
#include "NRF24L01_internals.hh"
#include "errors.hh"
#include "time.hh"

using namespace NRF24L01Internals;

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

void NRF24L01Impl::transmit_mode(uint8_t dest)
{
	// Setup primary transmit address
	addr_t tx_addr(_addr.network, dest);
	write_register(Register::TX_ADDR, &tx_addr, sizeof (tx_addr));
	//DEBUG try local optimization of WRITE command + TX_ADDR register
//	write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(Register::TX_ADDR)), &tx_addr, sizeof (tx_addr));
	
	// Trigger the transmitter mode
	if (_state != State::TX_STATE)
	{
		_ce.clear();
		write_register(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP));
		_ce.set();
	}
	
	// Wait for the transmitter to become active
	if (_state == State::STANDBY_STATE) Time::delay_us(Tstby2a_us);
	_state = State::TX_STATE;
}

void NRF24L01Impl::receive_mode()
{
	// Check already in receive mode
	if (_state == State::RX_STATE) return;

	// Configure primary receiver mode
	write_register(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP) | _BV(PRIM_RX));
	_ce.set();
	if (_state == State::STANDBY_STATE) Time::delay_us(Tstby2a_us);
	_state = State::RX_STATE;
}

bool NRF24L01Impl::available()
{
	// Check the receiver fifo
	if (read_fifo_status().rx_empty) return false;

	// Sanity check the size of the payload. Might require a flush
	if (read_command(Command::R_RX_PL_WID) <= DEVICE_PAYLOAD_MAX) return true;
	write_command(Command::FLUSH_RX);
	return false;
}

NRF24L01Impl::status_t NRF24L01Impl::read_status() 
{
	start_transfer();
	_status = transfer(uint8_t(Command::NOP));
	end_transfer();
	return _status;
}

int NRF24L01Impl::read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t size)
{
	// Check for payload error from device (Tab. 20, pp. 51, R_RX_PL_WID)
	uint8_t count = read_command(Command::R_RX_PL_WID) - 2;
	if ((count > PAYLOAD_MAX) || (count > size))
	{
		write_command(Command::FLUSH_RX);
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
