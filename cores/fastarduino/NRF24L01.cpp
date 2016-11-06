#include "NRF24L01.hh"
#include <util/delay.h>
#include "errors.hh"
#include "time.hh"

template<Board::ExternalInterruptPin IRQ>
NRF24L01::NRF24L01(	uint16_t net, uint8_t dev,
			Board::DigitalPin csn, Board::DigitalPin ce)
	:	SPIDevice{csn},
		_ce{PinMode::OUTPUT, false},
		_irq{PinMode::INPUT_PULLUP},
		_addr{net, dev},
		_channel{DEFAULT_CHANNEL},
		_status{},
		_state{State::POWER_DOWN_STATE},
		_dest{},
		_trans{},
		_retrans{},
		_drops{}
{
		INTSignal<IRQ> signal{InterruptTrigger::FALLING_EDGE};
		signal.enable();
}

void NRF24L01::begin()
{
	// Setup hardware features, channel, bitrate, retransmission, dynamic payload
	write(Register::FEATURE, (_BV(EN_DPL) | _BV(EN_ACK_PAY) | _BV(EN_DYN_ACK)));
	write(Register::RF_CH, _channel);
	write(Register::RF_SETUP, RF_DR_2MBPS | RF_PWR_0DBM);
	write(Register::SETUP_RETR, (DEFAULT_ARD << ARD) | (DEFAULT_ARC << ARC));
	write(Register::DYNPD, DPL_PA);

	// Setup hardware receive pipes address; network (16-bit), device (8-bit)
	// P0: auto-acknowledge (see set_transmit_mode)
	// P1: node address<network:device> with auto-acknowledge
	// P2: broadcast<network:0>
	addr_t rx_addr = _addr;
	write(Register::SETUP_AW, AW_3BYTES);
	write(Register::RX_ADDR_P1, &rx_addr, sizeof (rx_addr));
	write(Register::RX_ADDR_P2, BROADCAST);
	write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1));
	write(Register::EN_AA, _BV(ENAA_P1) | _BV(ENAA_P0));

	// Ready to go
	powerup();
}

uint8_t NRF24L01::read(uint8_t cmd)
{
	start_transfer();
	_status = transfer(cmd);
	uint8_t result = transfer(uint8_t(Command::NOP));
	end_transfer();
	return result;
}

void NRF24L01::read(uint8_t cmd, void* buf, size_t size)
{
	start_transfer();
	_status = transfer(cmd);
	transfer((uint8_t*) buf, size, uint8_t(Command::NOP));
	end_transfer();
}

void NRF24L01::write(uint8_t cmd)
{
	start_transfer();
	_status = transfer(cmd);
	end_transfer();
}

void NRF24L01::write(uint8_t cmd, uint8_t data)
{
	start_transfer();
	_status = transfer(cmd);
	transfer(data);
	end_transfer();
}

void NRF24L01::write(uint8_t cmd, const void* buf, size_t size)
{
	start_transfer();
	_status = transfer(cmd);
	transfer((uint8_t*) buf, size);
	end_transfer();
}

NRF24L01::status_t NRF24L01::read_status() 
{
	start_transfer();
	_status = transfer(uint8_t(Command::NOP));
	end_transfer();
	return _status;
}

void NRF24L01::powerup()
{
	if (_state != State::POWER_DOWN_STATE) return;
	_ce.clear();

	// Setup configuration for powerup and clear interrupts
	write(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP));
	Time::delay_ms(Tpd2stby_ms);
	_state = State::STANDBY_STATE;

	// Flush status
	write(Register::STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
	write(Command::FLUSH_TX);
	write(Command::FLUSH_RX);
}

void NRF24L01::standby()
{
	if (_state == State::STANDBY_STATE) return;
	_ce.clear();
	_state = State::STANDBY_STATE;
}

void NRF24L01::powerdown()
{
	if (_state == State::POWER_DOWN_STATE) return;
	_ce.clear();
	write(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO));
	_state = State::POWER_DOWN_STATE;
}

