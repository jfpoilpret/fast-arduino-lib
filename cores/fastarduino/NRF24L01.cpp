#include "NRF24L01.hh"

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

int NRF24L01::send(uint8_t dest, uint8_t port, const void* buf, size_t len) {
	if (buf == 0 && len > 0) return EINVAL;
	if (len > PAYLOAD_MAX) return EMSGSIZE;

	// Setting transmit destination first (needs to ensure standby mode)
	transmit_mode(dest);

	// Write source address and payload to the transmit fifo
	Command command = ((dest != BROADCAST) ? Command::W_TX_PAYLOAD : Command::W_TX_PAYLOAD_NO_ACK);
	start_transfer();
	_status = transfer(uint8_t(command));
	transfer(_addr.device);
	transfer(port);
	transfer((uint8_t*) buf, len);
	end_transfer();

	_trans += 1;

	// Check for auto-acknowledge pipe(0), and address setup and enable
	if (dest != BROADCAST)
	{
		addr_t tx_addr(_addr.network, dest);
		write(Register::RX_ADDR_P0, &tx_addr, sizeof (tx_addr));
		write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1) | _BV(ERX_P0));
	}
	
	// Wait for transmission
	status_t status = 0;
	while (true)
	{
		status = read_status();
		if (status.tx_ds || status.max_rt) break;
		Time::yield();
	}

	bool data_sent = status.tx_ds;
	
	// Check for auto-acknowledge pipe(0) disable
	if (dest != BROADCAST)
		write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1));

	// Reset status bits
	write(Register::STATUS, _BV(TX_DS) | _BV(MAX_RT));
	
	// Read retransmission counter and update
	observe_tx_t observe = read_observe_tx();
	_retrans += observe.arc_cnt;

	// Check that the message was delivered
	if (data_sent) return len;

	// Failed to deliver
	write(Command::FLUSH_TX);
	_drops += 1;

	return EIO;
}

int NRF24L01::recv(uint8_t& src, uint8_t& port, void* buf, size_t size, uint32_t ms)
{
	// Run in receive mode
	receive_mode();

	// Check if there is data available on any pipe
	uint32_t start = Time::millis();
	while (!available())
	{
		if ((ms != 0) && (Time::since(start) > ms))
			return ETIME;
		Time::yield();
	}
	
	// Try and read payload from FIFO
	return read_fifo_payload(src, port, buf, size);
}

void NRF24L01::set_output_power_level(int8_t dBm)
{
	uint8_t pwr = RF_PWR_0DBM;
	if (dBm < -12) pwr = RF_PWR_18DBM;
	else if (dBm < -6) pwr = RF_PWR_12DBM;
	else if (dBm < 0) pwr = RF_PWR_6DBM;
	write(Register::RF_SETUP, RF_DR_2MBPS | pwr);
}
