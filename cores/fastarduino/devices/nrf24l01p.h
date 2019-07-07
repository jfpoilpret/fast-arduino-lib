// Important copyright notice:
// Large parts of this file were copied from Mikael Patel's Cosa library, which copyright appears below
// Content has been adapted to use FastArduino original parts, under Copyright (c) 2016, Jean-Francois Poilpret

/*
 * Copyright (C) 2013-2015, Mikael Patel
 * Copyright (C) 2016-2019, Jean-Francois Poilpret
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
 * This file was originally part of the Arduino Che Cosa project,
 * it has been adapted and refactored to FastArduino library.
 */

/// @cond api

/**
 * @file
 * API to handle "nRF24L01+" chip that allows bi-directional wireless communication
 * in the 2.4GHz band.
 * These chips provide a cheap way to implement wireless communication between 2
 * MCU or between 1 MCU and another board (e.g. a Raspberry Pi).
 */
#ifndef NRF24L01_HH
#define NRF24L01_HH

#include <stddef.h>
#include "../utilities.h"
#include "../errors.h"
#include "../time.h"
#include "../spi.h"
#include "../int.h"
#include "nrf24l01p_internals.h"

/**
 * Defines the API for radio-frequency (wireless) communication support.
 * Current support is for Nordic Semiconductor nRF24L01+ chip (SPI-based).
 * API is provided in 2 flavours, IRQ-based or not, through two template classes.
 * IRQ-based flavour is encouraged but is sometimes impossible as it requires an
 * available `board::ExternalInterruptPin` on the MCU.
 */
namespace devices::rf
{
	/**
	 * SPI device driver for Nordic Semiconductor nRF24L01+ support, without IRQ support.
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
	 *                   --8-|IRQ         |
	 *                       +------------+
	 * Notes:
	 * - CSN is the usual CS pin used by SPI to select the device and can be set to any AVR pin
	 * 
	 * @tparam CSN the `board::DigitalPin` connected to the CSN pin
	 * @tparam CE the `board::DigitalPin` connected to the CE pin
	 * 
	 * @sa IRQ_NRF24L01
	 */
	template<board::DigitalPin CSN, board::DigitalPin CE> class NRF24L01 : public spi::SPIDevice<CSN>
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
		 * Construct NRF transceiver with given channel and pin numbers
		 * for SPI slave select, activity enable and interrupt. Default
		 * in parenthesis (Standard/Mega Arduino/TinyX4).
		 * @param[in] net network address.
		 * @param[in] dev device address.
		 * @param[in] csn spi slave select pin number (default CS0).
		 * @param[in] ce chip enable activates pin number (default GPIO P1-22).
		 */
		NRF24L01(uint16_t net, uint8_t dev);

		/**
		 * Get driver channel.
		 * @return channel.
		 */
		uint8_t get_channel() const
		{
			return channel_;
		}

		/**
		 * Get driver network address.
		 * @return network address.
		 */
		uint16_t get_network_address() const
		{
			return addr_.network;
		}

		/**
		 * Get driver device address.
		 * @return device address.
		 */
		uint8_t get_device_address() const
		{
			return addr_.device;
		}

		/**
		 * Set network and device address. Do not use the broadcast
		 * address(0). Should be used before calling begin().
		 * @param[in] net network address.
		 * @param[in] dev device address.
		 */
		void set_address(int16_t net, uint8_t dev)
		{
			addr_.network = net;
			addr_.device = dev;
		}

		/**
		 * Set device transmission channel. Should be used before calling
		 * begin().
		 * @param[in] channel.
		 */
		void set_channel(uint8_t channel)
		{
			channel_ = channel;
		}

		/**
		 * Start up the device driver. This must be called before any transmission
		 * or reception can take place.
		 */
		void begin();

