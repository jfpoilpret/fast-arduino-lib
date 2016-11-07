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

#ifndef NRF24L01_INTERNALS_HH
#define NRF24L01_INTERNALS_HH

#include <stddef.h>
#include "utilities.hh"

class NRF24L01Internals
{
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
	 * Register CONFIG bitfields, configuration.
	 */
	enum 
	{
		MASK_RX_DR = 6, //!< Mask interrupt caused by RX_DR.
		MASK_TX_DS = 5, //!< Mask interrupt caused by TX_DS.
		MASK_MAX_RT = 4, //!< Mask interrupt caused byt MAX_RT.
		EN_CRC = 3, //!< Enable CRC.
		CRCO = 2, //!< CRC encoding scheme (2/1 bytes CRC).
		PWR_UP = 1, //!< Power up/down.
		PRIM_RX = 0, //!< RX/TX control (PRX/PTX).
	};

	enum
	{
		POWER_DOWN = 0, //!< PWR_UP bit settings.
		POWER_UP = _BV(PWR_UP)
	} __attribute__((packed));

	/**
	 * Register EN_AA bitfields, auto acknowledgement.
	 */
	enum
	{
		ENAA_P5 = 5, //!< Enable auto acknowledgement data pipe 5.
		ENAA_P4 = 4, //!< - data pipe 4.
		ENAA_P3 = 3, //!< - data pipe 3.
		ENAA_P2 = 2, //!< - data pipe 2.
		ENAA_P1 = 1, //!< - data pipe 1.
		ENAA_P0 = 0, //!< - data pipe 0.
		ENAA_PA = 0x3f //!< Enable all auto ack on all data pipes.
	} __attribute__((packed));

	/**
	 * Register EN_RXADDR bitfields, enable receive pipe.
	 */
	enum
	{
		ERX_P5 = 5, //!< Enable data pipe 5.
		ERX_P4 = 4, //!< - data pipe 4.
		ERX_P3 = 3, //!< - data pipe 3.
		ERX_P2 = 2, //!< - data pipe 2.
		ERX_P1 = 1, //!< - data pipe 1.
		ERX_P0 = 0, //!< - data pipe 0.
		ERX_PA = 0x3f //!< Enable all data pipes.
	} __attribute__((packed));

	/**
	 * Register SETUP_AW bitfields, setup address width (3..5).
	 */
	enum
	{
		AW = 0, //!< RX/TX address field width (bits 2).
		AW_3BYTES = 1, //!< 3 bytes.
		AW_4BYTES = 2, //!< 4 bytes.
		AW_5BYTES = 3 //!< 5 bytes.
	} __attribute__((packed));

	/**
	 * Register SETUP_RETR bitfields, configure retransmission.
	 */
	enum
	{
		ARD = 4, //!< Auto retransmit delay (bits 4).
		//!< - delay * 250 us (250..4000 us).
		DEFAULT_ARD = 1, //!< Default auto retransmit delay (500 us)
		ARC = 0, //!< Auto retransmit count (bits 4).
		//!< - retransmit count (0..15).
		DEFAULT_ARC = 15 //!< Default auto retransmit count (15)
	} __attribute__((packed));

	/**
	 * Register RF_SETUP bitfields, radio configuration.
	 */
	enum
	{
		CONT_WAVE = 7, //!< Continuous carrier transmit.
		RF_DR_LOW = 5, //!< Set RF data rate to 250 kbps.
		PLL_LOCK_SIGNAL = 4, //!< Force PLL lock signal.
		RF_DR_HIGH = 3, //!< Air data bitrate (2 Mbps).
		RF_PWR = 1 //!< Set RF output power in TX mode (bits 2).
	} __attribute__((packed));

	/**
	 * Transmission rates RF_DR_LOW/RF_DR_HIGH values, radio bit-rate.
	 */
	enum
	{
		RF_DR_1MBPS = 0, //!< 1 Mbps.
		RF_DR_2MBPS = _BV(RF_DR_HIGH), //!< 2 Mbps.
		RF_DR_250KBPS = _BV(RF_DR_LOW) //!< 250 Kbps.
	} __attribute__((packed));

	/**
	 * Output power RF_PWR values, radio power setting.
	 */
	enum
	{
		RF_PWR_18DBM = 0, //!< -18dBm.
		RF_PWR_12DBM = 2, //!< -12dBm.
		RF_PWR_6DBM = 4, //!< -6dBm.
		RF_PWR_0DBM = 6 //!<  0dBm.
	} __attribute__((packed));

	/**
	 * Register STATUS bitfields.
	 */
	enum
	{
		RX_DR = 6, //!< Data ready RX FIFO interrupt.
		TX_DS = 5, //!< Data send TX FIFO interrupt.
		MAX_RT = 4, //!< Maximum number of TX retransmits interrupt.
		RX_P_NO = 1, //!< Data pipe number for available payload (3b).
		RX_P_NO_MASK = 0x0e, //!< Mask pipe number.
		RX_P_NO_NONE = 0x07, //!< No pipe.
		TX_FIFO_FULL = 0 //!< TX FIFO full flag.
	} __attribute__((packed));

	/**
	 * Register OBSERVE_TX bitfields, performance statistics.
	 */
	enum
	{
		PLOS_CNT = 4, //!< Count lost packets (bits 4).
		ARC_CNT = 0 //!< Count retransmitted packets (bits 4).
	} __attribute__((packed));

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
		observe_tx_t(uint8_t value) {
			as_byte = value;
		}
	};

	/**
	 * Register FIFO_STATUS bitfields, transmission queue status.
	 */
	enum
	{
		TX_REUSE = 6, //!< Reuse last transmitted data packat.
		TX_FULL = 5, //!< TX FIFO full flag.
		TX_EMPTY = 4, //!< TX FIFO empty flag.
		RX_FULL = 1, //!< RX FIFO full flag.
		RX_EMPTY = 0, //!< RX FIFO empty flag.
	} __attribute__((packed));

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
		fifo_status_t(uint8_t value) {
			as_byte = value;
		}
	};

	/**
	 * Register DYNPD bitfields.
	 */
	enum
	{
		DPL_P5 = 5, //!< Enable dynamic payload length data pipe 5.
		DPL_P4 = 4, //!< - data pipe 4.
		DPL_P3 = 3, //!< - data pipe 3.
		DPL_P2 = 2, //!< - data pipe 2.
		DPL_P1 = 1, //!< - data pipe 1.
		DPL_P0 = 0, //!< - data pipe 0.
		DPL_PA = 0x3f //!< Enable dynamic payload length on all pipes.
	} __attribute__((packed));

	/**
	 * Register FEATURE bitfields.
	 */
	enum
	{
		EN_DPL = 2, //!< Enable dynamic payload length.
		EN_ACK_PAY = 1, //!< Enable payload with ACK.
		EN_DYN_ACK = 0 //!< Enable the W_TX_PAYLOAD_NOACK command.
	} __attribute__((packed));

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
		status_t(uint8_t value)
		{
			as_byte = value;
		}
	};

	/**
	 * Timing information (ch. 6.1.7, tab. 16, pp. 24).
	 */
	static const uint16_t Tpd2stby_ms = 3;
	static const uint16_t Tstby2a_us = 130;
	static const uint16_t Thce_us = 10;

	/**
	 * Configuration max values.
	 */
	enum {
		AW_MAX = 5, //!< Max address width in bytes.
		PIPE_MAX = 6, //!< Max number of pipes.
	} __attribute__((packed));
};

#endif /* NRF24L01_INTERNALS_HH */
