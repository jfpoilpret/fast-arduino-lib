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

//#include "NRF24L01_impl.hh"
#include <stddef.h>
#include <util/delay.h>
#include "utilities.hh"
#include "errors.hh"
#include "time.hh"
#include "SPI.hh"
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

//TODO optimize code as much as can (size first, then speed)
class NRF24L01: public SPI::SPIDevice
{
public:
	/** Broadcast device address. */
	static const uint8_t BROADCAST = 0x00;

	/**
	 * Maximum size of payload on device.
	 */
	static const size_t DEVICE_PAYLOAD_MAX = 32;

	/**
	 * Maximum size of payload. The device allows 32 bytes payload.
	 * The source address one byte and port one byte as header.
	 */
	static const size_t PAYLOAD_MAX = DEVICE_PAYLOAD_MAX - 2;
	
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
	
protected:
	/**
	 * SPI Commands (See chap. 8.3.1, tab. 20, pp. 51).
	 */
	enum class Command: uint8_t
	{
		R_REGISTER = 0x00, //!< Read command and status register.
		W_REGISTER = 0x20, //!< Write command and status register.
		REG_MASK = 0x1f, //!< Mask register address (5b).
		
		R_RX_PAYLOAD = 0x61, //!< Read RX payload.
		W_TX_PAYLOAD = 0xa0, //!< Write TX payload.
		
		FLUSH_TX = 0xe1, //!< Flush TX FIFO.
		FLUSH_RX = 0xe2, //!< Flush RX FIFO.
		
		REUSE_TX_PL = 0xe3, //!< Reuse last transmitted payload.
		
		R_RX_PL_WID = 0x60, //!< Read RX payload width.
		
		W_ACK_PAYLOAD = 0xa8, //!< Write TX payload with ACK (3 bit addr).
		PIPE_MASK = 0x07, //!< Mask pipe address.
		
		W_TX_PAYLOAD_NO_ACK = 0xb0, //!< Disable AUTOACK on this specific packet.
		NOP = 0xff //!< No operation, return status.
	};

	/**
	 * NRF transceiver registers map (See chap. 9, tab. 28, pp. 57).
	 */
	enum class Register: uint8_t
	{
		CONFIG = 0x00, //!< Configuration register.
		EN_AA = 0x01, //!< Enable auto acknowledgement.
		EN_RXADDR = 0x02, //!< Enable rx addresses.
		SETUP_AW = 0x03, //!< Setup of address width.
		SETUP_RETR = 0x04, //!< Setup of auto retransmission.
		RF_CH = 0x05, //!< RF channel.
		RF_SETUP = 0x06, //!< RF setup register.
		STATUS = 0x07, //!< Status register.
		OBSERVE_TX = 0x08, //!< Transmit observe register.
		RPD = 0x09, //!< Received power detector.
		RX_ADDR_P0 = 0x0a, //!< Receive address data pipe 0.
		RX_ADDR_P1 = 0x0b, //!< - data pipe 1.
		RX_ADDR_P2 = 0x0c, //!< - data pipe 2.
		RX_ADDR_P3 = 0x0d, //!< - data pipe 3.
		RX_ADDR_P4 = 0x0e, //!< - data pipe 4.
		RX_ADDR_P5 = 0x0f, //!< - data pipe 5.
		TX_ADDR = 0x10, //!< Transmit address.
		RX_PW_P0 = 0x11, //!< Number of bytes in RX payload in data pipe 0.
		RX_PW_P1 = 0x12, //!< - data pipe 1.
		RX_PW_P2 = 0x13, //!< - data pipe 2.
		RX_PW_P3 = 0x14, //!< - data pipe 3.
		RX_PW_P4 = 0x15, //!< - data pipe 4.
		RX_PW_P5 = 0x16, //!< - data pipe 5.
		FIFO_STATUS = 0x17, //!< FIFO status register.
		DYNPD = 0x1c, //!< Enable dynamic payload length.
		FEATURE = 0x1d //!< Feature register.
	};

	/**
	 * Register OBSERVE_TX data type, performance statistics.
	 */
	union observe_tx_t
	{
		uint8_t as_byte; //!< Byte representation of performance statistics.

		struct
		{
			uint8_t arc_cnt : 4; //!< Count retransmitted packets.
			uint8_t plos_cnt : 4; //!< Count lost packets.
		};

		/**
		 * Construct transmitter performance statistics from register
		 * reading.
		 * @param[in] value register reading.
		 */
		observe_tx_t(uint8_t value) INLINE
		{
			as_byte = value;
		}
	};

	/**
	 * Register FIFO_STATUS data type, transmission queue status.
	 */
	union fifo_status_t
	{
		uint8_t as_byte; //!< Byte representation of fifo status.