		/**
		 * Shut down the device driver.
		 */
		void end()
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
		 * Send message with given object reference.
		 * 
		 * @tparam T the type of @p buf (the object to transfer)
		 * @tparam TREF the type passed for @p buf, typically a const reference 
		 * to type @p T
		 * 
		 * @param[in] dest destination network address.
		 * @param[in] port device port (or message type).
		 * @param[in] buf reference of object to transmit.
		 * @return number of bytes send or negative error code.
		 * @retval errors::EMSGSIZE if @p len > `PAYLOAD_MAX`
		 * @retval errors::EIO if a transmission failure happened
		 * 
		 * @sa errors::EMSGSIZE
		 * @sa errors::EIO
		 */
		template<typename T, typename TREF = const T&>
		int send(uint8_t dest, uint8_t port, TREF buf)
		{
			return send_(dest, port, (const uint8_t*) &buf, sizeof(T));
		}

		/**
		 * Send an empty message.
		 * 
		 * @param[in] dest destination network address.
		 * @param[in] port device port (or message type).
		 * @return number of bytes send or negative error code.
		 * @retval errors::EMSGSIZE if @p len > `PAYLOAD_MAX`
		 * @retval errors::EIO if a transmission failure happened
		 * 
		 * @sa errors::EMSGSIZE
		 * @sa errors::EIO
		 */
		int send(uint8_t dest, uint8_t port)
		{
			return send_(dest, port, nullptr, 0);
		}

		/**
		 * Receive message and store into given object reference.
		 * The source network address is returned in the parameter src.
		 * 
		 * @tparam T the type of @p buf (the object to be received)
		 * @tparam TREF the type passed for @p buf, typically a reference 
		 * to type @p T
		 * 
		 * @param[out] src source network address.
		 * @param[out] port device port (or message type).
		 * @param[out] buf reference to object to fill with received payload;
		 * note that no constructor will get called during this operation, it is
		 * best to use simple `struct` for type @p T.
		 * @param[in] buf buffer to store incoming message.
		 * @param[in] ms maximum time out period.
		 * @return number of bytes received or negative error code.
		 * @retval errors::ETIME if nothing was received and a timeout occurred
		 * after @p ms elapsed
		 * @retval errors::EMSGSIZE if a payload error occurred from the chip
		 * (Tab. 20, pp. 51, R_RX_PL_WID) or the received payload size is bigger
		 * than the requested @p size 
		 * 
		 * @sa errors::ETIME
		 * @sa errors::EMSGSIZE
		 */
		template<typename T, typename TREF = T&>
		int recv(uint8_t& src, uint8_t& port, TREF buf, uint32_t ms = 0L)
		{
			return recv_(src, port, (uint8_t*) &buf, sizeof(T), ms);
		}

		/**
		 * Receive an empty message.
		 * The source network address is returned in the parameter src.
		 * 
		 * @param[out] src source network address.
		 * @param[out] port device port (or message type).
		 * @param[in] ms maximum time out period.
		 * @return number of bytes received or negative error code.
		 * @retval errors::ETIME if nothing was received and a timeout occurred
		 * after @p ms elapsed
		 * @retval errors::EMSGSIZE if a payload error occurred from the chip
		 * (Tab. 20, pp. 51, R_RX_PL_WID) or the received payload size is bigger
		 * than the requested @p size 
		 * 
		 * @sa errors::ETIME
		 * @sa errors::EMSGSIZE
		 */
		int recv(uint8_t& src, uint8_t& port, uint32_t ms = 0L)
		{
			return recv_(src, port, nullptr, 0, ms);
		}

		/**
		 * Set output power level (-30..10 dBm)
		 * @param[in] dBm.
		 */
		void set_output_power_level(int8_t dBm);

		/**
		 * Return number of transmitted messages.
		 * @return transmit count.
		 */
		uint16_t get_trans() const
		{
			return trans_;
		}

		/**
		 * Return number of retransmissions.
		 * @return retransmit count.
		 */
		uint16_t get_retrans() const
		{
			return retrans_;
		}

		/**
		 * Return number of dropped messages.
		 * @return drop count.
		 */
		uint16_t get_drops() const
		{
			return drops_;
		}

