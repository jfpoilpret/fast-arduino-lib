/**
 * Copyright (C) 2016-2020, Jean-Francois Poilpret
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
 */

/*
 * NRF24L01+ ping/pong example.
 * This program shows usage of FastArduino support for SPI and NRF24L01+ device. 
 * It also uses FastArduino RTT and time support.
 * The program should be uploaded to 2 boards (these can be 2 different boards).
 * One board will act as "master" (initiates all exchanges), the other one as a 
 * "slave", will wait for master requests and will send reply after each received
 * request.
 * Master/Slave selection is performed by grounding PIN_CONFIG (if slave) or keep
 * it floating (if master).
 * For boards having a hardware USART, traces of all exchanges (and errors) are sent to it.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1 (TX) used for tracing program activities
 *   - D7 master/slave configuration pin
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D8 (CSN): SPI interface to NRF24L01+
 *   - D9 (CE): interface to NRF24L01+
 * - on Arduino LEONARDO:
 *   - D1 (TX) used for tracing program activities
 *   - D8 master/slave configuration pin
 *   - Board-ICSP (SCK, MISO, MOSI), D9 (CSN): SPI interface to NRF24L01+
 *   - D10 (CE): interface to NRF24L01+
 * - on Arduino MEGA:
 *   - D1 (TX) used for tracing program activities
 *   - D7 master/slave configuration pin
 *   - D52 (SCK), D50 (MISO), D51 (MOSI), D8 (CSN): SPI interface to NRF24L01+
 *   - D9 (CE): interface to NRF24L01+
 * - on ATtinyX4 based boards:
 *   - D7 master/slave configuration pin
 *   - D4 (SCK), D6 (MISO), D5 (MOSI), D2 (CSN): SPI interface to NRF24L01+
 *   - D3 (CE): interface to NRF24L01+
 * - on ATtinyX5 based boards (slave only):
 *   - D2 (SCK), D0 (MISO), D1 (MOSI), D3 (CSN): SPI interface to NRF24L01+
 *   - D4 (CE): interface to NRF24L01+
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output used for tracing program activities
 *   - D10 (PB2): master/slave configuration pin
 *   - D8 (PB0, CSN), D13 (PB5, MOSI), D14 (PB6, MISO), D15 (PB7, SCK): SPI interface to NRF24L01+
 *   - D9 (PB1, CE): interface to NRF24L01+
 * 
 * Note: this example does not use NRF24L01+ IRQ pin to wake up the active 
 * waiting loop during reception.
 */

#include <fastarduino/gpio.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/devices/nrf24l01p.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HAS_TRACE 1
#define USART_NUM 0
static const constexpr board::USART UART = board::USART::USART0;
static const constexpr board::DigitalPin PIN_CONFIG = board::DigitalPin::D7_PD7;
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D8_PB0;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D9_PB1;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER2;
// Define vectors we need in the example
REGISTER_RTT_ISR(2)
#elif defined(ARDUINO_LEONARDO)
#define HAS_TRACE 1
#define USART_NUM 1
static const constexpr board::USART UART = board::USART::USART1;
static const constexpr board::DigitalPin PIN_CONFIG = board::DigitalPin::D8_PB4;
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D9_PB5;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D10_PB6;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER1;
// Define vectors we need in the example
REGISTER_RTT_ISR(1)
#elif defined(ARDUINO_MEGA)
#define HAS_TRACE 1
#define USART_NUM 0
static const constexpr board::USART UART = board::USART::USART0;
static const constexpr board::DigitalPin PIN_CONFIG = board::DigitalPin::D7_PH4;
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D8_PH5;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D9_PH6;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER2;
// Define vectors we need in the example
REGISTER_RTT_ISR(2)
#elif defined (BREADBOARD_ATTINYX4)
#define HAS_TRACE 0
static const constexpr board::DigitalPin PIN_CONFIG = board::DigitalPin::D7_PA7;
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D2_PA2;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D3_PA3;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER0;
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
#elif defined (BREADBOARD_ATTINYX5)
#define HAS_TRACE 0
#define IS_SLAVE 1
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D3_PB3;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D4_PB4;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER0;
// Define vectors we need in the example
REGISTER_RTT_ISR(0)
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HAS_TRACE 1
#define USART_NUM 0
static const constexpr board::USART UART = board::USART::USART0;
static const constexpr board::DigitalPin PIN_CONFIG = board::DigitalPin::D10_PB2;
static const constexpr board::DigitalPin PIN_CSN = board::DigitalPin::D8_PB0;
static const constexpr board::DigitalPin PIN_CE = board::DigitalPin::D9_PB1;
static const constexpr board::Timer RTT_TIMER = board::Timer::TIMER2;
// Define vectors we need in the example
REGISTER_RTT_ISR(2)
#else
#error "Current target is not yet supported!"
#endif

