// Important copyright notice:
// Large parts of this file were copied from Mikael Patel's Cosa library, which copyright appears below
// Content has been adapted to use FastArduino original parts, under Copyright (c) 2016, Jean-Francois Poilpret

/*
 * Copyright (C) 2013-2015, Mikael Patel
 * Copyright (C) 2016-2018, Jean-Francois Poilpret
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

#ifndef NRF24L01_INTERNALS_HH
#define NRF24L01_INTERNALS_HH

#include <stddef.h>

namespace devices::rf::nrf24l01p_internals
{
	/**
	 * Register CONFIG bitfields, configuration.
	 */
	const uint8_t MASK_RX_DR = 6;  //!< Mask interrupt caused by RX_DR.
	const uint8_t MASK_TX_DS = 5;  //!< Mask interrupt caused by TX_DS.
	const uint8_t MASK_MAX_RT = 4; //!< Mask interrupt caused byt MAX_RT.
	const uint8_t EN_CRC = 3;	  //!< Enable CRC.
	const uint8_t CRCO = 2;		   //!< CRC encoding scheme (2/1 bytes CRC).
	const uint8_t PWR_UP = 1;	  //!< Power up/down.
	const uint8_t PRIM_RX = 0;	 //!< RX/TX control (PRX/PTX).

	/**
	 * Register EN_AA bitfields, auto acknowledgement.
	 */
	const uint8_t ENAA_P5 = 5;	//!< Enable auto acknowledgement data pipe 5.
	const uint8_t ENAA_P4 = 4;	//!< - data pipe 4.
	const uint8_t ENAA_P3 = 3;	//!< - data pipe 3.
	const uint8_t ENAA_P2 = 2;	//!< - data pipe 2.
	const uint8_t ENAA_P1 = 1;	//!< - data pipe 1.
	const uint8_t ENAA_P0 = 0;	//!< - data pipe 0.
	const uint8_t ENAA_PA = 0x3f; //!< Enable all auto ack on all data pipes.

	/**
	 * Register EN_RXADDR bitfields, enable receive pipe.
	 */
	const uint8_t ERX_P5 = 5;	//!< Enable data pipe 5.
	const uint8_t ERX_P4 = 4;	//!< - data pipe 4.
	const uint8_t ERX_P3 = 3;	//!< - data pipe 3.
	const uint8_t ERX_P2 = 2;	//!< - data pipe 2.
	const uint8_t ERX_P1 = 1;	//!< - data pipe 1.
	const uint8_t ERX_P0 = 0;	//!< - data pipe 0.
	const uint8_t ERX_PA = 0x3f; //!< Enable all data pipes.

	/**
	 * Register SETUP_AW bitfields, setup address width (3..5).
	 */
	const uint8_t AW = 0;		 //!< RX/TX address field width (bits 2).
	const uint8_t AW_3BYTES = 1; //!< 3 bytes.
	const uint8_t AW_4BYTES = 2; //!< 4 bytes.
	const uint8_t AW_5BYTES = 3; //!< 5 bytes.

	/**
	 * Register SETUP_RETR bitfields, configure retransmission.
	 */
	const uint8_t ARD = 4; //!< Auto retransmit delay (4 bits).
							//!< - delay * 250 us (250..4000 us).
	const uint8_t ARC = 0; //!< Auto retransmit count (4 bits).
							//!< - retransmit count (0..15).

	const uint8_t DEFAULT_ARC = 15; //!< Default auto retransmit count (15)
	const uint8_t DEFAULT_ARD = 1;  //!< Default auto retransmit delay (500 us)

	/**
	 * Register RF_SETUP bitfields, radio configuration.
	 */
	const uint8_t CONT_WAVE = 7;	   //!< Continuous carrier transmit.
	const uint8_t RF_DR_LOW = 5;	   //!< Set RF data rate to 250 kbps.
	const uint8_t PLL_LOCK_SIGNAL = 4; //!< Force PLL lock signal.
	const uint8_t RF_DR_HIGH = 3;	  //!< Air data bitrate (2 Mbps).
	const uint8_t RF_PWR = 1;		   //!< Set RF output power in TX mode (bits 2).

	/**
	 * Transmission rates RF_DR_LOW/RF_DR_HIGH values, radio bit-rate.
	 */
	const uint8_t RF_DR_1MBPS = 0;				  //!< 1 Mbps.
	const uint8_t RF_DR_2MBPS = _BV(RF_DR_HIGH);  //!< 2 Mbps.
	const uint8_t RF_DR_250KBPS = _BV(RF_DR_LOW); //!< 250 Kbps.

	/**
	 * Output power RF_PWR values, radio power setting.
	 */
	const uint8_t RF_PWR_18DBM = 0; //!< -18dBm.
	const uint8_t RF_PWR_12DBM = 2; //!< -12dBm.
	const uint8_t RF_PWR_6DBM = 4;  //!< -6dBm.
	const uint8_t RF_PWR_0DBM = 6;  //!<  0dBm.

	/**
	 * Register STATUS bitfields.
	 */
	const uint8_t RX_DR = 6;		   //!< Data ready RX FIFO interrupt.
	const uint8_t TX_DS = 5;		   //!< Data send TX FIFO interrupt.
	const uint8_t MAX_RT = 4;		   //!< Maximum number of TX retransmits interrupt.
	const uint8_t RX_P_NO = 1;		   //!< Data pipe number for available payload (3b).
	const uint8_t RX_P_NO_MASK = 0x0e; //!< Mask pipe number.
	const uint8_t RX_P_NO_NONE = 0x07; //!< No pipe.
	const uint8_t TX_FIFO_FULL = 0;	//!< TX FIFO full flag.

	/**
	 * Register OBSERVE_TX bitfields, performance statistics.
	 */
	const uint8_t PLOS_CNT = 4; //!< Count lost packets (4 bits).
	const uint8_t ARC_CNT = 0;  //!< Count retransmitted packets (4 bits).

	/**
	 * Register FIFO_STATUS bitfields, transmission queue status.
	 */
	const uint8_t TX_REUSE = 6; //!< Reuse last transmitted data packat.
	const uint8_t TX_FULL = 5;  //!< TX FIFO full flag.
	const uint8_t TX_EMPTY = 4; //!< TX FIFO empty flag.
	const uint8_t RX_FULL = 1;  //!< RX FIFO full flag.
	const uint8_t RX_EMPTY = 0; //!< RX FIFO empty flag.

	/**
	 * Register DYNPD bitfields.
	 */
	const uint8_t DPL_P5 = 5;	//!< Enable dynamic payload length data pipe 5.
	const uint8_t DPL_P4 = 4;	//!< - data pipe 4.
	const uint8_t DPL_P3 = 3;	//!< - data pipe 3.
	const uint8_t DPL_P2 = 2;	//!< - data pipe 2.
	const uint8_t DPL_P1 = 1;	//!< - data pipe 1.
	const uint8_t DPL_P0 = 0;	//!< - data pipe 0.
	const uint8_t DPL_PA = 0x3f; //!< Enable dynamic payload length on all pipes.

	/**
	 * Register FEATURE bitfields.
	 */
	const uint8_t EN_DPL = 2;	 //!< Enable dynamic payload length.
	const uint8_t EN_ACK_PAY = 1; //!< Enable payload with ACK.
	const uint8_t EN_DYN_ACK = 0; //!< Enable the W_TX_PAYLOAD_NOACK command.

	/**
	 * Timing information (ch. 6.1.7, tab. 16, pp. 24).
	 */
	const uint16_t Tpd2stby_ms = 3;
	const uint16_t Tstby2a_us = 130;
	const uint16_t Thce_us = 10;

	/**
	 * Configuration max values.
	 */
	const uint8_t AW_MAX = 5;   //!< Max address width in bytes.
	const uint8_t PIPE_MAX = 6; //!< Max number of pipes.
}
#endif /* NRF24L01_INTERNALS_HH */