		/**
		 * Broadcast message with given object reference.
		 * Returns number of bytes sent if successful otherwise a negative
		 * error code.
		 * 
		 * @tparam T the type of @p buf (the object to transfer)
		 * @tparam TREF the type passed for @p buf, typically a const reference 
		 * to type @p T
		 * 
		 * @param[in] port device port (or message type).
		 * @param[in] buf reference of object to transmit.
		 * @return number of bytes send or negative error code.
		 */
		template<typename T, typename TREF = const T&>
		int broadcast(uint8_t port, TREF buf)
		{
			return send(BROADCAST, port, buf);
		}

		/**
		 * Return true if the latest received message was a broadcast
		 * otherwise false.
		 */
		bool is_broadcast() const
		{
			return (dest_ == BROADCAST);
		}

	protected:
		/// @cond notdocumented
		int send_(uint8_t dest, uint8_t port, const uint8_t* buf, size_t len);
		int recv_(uint8_t& src, uint8_t& port, uint8_t* buf, size_t size, uint32_t ms);
		/// @endcond

		/**
		 * SPI Commands (See chap. 8.3.1, tab. 20, pp. 51).
		 */
		enum class Command : uint8_t
		{
			R_REGISTER = 0x00, //!< Read command and status register.
			W_REGISTER = 0x20, //!< Write command and status register.
			REG_MASK = 0x1f,   //!< Mask register address (5b).

			R_RX_PAYLOAD = 0x61, //!< Read RX payload.
			W_TX_PAYLOAD = 0xa0, //!< Write TX payload.

			FLUSH_TX = 0xe1, //!< Flush TX FIFO.
			FLUSH_RX = 0xe2, //!< Flush RX FIFO.

			REUSE_TX_PL = 0xe3, //!< Reuse last transmitted payload.

			R_RX_PL_WID = 0x60, //!< Read RX payload width.

			W_ACK_PAYLOAD = 0xa8, //!< Write TX payload with ACK (3 bit addr).
			PIPE_MASK = 0x07,	 //!< Mask pipe address.

			W_TX_PAYLOAD_NO_ACK = 0xb0, //!< Disable AUTOACK on this specific packet.
			NOP = 0xff					//!< No operation, return status.
		};

		/**
		 * NRF transceiver registers map (See chap. 9, tab. 28, pp. 57).
		 */
		enum class Register : uint8_t
		{
			CONFIG = 0x00,		//!< Configuration register.
			EN_AA = 0x01,		//!< Enable auto acknowledgement.
			EN_RXADDR = 0x02,   //!< Enable rx addresses.
			SETUP_AW = 0x03,	//!< Setup of address width.
			SETUP_RETR = 0x04,  //!< Setup of auto retransmission.
			RF_CH = 0x05,		//!< RF channel.
			RF_SETUP = 0x06,	//!< RF setup register.
			STATUS = 0x07,		//!< Status register.
			OBSERVE_TX = 0x08,  //!< Transmit observe register.
			RPD = 0x09,			//!< Received power detector.
			RX_ADDR_P0 = 0x0a,  //!< Receive address data pipe 0.
			RX_ADDR_P1 = 0x0b,  //!< - data pipe 1.
			RX_ADDR_P2 = 0x0c,  //!< - data pipe 2.
			RX_ADDR_P3 = 0x0d,  //!< - data pipe 3.
			RX_ADDR_P4 = 0x0e,  //!< - data pipe 4.
			RX_ADDR_P5 = 0x0f,  //!< - data pipe 5.
			TX_ADDR = 0x10,		//!< Transmit address.
			RX_PW_P0 = 0x11,	//!< Number of bytes in RX payload in data pipe 0.
			RX_PW_P1 = 0x12,	//!< - data pipe 1.
			RX_PW_P2 = 0x13,	//!< - data pipe 2.
			RX_PW_P3 = 0x14,	//!< - data pipe 3.
			RX_PW_P4 = 0x15,	//!< - data pipe 4.
			RX_PW_P5 = 0x16,	//!< - data pipe 5.
			FIFO_STATUS = 0x17, //!< FIFO status register.
			DYNPD = 0x1c,		//!< Enable dynamic payload length.
			FEATURE = 0x1d		//!< Feature register.
		};

