/**
 * @file NRF24L01P.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2016, Jean-François Poilprêt
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is inspired from NRF 24L01 library of the Arduino Che Cosa project.
 * It was adapted to work with the FastArduino library.
 */

#ifndef NRF24L01_HH
#define NRF24L01_HH

#include "NRF24L01_impl.hh"
#include <util/delay.h>
#include "errors.hh"
#include "time.hh"
#include "INT.hh"

/**
 * SPI device driver for Nordic Semiconductor nRF24L01+ support.
 * nRF24L01+ is a cheap 2.4GHz RX/TX chip.
 *
 * It must be powered at 3.3V maximum but all its input pins are 5V tolerant, hence no
 * level shifting is needed to operate it.
 *
 *                          NRF24L01P
 *                       +------------+
 * (GND)---------------1-|GND         |
 * (3V3)---------------2-|VCC         |
 * (Dn)----------------3-|CE          |
 * (Dn)----------------4-|CSN         |
 * (SCK)---------------5-|SCK         |
 * (MOSI)--------------6-|MOSI        |
 * (MISO)--------------7-|MISO        |
 * (PCIn/EXTn)---------8-|IRQ         |
 *                       +------------+
 * Notes:
 * - IRQ can be linked to any EXT or PCI pin. However, on some AVR chips, some pins will not awaken
 * the MCU from some "deep" sleep modes. Hence one has to think about this when selecting the pin.
 * - CSN is the usual CS pin used by SPI to select the device and can be set to any AVR pin
 * IMPORTANT: PCI pin is not yet supported actually.
 */

//TODO reformat code to comply with fastArduino coding standards
//TODO split in 2 classes to move out configuration and only keep API in this file
//TODO replace all enum with better code (enum class or bitfield structs and unions)
//TODO optimize code as much as can (size first, then speed)
//TODO make IRQ optional (can be useful when limited pins number)
template<Board::ExternalInterruptPin IRQ>
class NRF24L01: public NRF24L01Impl
{
public:
	/**
	 * Default timeout (in ms) to ensure send() is never stuck forever waiting 
	 * for active IRQ (this can happen for an undetermined reason).
	 * 10ms is high enough, considering 15 retransmits * 500us = 7.5ms
	 */
	static const uint32_t DEFAULT_SEND_TIMEOUT = 10;

	/**
	 * Construct NRF transceiver with given channel and pin numbers
	 * for SPI slave select, activity enable and interrupt. Default
	 * in parenthesis (Standard/Mega Arduino/TinyX4).
	 * @param[in] net network address.
	 * @param[in] dev device address.
	 * @param[in] csn spi slave select pin number (default CS0).
	 * @param[in] ce chip enable activates pin number (default GPIO P1-22).
	 */
	NRF24L01(	uint16_t net, uint8_t dev,
				Board::DigitalPin csn, Board::DigitalPin ce);

	/**
	 * Get driver channel.
	 * @return channel.
	 */
	inline uint8_t get_channel() const
	{
		return _channel;
	}

	/**
	 * Get driver network address.
	 * @return network address.
	 */
	inline uint16_t get_network_address() const
	{
		return _addr.network;
	}

	/**
	 * Get driver device address.
	 * @return device address.
	 */
	inline uint8_t get_device_address() const
	{
		return _addr.device;
	}

	/**
	 * Set network and device address. Do not use the broadcast
	 * address(0). Should be used before calling begin().
	 * @param[in] net network address.
	 * @param[in] dev device address.
	 */
	inline void set_address(int16_t net, uint8_t dev)
	{
		_addr.network = net;
		_addr.device = dev;
	}

	/**
	 * Set device transmission channel. Should be used before calling
	 * begin().
	 * @param[in] channel.
	 */
	inline void set_channel(uint8_t channel)
	{
		_channel = channel;
	}

	/**
	 * Start up the device driver. Return true(1) if successful
	 * otherwise false(0).
	 * @param[in] config device configuration (default NULL).
	 */
	void begin();

	/**
	 * Shut down the device driver. Return true(1) if successful
	 * otherwise false(0).
	 */
	inline void end()
	{
		powerdown();
		_irq_signal.disable();
	}

	/**
	 * Set power up mode. Will initiate radio with necessary settings
	 * after power on reset.
	 */
	void powerup();

	/**
	 * Set standby mode.
	 */
	void standby();

	/**
	 * Set power down. Turn off radio and go into low power mode.
	 */
	void powerdown();

	/**
	 * Send message in given buffer, with given number of bytes. Returns
	 * number of bytes sent. Returns error code(-1) if number of bytes
	 * is greater than PAYLOAD_MAX. Return error code(-2) if fails to
	 * set transmit mode. Note that port numbers (128 and higher are
	 * reserved for system protocols).
	 * @param[in] dest destination network address.
	 * @param[in] port device port (or message type).
	 * @param[in] buf buffer to transmit.
	 * @param[in] len number of bytes in buffer.
	 * @return number of bytes send or negative error code.
	 */
	int send(uint8_t dest, uint8_t port, const void* buf, size_t len, uint32_t ms = DEFAULT_SEND_TIMEOUT);

