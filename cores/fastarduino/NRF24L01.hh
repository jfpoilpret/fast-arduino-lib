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
class NRF24L01: public NRF24L01Impl
{
public:
	/**
	 * Default timeout (in ms) to ensure send() is never stuck forever waiting 
	 * for active IRQ (this can happen for an undetermined reason).
	 * 10ms is high enough, considering 15 retransmits * 500us = 7.5ms
	 */
	//TODO Remove if useless, reintroduce if system gets stuck
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
				Board::DigitalPin csn, Board::DigitalPin ce)
		:NRF24L01Impl{net, dev, csn, ce} {}

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
	int send(uint8_t dest, uint8_t port, const void* buf, size_t len);

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
	inline uint16_t get_retrans() const
	{
		return _retrans;
	}

	/**
	 * Return number of dropped messages.
	 * @return drop count.
	 */
	inline uint16_t get_drops() const
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
	inline int broadcast(uint8_t port, const void* buf, size_t len)
	{
		return send(BROADCAST, port, buf, len);
	}

	/**
	 * Return true(1) if the latest received message was a broadcast
	 * otherwise false(0).
	 */
	inline bool is_broadcast() const
	{
		return (_dest == BROADCAST);
	}

private:
	static const uint8_t DEFAULT_CHANNEL = 64;
};

template<Board::ExternalInterruptPin IRQ>
class IRQ_NRF24L01: public NRF24L01
{
public:
	/**
	 * Construct NRF transceiver with given channel and pin numbers
	 * for SPI slave select, activity enable and interrupt. Default
	 * in parenthesis (Standard/Mega Arduino/TinyX4).
	 * @param[in] net network address.
	 * @param[in] dev device address.
	 * @param[in] csn spi slave select pin number (default CS0).
	 * @param[in] ce chip enable activates pin number (default GPIO P1-22).
	 */
	IRQ_NRF24L01(	uint16_t net, uint8_t dev,
					Board::DigitalPin csn, Board::DigitalPin ce)
		:	NRF24L01{net, dev, csn, ce},
			_irq_signal{InterruptTrigger::FALLING_EDGE}
	{
		IOPin irq{Board::DigitalPin(IRQ), PinMode::INPUT_PULLUP};
	}

	inline void begin()
	{
		NRF24L01::begin();
		_irq_signal.enable();
	}

	inline void end()
	{
		_irq_signal.disable();
		NRF24L01::end();
	}

private:
	INTSignal<IRQ> _irq_signal;
};

#endif /* NRF24L01_HH */
