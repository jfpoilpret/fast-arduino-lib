//   Copyright 2016-2023 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

/**
 * @file
 * API to handle AdaFruit BLE SPI Friend breakout through SPI interface.
 * Connection diagram:
 * 
 * TODO
 * 
 * @sa https://learn.adafruit.com/introducing-the-adafruit-bluefruit-spi-breakout/introduction
 */
#ifndef BLUEFRUIT_SPI_HH
#define BLUEFRUIT_SPI_HH

#include <string.h>

#include "../flash.h"
#include "../gpio.h"
#include "../spi.h"
#include "../time.h"
#include "../utilities.h"

namespace devices
{
	/**
	 * SPI device driver for AdaFruit BLE Friend SPI breakout.
	 * @tparam CS the output pin used for Chip Selection of the device on
	 * the SPI bus.
	 * @tparam TODO other pins (IRQ, RESET)
	 * TODO check default clock divider! "The SPI clock should run <=2MHz"
	 * TODO "A 100us delay should be added between the moment that the CS line is asserted, and before any data is transmitted on the SPI bus"
	 * TODO define buffer for request/response no?
	 */
	template<board::DigitalPin CS, board::DigitalPin IRQ, board::DigitalPin RESET>
//	class BlueFruitSpi : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_HIGH, spi::ClockRate::CLOCK_DIV_16>
	class BlueFruitSpi : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_LOW, spi::ClockRate::CLOCK_DIV_16>
//	class BlueFruitSpi : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(2000000UL)>
	{
	public:
		/**
		 * Create a new device driver for an AdaFruit BLE Friend.
		 */
		BlueFruitSpi() = default;

		void begin()
		{
			reset_.clear();
			time::delay_ms(TIME_RESET_MS);
			reset_.set();
		}

		enum class CommandStatus: uint8_t
		{
			NONE,
			OK,
			ERROR,
			TIMEOUT
		};

		// API low-level AT command with result (string only), command/data mode
		//TODO Better return as a class instance holding status, payload or whatever is relevant?
		CommandStatus at_command(const char* command, bool wait_for_reply = true)
		{
			//TODO refactor (later)
			// Breakdown command into SDEP packets
			const uint8_t size = strlen(command);
			uint8_t done = 0;
			while (true)
			{
				SDEPPacket packet;
				done = breakdown_payload(((const uint8_t*) command), size, done, packet);
				write_packet(packet);
				if (done == size)
					break;
			}

			if (!wait_for_reply)
				return CommandStatus::NONE;

			// Wait for reply (TODO add utility function)
			if (!wait_for_irq(TIME_MAX_WAIT_FOR_IRQ_US))
				return CommandStatus::TIMEOUT;

			// Read reply packets
			MessageType type = read_reply();
			// Check message type and resurn status accordingly
			switch (type)
			{
				case MessageType::RESPONSE:
				//NOTE that OK may mean ERROR :-o
				return CommandStatus::OK;

				//TODO can that really happen?
				case MessageType::COMMAND:
				case MessageType::ALERT:
				case MessageType::ERROR:
				return CommandStatus::ERROR;

				case MessageType::NOT_READY:
				case MessageType::READ_OVERFLOW:
				return CommandStatus::TIMEOUT;
			}
		}

		const uint8_t* latest_reply() const
		{
			return rx_buffer;
		}

//		CommandStatus at_command(const flash::FlashStorage* command, bool wait_for_reply = true);

		// API high-level: reset, factory, NVM
		// API high-level: GATT setup: create service, attribute, events, callbacks...
		//TODO for GATT, maybe use friend classes (one class for service, one for characteristic)

	private:
		//TODO delay but how long (doc mentions 100us, but Digital Analyzer shows it is less!)
		static constexpr uint16_t TIME_RESET_MS = 10;
		static constexpr uint16_t TIME_AFTER_CS_ACTIVE_US = 10;
//		static constexpr uint16_t TIME_AFTER_CS_ACTIVE_US = 100;
		static constexpr uint16_t TIME_MAX_WAIT_FOR_IRQ_US = 10000;
		static constexpr uint16_t TIME_WAIT_FOR_IRQ_RETRY_US = 10;
		static constexpr uint16_t TIME_WAIT_FOR_READY_RETRY_US = 10;
		//TODO make it a template argument instead (or constructor template argument?)
		static constexpr uint8_t RX_BUFFER_SIZE = 128;
		// Buffer for all received payload bytes in response to a command
		uint8_t rx_buffer[RX_BUFFER_SIZE];
		uint8_t rx_next_byte = 0;

		// Implement SDEP protocol (https://github.com/adafruit/Adafruit_BluefruitLE_nRF51/blob/master/SDEP.md)
		// SDEP message types
		enum class MessageType: uint8_t
		{
			COMMAND = 0x10,
			RESPONSE = 0x20,
			ALERT = 0x40,
			ERROR = 0x80,
			NOT_READY = 0xFE,
			READ_OVERFLOW = 0xFF
		};

		// Bytes already ivnerted to avoid covnersion to little-endian
		enum class CommandId: uint16_t
		{
			INITIALIZE = 0xEFBE,
			AT_WRAPPER = 0x000A,
			BLE_UARTTX = 0x010A,
			BLE_UARTRX = 0x020A
		};

		// SDEP Packet
		static constexpr uint8_t SDEP_PAYLOAD_MAX_LENGTH = 16;
		struct SDEPPacket
		{
			SDEPPacket(
				MessageType type = MessageType::COMMAND,
				CommandId command_id = CommandId::AT_WRAPPER,
				bool more_data = false,
				uint8_t length = 0)
				: type{type}, command_id{command_id}, more_data{uint8_t(more_data ? 1 : 0)}, length{length} {}

			MessageType	type;
			CommandId command_id;
			uint8_t more_data :1;
			uint8_t reserved :2;
			uint8_t length :5;
			uint8_t payload[SDEP_PAYLOAD_MAX_LENGTH];
		};
		static constexpr uint8_t SDEP_HEADER_LENGTH = sizeof(SDEPPacket) - SDEP_PAYLOAD_MAX_LENGTH;
		
		// SDEP functions
		void write_packet(const SDEPPacket& packet)
		{
			this->start_transfer();
			time::delay_us(TIME_AFTER_CS_ACTIVE_US);
			const uint8_t* byte_packet = ((const uint8_t*) &packet);
			uint8_t size = SDEP_HEADER_LENGTH + packet.length;
			while (size--)
			{
				this->transfer(*byte_packet++);
			}
			this->end_transfer();
		}

		bool read_packet_type(MessageType& type)
		{
			type = MessageType(this->transfer(0xFF));
			switch (type)
			{
				case MessageType::NOT_READY:
				case MessageType::READ_OVERFLOW:
				return false;

				default:
				return true;
			}
		}

		MessageType wait_packet_type()
		{
			MessageType type;
			while (!read_packet_type(type))
			{
				//TODO do not wait indefinately, include some timeout!!!
				this->end_transfer();
				time::delay_us(TIME_WAIT_FOR_READY_RETRY_US);
				this->start_transfer();
			}
			return type;
		}

		void read_packet(SDEPPacket& packet)
		{
			this->start_transfer();
			time::delay_us(TIME_AFTER_CS_ACTIVE_US);
			// First read first byte (loop until READY)
			MessageType type = wait_packet_type();
			//TODO Check MessageType and act accordingly!!!
			packet.type = type;
			// Read header 3 remaining bytes (command id, length)
			uint8_t* byte_packet = ((uint8_t*) &packet);
			const uint8_t size1 = SDEP_HEADER_LENGTH - 1;
			this->transfer(&byte_packet[1], size1, 0xFF);
			// Then read payload
			const uint8_t size2 = packet.length;
			this->transfer(&byte_packet[size1], size2, 0xFF);
			this->end_transfer();
		}

		// Return total transmitted size (used for 'done' on next call)
		static uint8_t breakdown_payload(const uint8_t* payload, uint8_t size, uint8_t done, SDEPPacket& packet)
		{
			uint8_t remain = size - done;
			uint8_t last_packet = 1;
			if (remain > SDEP_PAYLOAD_MAX_LENGTH)
			{
				remain = SDEP_PAYLOAD_MAX_LENGTH;
				last_packet = 0;
			}
			packet.more_data = last_packet;
			packet.length = remain;
			memcpy(packet.payload, &payload[done], remain);

			return done + remain;
		}
		//TODO reverse breakdown (received packets)

		bool wait_for_irq(uint16_t timeout_us)
		{
			//TODO use timeout value
			while (!irq_.value())
				time::delay_us(TIME_WAIT_FOR_IRQ_RETRY_US);
			return true;
		}

		MessageType read_reply()
		{
			MessageType type;
			uint8_t* rx = rx_buffer;
			bool finished = false;
			// Clear RX buffer
			memset(rx_buffer, 0, RX_BUFFER_SIZE);
			// Loop until no more packets to read
			//TODO what if RX buffer overflow!!!!
			while (!finished)
			{
				SDEPPacket packet;
				read_packet(packet);
				type = packet.type;
				switch (type)
				{
					case MessageType::RESPONSE:
					case MessageType::ERROR:
					case MessageType::ALERT:
					//TODO what if ERROR or ALERT?
					// Fill RX buffer with received payload
					memcpy(rx, packet.payload, packet.length);
					rx += packet.length;
					if (!packet.more_data)
						finished = true;
					break;

					case MessageType::COMMAND:
					case MessageType::NOT_READY:
					case MessageType::READ_OVERFLOW:
					// Should not happen normally? TODO double check!!!
					finished = true;
					break;
				}
			}
			return type;
		}

		gpio::FAST_PIN<IRQ> irq_ = gpio::FAST_PIN<IRQ>{gpio::PinMode::INPUT};
		gpio::FAST_PIN<RESET> reset_ = gpio::FAST_PIN<RESET>{gpio::PinMode::OUTPUT, true};
	};
}

#endif /* BLUEFRUIT_SPI_HH */
/// @endcond