bool NRF24L01::wait_for_irq(uint32_t max_ms)
{
	uint32_t start = Time::millis();
	while (true)
	{
		if (!_irq.value()) break;
		if ((max_ms != 0) && (Time::since(start) > max_ms))
			return false;
		Time::yield();
	}
	return true;
}

int NRF24L01::send(uint8_t dest, uint8_t port, const void* buf, size_t len, uint32_t ms) {
	if (buf == 0) return EINVAL;
	if (len > PAYLOAD_MAX) return EMSGSIZE;

	// Setting transmit destination first (needs to ensure standby mode)
	standby();
	// Trigger the transmitter mode
	write(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP));
	// Setup primary transmit address
	addr_t tx_addr(_addr.network, dest);
	write(Register::TX_ADDR, &tx_addr, sizeof (tx_addr));
	// Write source address and payload to the transmit fifo
	Command command = ((dest != BROADCAST) ? Command::W_TX_PAYLOAD : Command::W_TX_PAYLOAD_NO_ACK);
	start_transfer();
	_status = transfer(uint8_t(command));
	transfer(_addr.device);
	transfer(port);
	transfer((uint8_t*) buf, len);
	end_transfer();

	// Check for auto-acknowledge pipe(0), and address setup and enable
	if (dest != BROADCAST)
	{
		write(Register::RX_ADDR_P0, &tx_addr, sizeof (tx_addr));
		write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1) | _BV(ERX_P0));
	}
	
	_trans += 1;

	// Pulse CE for 10us in order to start transmission
	_ce.set();
	Time::delay_us(Thce_us);
	_ce.clear();
	
	// Wait for transmission
	wait_for_irq(ms);

	// Clear IRQ
	status_t status = read_status();
	write(Register::STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
	
	// Check for auto-acknowledge pipe(0) disable
	if (dest != BROADCAST)
	{
		write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1));
	}

	bool data_sent = status.tx_ds;

	// Read retransmission counter and update
	observe_tx_t observe = read_observe_tx();
	_retrans += observe.arc_cnt;

	// Check that the message was delivered
	if (data_sent) return len;

	// Failed to delivery
	write(Command::FLUSH_TX);
	_drops += 1;

	return EIO;
}

int NRF24L01::recv(uint8_t& src, uint8_t& port, void* buf, size_t size, uint32_t ms)
{
	// First check if there is some payload in RX FIFO
	if (!read_fifo_status().rx_empty)
	{
		return read_fifo_payload(src, port, buf, size);
	}
	
	// Run in receiver mode
	if (_state != State::RX_STATE)
	{
		standby();
		// Configure primary receiver mode
		write(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP) | _BV(PRIM_RX));
		_ce.set();
		Time::delay_us(Tstby2a_us);
		_state = State::RX_STATE;
	}

	// Check if there is data available on any pipe
	if (!wait_for_irq(ms)) return ETIME;
	status_t status =  read_status();
	
	// Go to standby mode and clear IRQ
	standby();
	write(Register::STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

	// Check expected status (RX_DR)
	if (!status.rx_dr) return EIO;

	// Check the receiver fifo
	if (read_fifo_status().rx_empty)
	{
		// UNEXPECTED BRANCH! TODO FIXME
	}

	// Check for payload error from device (Tab. 20, pp. 51, R_RX_PL_WID)
	return read_fifo_payload(src, port, buf, size);
}

int NRF24L01::read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t size)
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

void NRF24L01::set_output_power_level(int8_t dBm)
{
	uint8_t pwr = RF_PWR_0DBM;
	if (dBm < -12) pwr = RF_PWR_18DBM;
	else if (dBm < -6) pwr = RF_PWR_12DBM;
	else if (dBm < 0) pwr = RF_PWR_6DBM;
	write(Register::RF_SETUP, RF_DR_2MBPS | pwr);
}