#if HAS_TRACE
#include <fastarduino/uart.h>
// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
REGISTER_UATX_ISR(USART_NUM)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
#include <fastarduino/empty_streams.h>
#endif

static const uint16_t NETWORK = 0xFFFF;
static const uint8_t MASTER = 0x01;
static const uint8_t SLAVE = 0x02;

static const uint32_t REPLY_MAX_WAIT_MS = 1000L;
static const uint32_t RECEIVE_MAX_WAIT_MS = 10000L;
static const uint32_t DELAY_BETWEEN_2_FRAMES_MS = 100L;

#ifdef IS_SLAVE
static bool is_master()
{
	return false;
}
#else
static bool is_master()
{
	gpio::FAST_PIN<PIN_CONFIG> config{gpio::PinMode::INPUT_PULLUP};
	return config.value();
}
#endif

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

#if HAS_TRACE
	// Setup traces
	serial::hard::UATX<UART> uatx{output_buffer};
	uatx.begin(115200);
	auto trace = uatx.out();
	trace.width(0);
#else
	streams::null_ostream trace;
#endif
	
	bool master = is_master();
	uint8_t self_device = master ? MASTER : SLAVE;
	uint8_t other_device = master ? SLAVE : MASTER;
	trace << "RF24App1 started as " << (master ? "Master" : "Slave") << streams::endl;

	// Setup RTT
	timer::RTT<RTT_TIMER> rtt;
	rtt.begin();
	// Set RTT instance as default clock from now
	time::set_clock(rtt);
	trace << "RTT started" << streams::endl;

	// Start SPI and setup NRF24
	spi::init();
	devices::rf::NRF24L01<PIN_CSN, PIN_CE> rf{NETWORK, self_device};
	rf.begin();
	trace << "NRF24L01+ started" << streams::endl;
	
	// Event Loop
	uint8_t sent_port = 0;
	uint32_t count = 0;
	while (true)
	{
		// Reset RTT milliseconds counter to avoid overflow
		rtt.millis(0);
		if (master)
		{
			// Try to send to slave
			trace << "S " << sent_port << streams::flush;
			int result = rf.send(other_device, sent_port);
			if (result < 0)
				trace	<< "\nError " << result << "! #Trans=" << rf.get_trans() 
						<< " #Retrans=" << rf.get_retrans() 
						<< " #Drops=" << rf.get_drops() << streams::endl;
			
			// Then wait for slave reply
			trace << " R " << streams::flush;
			uint8_t src, port;
			result = rf.recv(src, port, REPLY_MAX_WAIT_MS);
			if (result < 0)
				trace << "\nError " << result << '!' << streams::endl;
			else
				trace << uint16_t(port) << " (" << uint16_t(src) << ") " << streams::flush;
			
			// Wait 1 second before doing it again
			++sent_port;
			time::delay(DELAY_BETWEEN_2_FRAMES_MS);
		}
		else
		{
			// Wait for master payload
			trace << "R " << streams::flush;
			uint8_t src, port;
			int result = rf.recv(src, port);
			if (result < 0)
				trace << "\nError " << result << '!' << streams::endl;
			else
			{
				trace << uint16_t(port) << " (" << uint16_t(src) << ") RR " << streams::flush;
				// Reply to master with same content
				result = rf.send(src, port);
				if (result < 0)
					trace	<< "\nError " << result << "! #Trans=" << rf.get_trans() 
							<< " #Retrans=" << rf.get_retrans() 
							<< " #Drops=" << rf.get_drops() << streams::endl;
			}
		}
		if (++count % 1000 == 0)
			trace << "\n count = " << count << "\n#Trans=" << rf.get_trans() 
					<< " #Retrans=" << rf.get_retrans() 
					<< " #Drops=" << rf.get_drops() << streams::endl;
	}
}