	/**
	 * Receive message and store into given buffer with given maximum
	 * size. The source network address is returned in the parameter src.
	 * Returns error code(-2) if no message is available and/or a
	 * timeout occured. Returns error code(-1) if the buffer size if to
	 * small for incoming message or if the receiver fifo has overflowed.
	 * Otherwise the actual number of received bytes is returned
	 * @param[out] src source network address.
	 * @param[out] port device port (or message type).
	 * @param[in] buf buffer to store incoming message.
	 * @param[in] count maximum number of bytes to receive.
	 * @param[in] ms maximum time out period.
	 * @return number of bytes received or negative error code.
	 */
	int recv(uint8_t& src, uint8_t& port, void* buf, size_t count, uint32_t ms = 0L);

	/**
	 * Set output power level (-30..10 dBm)
	 * @param[in] dBm.
	 */
	void set_output_power_level(int8_t dBm);

	/**
	 * Return number of transmitted messages.
	 * @return transmit count.
	 */
	inline uint16_t get_trans() const
	{
		return _trans;
	}

	/**
	 * Return number of retransmissions.
	 * @return retransmit count.
	 */
	uint16_t get_retrans() const
	{
		return _retrans;
	}

	/**
	 * Return number of dropped messages.
	 * @return drop count.
	 */
	uint16_t get_drops() const
	{
		return _drops;
	}

	/**
	 * Broadcast message in given buffer, with given number of bytes.
	 * Returns number of bytes sent if successful otherwise a negative
	 * error code.
	 * @param[in] port device port (or message type).
	 * @param[in] buf buffer to transmit.
	 * @param[in] len number of bytes in buffer.
	 * @return number of bytes send or negative error code.
	 */
	int broadcast(uint8_t port, const void* buf, size_t len)
	{
		return send(BROADCAST, port, buf, len);
	}

	/**
	 * Return true(1) if the latest received message was a broadcast
	 * otherwise false(0).
	 */
	bool is_broadcast() const
	{
		return (_dest == BROADCAST);
	}

private:
	static const uint8_t DEFAULT_CHANNEL = 64;

	bool wait_for_irq(uint32_t max_ms);

	IOPin _irq;
	INTSignal<IRQ> _irq_signal;
};

template<Board::ExternalInterruptPin IRQ>
NRF24L01<IRQ>::NRF24L01(
		uint16_t net, uint8_t dev,
		Board::DigitalPin csn, Board::DigitalPin ce)
	:	NRF24L01Impl{net, dev, csn, ce},
		_irq{Board::DigitalPin(IRQ), PinMode::INPUT_PULLUP},
		_irq_signal{InterruptTrigger::FALLING_EDGE}
{
}

template<Board::ExternalInterruptPin IRQ>
void NRF24L01<IRQ>::begin()
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
	
	_irq_signal.enable();
}

template<Board::ExternalInterruptPin IRQ>
void NRF24L01<IRQ>::powerup()
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

template<Board::ExternalInterruptPin IRQ>
void NRF24L01<IRQ>::standby()
{
	if (_state == State::STANDBY_STATE) return;
	_ce.clear();
	_state = State::STANDBY_STATE;
}

template<Board::ExternalInterruptPin IRQ>
void NRF24L01<IRQ>::powerdown()
{
	if (_state == State::POWER_DOWN_STATE) return;
	_ce.clear();
	write(Register::CONFIG, _BV(EN_CRC) | _BV(CRCO));
	_state = State::POWER_DOWN_STATE;
}

template<Board::ExternalInterruptPin IRQ>
bool NRF24L01<IRQ>::wait_for_irq(uint32_t max_ms)
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

template<Board::ExternalInterruptPin IRQ>
int NRF24L01<IRQ>::send(uint8_t dest, uint8_t port, const void* buf, size_t len, uint32_t ms) {
	if (buf == 0 && len > 0) return EINVAL;
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
//	wait_for_irq(ms);
	uint32_t start = Time::millis();
	status_t status = 0;
	while (true)
	{
		Time::yield();
		status = read_status();
		if (status.tx_ds || status.max_rt)
			break;
		if ((ms != 0) && (Time::since(start) > ms))
			break;
	}

	bool data_sent = status.tx_ds;
	// Clear IRQ
	write(Register::STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
	
	// Check for auto-acknowledge pipe(0) disable
	if (dest != BROADCAST)
	{
		write(Register::EN_RXADDR, _BV(ERX_P2) | _BV(ERX_P1));
	}

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

template<Board::ExternalInterruptPin IRQ>
int NRF24L01<IRQ>::recv(uint8_t& src, uint8_t& port, void* buf, size_t size, uint32_t ms)
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
		return -1;
	}

	// Check for payload error from device (Tab. 20, pp. 51, R_RX_PL_WID)
	return read_fifo_payload(src, port, buf, size);
}

template<Board::ExternalInterruptPin IRQ>
void NRF24L01<IRQ>::set_output_power_level(int8_t dBm)
{
	uint8_t pwr = RF_PWR_0DBM;
	if (dBm < -12) pwr = RF_PWR_18DBM;
	else if (dBm < -6) pwr = RF_PWR_12DBM;
	else if (dBm < 0) pwr = RF_PWR_6DBM;
	write(Register::RF_SETUP, RF_DR_2MBPS | pwr);
}

#endif /* NRF24L01_HH */