		struct
		{
			uint8_t rx_empty : 1; //!< RX FIFO empty flag.
			uint8_t rx_full : 1; //!< RX FIFO full flag.
			uint8_t reserved1 : 2;
			uint8_t tx_empty : 1; //!< TX FIFO empty flag.
			uint8_t tx_full : 1; //!< TX FIFO full flag.
			uint8_t tx_reuse : 1; //!< Reuse last transmitted data packat.
			uint8_t reserved2 : 1;
		};

		/**
		 * Construct transmitter queue status from register reading.
		 * @param[in] value register reading.
		 */
		fifo_status_t(uint8_t value) INLINE
		{
			as_byte = value;
		}
	};

	/**
	 * Network address together with port.
	 */
	struct addr_t
	{
		uint8_t device; //!< Device address (LSB).
		uint16_t network; //!< Network address.

		addr_t(uint16_t net, uint8_t dev) INLINE
		{
			network = net;
			device = dev;
		}
	};

	/**
	 * NRF transceiver states (See chap. 6.1.1, fig. 4, pp. 22).
	 */
	enum class State: uint8_t
	{
		POWER_DOWN_STATE = 0,
		STANDBY_STATE,
		RX_STATE,
		TX_STATE
	};
	
	/**
	 * Register STATUS data type.
	 */
	union status_t
	{
		uint8_t as_byte; //!< Byte representation of status.
		struct
		{
			uint8_t tx_full		:1; //!< TX FIFO full.
			uint8_t rx_p_no		:3; //!< Data pipe number for available payload.
			uint8_t max_rt		:1; //!< Max number of TX retransmit interrupt.
			uint8_t tx_ds		:1; //!< Data send TX FIFO interrupt.
			uint8_t rx_dr		:1; //!< Data ready RX FIFO interrupt.
			uint8_t reserved	:1;
		};

		/**
		 * Construct status from register reading.
		 * @param[in] value register reading.
		 */
		status_t(uint8_t value) INLINE
		{
			as_byte = value;
		}
	};

	// Lowest-level methods to access NRF24L01 device
	inline uint8_t read(uint8_t cmd)
	{
		start_transfer();
		_status = transfer(cmd);
		uint8_t result = transfer(uint8_t(Command::NOP));
		end_transfer();
		return result;
	}
	inline void read(uint8_t cmd, void* buf, size_t size)
	{
		start_transfer();
		_status = transfer(cmd);
		transfer((uint8_t*) buf, size, uint8_t(Command::NOP));
		end_transfer();
	}
	inline void write(uint8_t cmd)
	{
		start_transfer();
		_status = transfer(cmd);
		end_transfer();
	}
	inline void write(uint8_t cmd, uint8_t data)
	{
		start_transfer();
		_status = transfer(cmd);
		transfer(data);
		end_transfer();
	}
	inline void write(uint8_t cmd, const void* buf, size_t size)
	{
		start_transfer();
		_status = transfer(cmd);
		transfer((uint8_t*) buf, size);
		end_transfer();
	}

	// Command-level methods to access NRF24L01 device
	inline uint8_t read_command(Command cmd)
	{
		return read(uint8_t(cmd));
	}
	inline void read_command(Command cmd, void* buf, size_t size)
	{
		read(uint8_t(cmd), buf, size);
	}
	inline void write_command(Command cmd)
	{
		write(uint8_t(cmd));
	}
	inline void write_command(Command cmd, uint8_t data)
	{
		write(uint8_t(cmd), data);
	}
	inline void write_command(Command cmd, const void* buf, size_t size)
	{
		write(uint8_t(cmd), buf, size);
	}

	// Mid-level methods to access NRF24L01 device registers
	inline uint8_t read_register(Register reg)
	{
		return read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)));
	}
	inline void read_register(Register reg, void* buf, size_t size)
	{
		read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}
	inline void write_register(Register reg, uint8_t data)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), data);
	}
	inline void write_register(Register reg, const void* buf, size_t size)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}

	inline status_t read_status()
	{
		start_transfer();
		_status = transfer(uint8_t(Command::NOP));
		end_transfer();
		return _status;
	}

	void transmit_mode(uint8_t dest);
	void receive_mode();

	bool available();
	int read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t count);
	inline fifo_status_t read_fifo_status()
	{
		return read_register(Register::FIFO_STATUS);
	}
	inline observe_tx_t read_observe_tx()
	{
		return read_register(Register::OBSERVE_TX);
	}
		
private:
	static const uint8_t DEFAULT_CHANNEL = 64;
	
	IOPin _ce;

	addr_t _addr; //!< Current network and device address.
	uint8_t _channel; //!< Current channel (device dependent.
	uint8_t _dest; //!< Latest message destination device address.

	status_t _status; //!< Latest status.
	State _state; //!< Transceiver state.

	uint16_t _trans; //!< Send count.
	uint16_t _retrans; //!< Retransmittion count.
	uint16_t _drops; //!< Dropped messages.
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
