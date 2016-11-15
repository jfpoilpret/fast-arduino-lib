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

namespace NRF24L01Internals
{
	/**
	 * Register CONFIG bitfields, configuration.
	 */
	const uint8_t	MASK_RX_DR = 6;		//!< Mask interrupt caused by RX_DR.
	const uint8_t	MASK_TX_DS = 5;		//!< Mask interrupt caused by TX_DS.
	const uint8_t	MASK_MAX_RT = 4;	//!< Mask interrupt caused byt MAX_RT.
	const uint8_t	EN_CRC = 3;			//!< Enable CRC.
	const uint8_t	CRCO = 2;			//!< CRC encoding scheme (2/1 bytes CRC).
	const uint8_t	PWR_UP = 1;			//!< Power up/down.
	const uint8_t	PRIM_RX = 0;		//!< RX/TX control (PRX/PTX).

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