		/**
		 * Register OBSERVE_TX data type, performance statistics.
		 */
		union observe_tx_t
		{
			uint8_t as_byte; //!< Byte representation of performance statistics.

			struct
			{
				uint8_t arc_cnt : 4;  //!< Count retransmitted packets.
				uint8_t plos_cnt : 4; //!< Count lost packets.
			};

			/**
			 * Construct transmitter performance statistics from register
			 * reading.
			 * @param[in] value register reading.
			 */
			explicit observe_tx_t(uint8_t value) INLINE : as_byte{value} {}
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
				uint8_t rx_full : 1;  //!< RX FIFO full flag.
				uint8_t reserved1 : 2;
				uint8_t tx_empty : 1; //!< TX FIFO empty flag.
				uint8_t tx_full : 1;  //!< TX FIFO full flag.
				uint8_t tx_reuse : 1; //!< Reuse last transmitted data packat.
				uint8_t reserved2 : 1;
			};

			/**
			 * Construct transmitter queue status from register reading.
			 * @param[in] value register reading.
			 */
			explicit fifo_status_t(uint8_t value) INLINE : as_byte{value} {}
		};

		/**
		 * Network address together with port.
		 */
		struct addr_t
		{
			uint8_t device;   //!< Device address (LSB).
			uint16_t network; //!< Network address.

			addr_t(uint16_t net, uint8_t dev) INLINE : device{dev}, network{net} {}
		};

		/**
		 * NRF transceiver states (See chap. 6.1.1, fig. 4, pp. 22).
		 */
		enum class State : uint8_t
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
				uint8_t tx_full : 1; //!< TX FIFO full.
				uint8_t rx_p_no : 3; //!< Data pipe number for available payload.
				uint8_t max_rt : 1;  //!< Max number of TX retransmit interrupt.
				uint8_t tx_ds : 1;   //!< Data send TX FIFO interrupt.
				uint8_t rx_dr : 1;   //!< Data ready RX FIFO interrupt.
				uint8_t reserved : 1;
			};

			/**
			 * Construct status from register reading.
			 * @param[in] value register reading.
			 */
			explicit status_t(uint8_t value) INLINE : as_byte{value} {}
		};

		/// @cond notdocumented
		// Lowest-level methods to access NRF24L01 device
		uint8_t read(uint8_t cmd)
		{
			this->start_transfer();
			status_ = status_t{this->transfer(cmd)};
			uint8_t result = this->transfer(uint8_t(Command::NOP));
			this->end_transfer();
			return result;
		}
		void write(uint8_t cmd)
		{
			this->start_transfer();
			status_ = status_t{this->transfer(cmd)};
			this->end_transfer();
		}
		void write(uint8_t cmd, uint8_t data)
		{
			this->start_transfer();
			status_ = status_t{this->transfer(cmd)};
			this->transfer(data);
			this->end_transfer();
		}
		void write(uint8_t cmd, const uint8_t* buf, size_t size)
		{
			this->start_transfer();
			status_ = status_t{this->transfer(cmd)};
			this->transfer((uint8_t*) buf, size);
			this->end_transfer();
		}

		// Command-level methods to access NRF24L01 device
		uint8_t read_command(Command cmd)
		{
			return read(uint8_t(cmd));
		}
		void write_command(Command cmd)
		{
			write(uint8_t(cmd));
		}

		// Mid-level methods to access NRF24L01 device registers
		uint8_t read_register(Register reg)
		{
			return read(uint8_t(Command::R_REGISTER) | uint8_t(uint8_t(Command::REG_MASK) & uint8_t(reg)));
		}
		void write_register(Register reg, uint8_t data)
		{
			write(uint8_t(Command::W_REGISTER) | uint8_t(uint8_t(Command::REG_MASK) & uint8_t(reg)), data);
		}
		void write_register(Register reg, const uint8_t* buf, size_t size)
		{
			write(uint8_t(Command::W_REGISTER) | uint8_t(uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
		}

		status_t read_status()
		{
			this->start_transfer();
			status_ = status_t{this->transfer(uint8_t(Command::NOP))};
			this->end_transfer();
			return status_;
		}

		void transmit_mode(uint8_t dest);
		void receive_mode();

		bool available();
		int read_fifo_payload(uint8_t& src, uint8_t& port, uint8_t* buf, size_t count);
		fifo_status_t read_fifo_status()
		{
			return fifo_status_t{read_register(Register::FIFO_STATUS)};
		}
		observe_tx_t read_observe_tx()
		{
			return observe_tx_t{read_register(Register::OBSERVE_TX)};
		}
		/// @endcond

	private:
		static const uint8_t DEFAULT_CHANNEL = 64;

		typename gpio::FastPinType<CE>::TYPE ce_;

		addr_t addr_;	 //!< Current network and device address.
		uint8_t channel_; //!< Current channel (device dependent.
		uint8_t dest_;	//!< Latest message destination device address.

		status_t status_; //!< Latest status.
		State state_;	 //!< Transceiver state.

		uint16_t trans_;   //!< Send count.
		uint16_t retrans_; //!< Retransmittion count.
		uint16_t drops_;   //!< Dropped messages.
	};

	/**
	 * SPI device driver for Nordic Semiconductor nRF24L01+ support, with IRQ.
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
	 * - IRQ can normally be linked to any EXT or PCI pin. However, on some AVR chips, 
	 * some pins will not awaken the MCU from some "deep" sleep modes. Hence one has 
	 * to think about this when selecting the pin.
	 * - CSN is the usual CS pin used by SPI to select the device and can be set to any AVR pin
	 * IMPORTANT: PCI pin is not yet supported actually.
	 * 
	 * @tparam CSN the `board::DigitalPin` connected to the CSN pin
	 * @tparam CE the `board::DigitalPin` connected to the CE pin
	 * @tparam IRQ the `board::ExternalInterruptPin` connected to the IRQ pin
	 * 
	 * @sa NRF24L01
	 */
	template<board::DigitalPin CSN, board::DigitalPin CE, board::ExternalInterruptPin IRQ>
	class IRQ_NRF24L01 : public NRF24L01<CSN, CE>
	{
	public:
		/**
		 * Construct NRF transceiver with given channel and pin numbers
		 * for SPI slave select, activity enable and interrupt. Default
		 * in parenthesis (Standard/Mega Arduino/TinyX4).
		 * @param[in] net network address.
		 * @param[in] dev device address.
		 */
		IRQ_NRF24L01(uint16_t net, uint8_t dev)
			: NRF24L01<CSN, CE>{net, dev}, irq_signal_{interrupt::InterruptTrigger::FALLING_EDGE}
		{
			gpio::FastPinType<board::EXT_PIN<IRQ>()>::set_mode(gpio::PinMode::INPUT_PULLUP);
		}

		/**
		 * Start up the device driver. This must be called before any transmission
		 * or reception can take place.
		 */
		void begin()
		{
			NRF24L01<CSN, CE>::begin();
			irq_signal_.enable();
		}

		/**
		 * Shut down the device driver.
		 */
		void end()
		{
			irq_signal_.disable();
			NRF24L01<CSN, CE>::end();
		}

	private:
		interrupt::INTSignal<IRQ> irq_signal_;
	};

	template<board::DigitalPin CSN, board::DigitalPin CE>
	NRF24L01<CSN, CE>::NRF24L01(uint16_t net, uint8_t dev)
		: ce_{gpio::PinMode::OUTPUT, false}, addr_{net, dev}, channel_{DEFAULT_CHANNEL}, dest_{}, status_{0},
			state_{State::POWER_DOWN_STATE}, trans_{}, retrans_{}, drops_{}
	{
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::begin()
	{
		using namespace nrf24l01p_internals;
		// Setup hardware features, channel, bitrate, retransmission, dynamic payload
		write_register(Register::FEATURE, (bits::BV8(EN_DPL, EN_ACK_PAY, EN_DYN_ACK)));
		write_register(Register::RF_CH, channel_);
		write_register(Register::RF_SETUP, RF_DR_2MBPS | RF_PWR_0DBM);
		write_register(Register::SETUP_RETR, uint8_t(DEFAULT_ARD << ARD) | uint8_t(DEFAULT_ARC << ARC));
		write_register(Register::DYNPD, DPL_PA);

		// Setup hardware receive pipes address; network (16-bit), device (8-bit)
		// P0: auto-acknowledge (see set_transmit_mode)
		// P1: node address<network:device> with auto-acknowledge
		// P2: broadcast<network:0>
		addr_t rx_addr = addr_;
		write_register(Register::SETUP_AW, AW_3BYTES);
		write_register(Register::RX_ADDR_P1, (const uint8_t*) &rx_addr, sizeof(rx_addr));
		write_register(Register::RX_ADDR_P2, BROADCAST);
		write_register(Register::EN_RXADDR, bits::BV8(ERX_P2, ERX_P1));
		write_register(Register::EN_AA, bits::BV8(ENAA_P1, ENAA_P0));

		// Ready to go
		powerup();
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::powerup()
	{
		using namespace nrf24l01p_internals;
		if (state_ != State::POWER_DOWN_STATE) return;
		ce_.clear();

		// Setup configuration for powerup and clear interrupts
		write_register(Register::CONFIG, bits::BV8(EN_CRC, CRCO, PWR_UP));
		time::delay_ms(Tpd2stby_ms);
		state_ = State::STANDBY_STATE;

		// Flush status
		write_register(Register::STATUS, bits::BV8(RX_DR, TX_DS, MAX_RT));
		write_command(Command::FLUSH_TX);
		write_command(Command::FLUSH_RX);
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::standby()
	{
		if (state_ == State::STANDBY_STATE) return;
		ce_.clear();
		state_ = State::STANDBY_STATE;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::powerdown()
	{
		using namespace nrf24l01p_internals;
		if (state_ == State::POWER_DOWN_STATE) return;
		ce_.clear();
		write_register(Register::CONFIG, bits::BV8(EN_CRC, CRCO));
		state_ = State::POWER_DOWN_STATE;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE>
	int NRF24L01<CSN, CE>::send_(uint8_t dest, uint8_t port, const uint8_t* buf, size_t len)
	{
		using namespace nrf24l01p_internals;
		// Note buf == 0 and len == 0 is perfectly acceptable
		if ((buf == nullptr) && (len > 0)) return errors::EINVAL;
		if (len > PAYLOAD_MAX) return errors::EMSGSIZE;

		// Setting transmit destination first (needs to ensure standby mode)
		transmit_mode(dest);

		// Write source address and payload to the transmit fifo
		Command command = ((dest != BROADCAST) ? Command::W_TX_PAYLOAD : Command::W_TX_PAYLOAD_NO_ACK);
		this->start_transfer();
		status_ = status_t{this->transfer(uint8_t(command))};
		this->transfer(addr_.device);
		this->transfer(port);
		this->transfer(buf, len);
		this->end_transfer();

		trans_ += 1;

		// Check for auto-acknowledge pipe(0), and address setup and enable
		if (dest != BROADCAST)
		{
			addr_t tx_addr(addr_.network, dest);
			write_register(Register::RX_ADDR_P0, (const uint8_t*) &tx_addr, sizeof(tx_addr));
			write_register(Register::EN_RXADDR, bits::BV8(ERX_P2, ERX_P1, ERX_P0));
		}

		// Wait for transmission
		status_t status = status_t{0};
		while (true)
		{
			status = read_status();
			if (status.tx_ds || status.max_rt) break;
			time::yield();
		}

		bool data_sent = status.tx_ds;

		// Check for auto-acknowledge pipe(0) disable
		if (dest != BROADCAST) write_register(Register::EN_RXADDR, bits::BV8(ERX_P2, ERX_P1));

		// Reset status bits
		write_register(Register::STATUS, bits::BV8(TX_DS, MAX_RT));

		// Read retransmission counter and update
		observe_tx_t observe = read_observe_tx();
		retrans_ += observe.arc_cnt;

		// Check that the message was delivered
		if (data_sent) return len;

		// Failed to deliver
		write_command(Command::FLUSH_TX);
		drops_ += 1;

		return errors::EIO;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE>
	int NRF24L01<CSN, CE>::recv_(uint8_t& src, uint8_t& port, uint8_t* buf, size_t size, uint32_t ms)
	{
		// Run in receive mode
		receive_mode();

		// Check if there is data available on any pipe
		uint32_t start = time::millis();
		while (!available())
		{
			if ((ms != 0) && (time::since(start) > ms)) return errors::ETIME;
			time::yield();
		}

		// Try and read payload from FIFO
		return read_fifo_payload(src, port, buf, size);
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::set_output_power_level(int8_t dBm)
	{
		using namespace nrf24l01p_internals;
		uint8_t pwr;
		if (dBm < -12)
			pwr = RF_PWR_18DBM;
		else if (dBm < -6)
			pwr = RF_PWR_12DBM;
		else if (dBm < 0)
			pwr = RF_PWR_6DBM;
		else
			pwr = RF_PWR_0DBM;
		write_register(Register::RF_SETUP, RF_DR_2MBPS | pwr);
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::transmit_mode(uint8_t dest)
	{
		using namespace nrf24l01p_internals;
		// Setup primary transmit address
		addr_t tx_addr(addr_.network, dest);
		write_register(Register::TX_ADDR, (const uint8_t*) &tx_addr, sizeof(tx_addr));

		// Trigger the transmitter mode
		if (state_ != State::TX_STATE)
		{
			ce_.clear();
			write_register(Register::CONFIG, bits::BV8(EN_CRC, CRCO, PWR_UP));
			ce_.set();
		}

		// Wait for the transmitter to become active
		if (state_ == State::STANDBY_STATE) time::delay_us(Tstby2a_us);
		state_ = State::TX_STATE;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> void NRF24L01<CSN, CE>::receive_mode()
	{
		using namespace nrf24l01p_internals;
		// Check already in receive mode
		if (state_ == State::RX_STATE) return;

		// Configure primary receiver mode
		write_register(Register::CONFIG, bits::BV8(EN_CRC, CRCO, PWR_UP, PRIM_RX));
		ce_.set();
		if (state_ == State::STANDBY_STATE) time::delay_us(Tstby2a_us);
		state_ = State::RX_STATE;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE> bool NRF24L01<CSN, CE>::available()
	{
		// Check the receiver fifo
		if (read_fifo_status().rx_empty) return false;

		// Sanity check the size of the payload. Might require a flush
		if (read_command(Command::R_RX_PL_WID) <= DEVICE_PAYLOAD_MAX) return true;
		write_command(Command::FLUSH_RX);
		return false;
	}

	template<board::DigitalPin CSN, board::DigitalPin CE>
	int NRF24L01<CSN, CE>::read_fifo_payload(uint8_t& src, uint8_t& port, uint8_t* buf, size_t size)
	{
		// Check for payload error from device (Tab. 20, pp. 51, R_RX_PL_WID)
		uint8_t count = read_command(Command::R_RX_PL_WID) - 2;
		if ((count > PAYLOAD_MAX) || (count > size))
		{
			write_command(Command::FLUSH_RX);
			return errors::EMSGSIZE;
		}

		// Data is available, check if this a broadcast or not
		dest_ = (read_status().rx_p_no == 1 ? addr_.device : BROADCAST);

		// Read the source address, port and payload
		this->start_transfer();
		status_ = status_t{this->transfer(uint8_t(Command::R_RX_PAYLOAD))};
		src = this->transfer(0);
		port = this->transfer(0);
		this->transfer(buf, count, uint8_t(Command::NOP));
		this->end_transfer();
		return count;
	}
}

#endif /* NRF24L01_HH */
/// @endcond
