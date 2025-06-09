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

/*
 * This is a skeleton program to help debug AdaFruit BlueFruuit SPI Friend device,
 * before developing a real FastArduino SPI device for it.
 * To ease wiring and debugging, I suggest using a real Arduino UNO board
 * and a small breadboard for connecting the SPI device.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (SCK): connected to BlueFruit SPI device SCK pin
 *   - D12 (MISO): connected to BlueFruit SPI device MISO pin (sometimes called Dout)
 *   - D11 (MOSI): connected to BlueFruit SPI device MOSI pin (sometimes called Din)
 *   - D10 (SS): connected to BlueFruit SPI device CS pin
 *   - D2 (INT0): connected to BlueFruit SPI device IRQ PIN
 *   - D6: connected to BlueFruit SPI device RESET pin
 *   - direct USB access (traces output to console)
 */

#include <fastarduino/bits.h>
#include <fastarduino/gpio.h>
#include <fastarduino/initializer_list.h>
#include <fastarduino/int.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/spi.h>
#include <fastarduino/uart.h>
#include <fastarduino/utilities.h>

// Define vectors we need in the example
constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// SPI Device specific stuff goes here
//=====================================

static constexpr const uint32_t SPI_CLOCK = 4'000'000UL;
static constexpr const spi::ChipSelect CHIP_SELECT = spi::ChipSelect::ACTIVE_LOW;
static constexpr const spi::DataOrder DATA_ORDER = spi::DataOrder::MSB_FIRST;
static constexpr const spi::Mode MODE = spi::Mode::MODE_0;
static constexpr const spi::ClockRate CLOCK_RATE = spi::compute_clockrate(SPI_CLOCK);

// For testing we use default SS pin as CS
static constexpr const board::DigitalPin CS = board::DigitalPin::D10_PB2;
#define IRQ_PIN 0
static constexpr const board::ExternalInterruptPin IRQ = board::ExternalInterruptPin::D2_PD2_EXT0;
static constexpr const board::DigitalPin RESET = board::DigitalPin::D6_PD6;

// Timer 16 bit for ticks every 60us or so
#define NTIMER 1
static constexpr const board::Timer BLE_TIMER = board::Timer::TIMER1;

//TODO POC NEXT
// 4h	- rework example to use current Android app!
//FIXME	- some transactions fail sometimes (e.g. advertise()) => DEBUG!

// 4h	- rework as real FastArduino device (same API as POC to start with)
//			- implement basic example
//			- implement example with LED blinking controlled by Android application
// 1h	- do we need to call the special init command? in lieu of hard reset? when? timing?
// 16h	- then improve device to:
//			- use Futures for all API
//			- also define sync API based on async API
//			- callback design with CRTP pattern
//			- buffer size management? (normally at Future level no?)
// 4h+	- implement UART API (to be inferred thoroughly first)
//			- support data/command mode?
// 1h	- add ostream for AT commands (how to trigger sending?)
// 1h	- add istream for AT responses (how to trigger end of reception?)

using UUID16 = uint16_t;
using UUID128 = uint8_t[16];

//TODO gather all these utilities methods in a specific class and/or namespace
// Conversion utilities
static char* copy(const char* str, char* buffer, bool terminate = false)
{
	while (*str)
		*buffer++ = *str++;
	if (terminate)
		*buffer = 0;
	return buffer;
}
static char* copy(const flash::FlashStorage* str, char* buffer, bool terminate = false)
{
	uint16_t address = (uint16_t) str;
	while (char value = pgm_read_byte(address++))
		*buffer++ = value;
	if (terminate)
		*buffer = 0;
	return buffer;
}
static char* convert_int(uint16_t value, char* buffer, bool terminate = false)
{
	utoa(value, buffer, 10);
	buffer += strlen(buffer);
	if (terminate)
		*buffer = 0;
	return buffer;
}
static char* convert_hex(uint8_t byte, char* buffer, bool prefix = false, bool terminate = false)
{
	if (prefix)
		buffer = copy("0x", buffer);
	char buf[3];
	utoa(byte, buf, 16);
	if (strlen(buf) == 1)
		*buffer++ = '0';
	return copy(buf, buffer, terminate);
}
static char* convert_hex(uint16_t word, char* buffer, bool terminate = false)
{
	buffer = copy("0x", buffer);
	// Change endianness
	word = utils::change_endianness(word);
	char buf[5];
	utoa(word, buf, 16);
	uint8_t padding = 4 - strlen(buf);
	while (padding--)
		*buffer++ = '0';
	return copy(buf, buffer, terminate);
}
static char* convert_hex(uint32_t dword, char* buffer, bool terminate = false)
{
	buffer = copy("0x", buffer);
	// Change endianness
	dword = utils::change_endianness(dword);
	char buf[9];
	ultoa(dword, buf, 16);
	uint8_t padding = 8 - strlen(buf);
	while (padding--)
		*buffer++ = '0';
	return copy(buf, buffer, terminate);
}

static char* convert_hex(const uint8_t byte_array[], uint8_t size, char* buffer, bool terminate = false)
{
	for (uint8_t i = 0; i < size; ++i)
	{
		if (i != 0)
			*buffer++ = '-';
		buffer = convert_hex(*byte_array++, buffer, false);
	}
	if (terminate)
		*buffer = 0;
	return buffer;
}

struct GATTProperty
{
	static constexpr uint8_t Broadcast = bits::BV8(0);
	static constexpr uint8_t Read = bits::BV8(1);
	static constexpr uint8_t WriteWithoutResponse = bits::BV8(2);
	static constexpr uint8_t Write = bits::BV8(3);
	static constexpr uint8_t Notify = bits::BV8(4);
	static constexpr uint8_t Indicate = bits::BV8(5);
};

//TODO clarify range for integer (2 or 4 bytes)
enum class GattDataType : uint8_t
{
	STRING = 1,
	BYTEARRAY = 2,
	INTEGER = 3
};

// Subclass SPIDevice to make protected methods available from main()
class PublicDevice: public spi::SPIDevice<CS, CHIP_SELECT, CLOCK_RATE, MODE, DATA_ORDER>
{
private:
	using THIS = PublicDevice;
	static constexpr const uint8_t BUFFER_SIZE = 128;

public:
	PublicDevice(): SPIDevice()
	{
		// Register callback handlers
		interrupt::register_handler(*this);
		// Start timer but suspend interrupts
		synchronized
		{
			timer_.begin_(COUNTER);
			timer_.suspend_interrupts_();
		}
	}

	//TODO Add static_assert on RESET != NONE?
	void hard_reset(bool force_wait = true)
	{
		reset_.clear();
		time::delay_ms(DELAY_RESET_CLEAR_MS);
		reset_.set();
		if (force_wait)
			time::delay_ms(DELAY_AFTER_RESET_MS);
	}

	// Forward declaration
	class Response;

	bool reset(bool force_wait = true)
	{
		char cmd[32];
		copy(F("ATZ"), cmd, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		if (response.is_successful())
		{
			if (force_wait)
				time::delay_ms(DELAY_AFTER_RESET_MS);
			return true;
		}
		return false;
	}

	bool factory_reset(bool force_wait = true)
	{
		char cmd[32];
		copy(F("AT+FACTORYRESET"), cmd, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		if (response.is_successful())
		{
			if (force_wait)
				time::delay_ms(DELAY_AFTER_RESET_MS);
			return true;
		}
		return false;
	}

	// Launch AT command synchronously
	bool await_at_command(const char* command, Response& response, uint16_t loop_until_timout = 
		loop_count_for_await_response(TIMEOUT_AWAIT_RESPONSE_US))
	{
		if (!at_command(command))
		{
			response = Response();
			return false;
		}
		return await_response(response, loop_until_timout);
	}

	// Launch AT command asynchronously
	bool at_command(const char* command)
	{
		// Ensure transfer operation can be started (no other operation is on going at the same time)
		synchronized
		{
			if (operation_ != OperationStatus::NO_OP)
				return false;
			operation_ = OperationStatus::SENDING_PACKET;
		}
		// Setup operation extra data
		error_ = Error::OK;
		wait_loop_count_ = loop_count_for_cs(TIMEOUT_DEVICE_READY_US);
		// Setup data to be transmitted
		size_ = strlen(command);
		memcpy(buffer_, command, size_);
		current_ = buffer_;

		// Try to send 1st packet immediately
		send_packet();
		// Start timer to send next packets
		timer_.resume_interrupts();
		return true;
	}

	bool is_response_ready() const
	{
		return operation_ == OperationStatus::FINISHED;
	}

	// Return false if there is nothing to await for (typically because method called twice for 1 exchange)
	// Return false if timeout elapsed before end of transaction
	// Return true if response is available and usable (but that may include error situations!)
	bool await_response(Response& response, uint16_t loop_until_timout = 
		loop_count_for_await_response(TIMEOUT_AWAIT_RESPONSE_US))
	{
		if (operation_ == OperationStatus::NO_OP)
		{
			response = Response();
			return false;
		}
		while (operation_ != OperationStatus::FINISHED)
		{
			time::delay_us(DELAY_AWAIT_RESPONSE_US);
			if (--loop_until_timout == 0)
			{
				response = Response();
				return false;
			}
		}
		// Check if OK in response
		buffer_[size_] = 0;
		const Error error = (error_ == Error::OK ? check_ok((const char*) buffer_) : error_);
		response = Response{error, message_id_, size_, buffer_};
		// Now we can authorize new transactions again
		operation_ = OperationStatus::NO_OP;
		return true;
	}

	// synchronously clear all GATT configuration
	bool clear_GATT()
	{
		char cmd[32];
		copy(F("AT+GATTCLEAR"), cmd, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		return response.is_successful();
	}

	// synchronously add a new service
	// Return 0 in case of failure
	// Return >0 index to created service
	uint8_t add_GATT_service(UUID128 uuid)
	{
		// Prepare AT command
		char cmd[128];
		char* buf = copy(F("AT+GATTADDSERVICE=UUID128="), cmd);
		buf = convert_hex(uuid, sizeof(UUID128), buf, true);
		//Execute AT command and await response
		Response response;
		if (!await_at_command(cmd, response))
			return 0;
		if (!response.is_successful())
			return 0;
		// Successfully created service, parse response to get index
		return atoi(response.response());
	}

	// synchronously add a new characteristic
	// Return 0 in case of failure
	// Return >0 index to created characteristic (for later use in get/set)
	uint8_t add_GATT_characteristic(UUID16 uuid, uint8_t properties, GattDataType type, uint8_t min_len, uint8_t max_len)
	{
		//TODO Check a service was added before!
		// Prepare AT command
		char cmd[128];
		char *buf = copy(F("AT+GATTADDCHAR=UUID="), cmd);
		buf = convert_hex(uuid, buf);
		buf = copy(F(",PROPERTIES="), buf);
		buf = convert_hex(properties, buf, true);
		buf = copy(F(",MIN_LEN="), buf);
		buf = convert_int(min_len, buf);
		buf = copy(F(",MAX_LEN="), buf);
		buf = convert_int(max_len, buf);
		//TODO VALUE?
		buf = copy(F(",DATATYPE="), buf);
		buf = convert_int(uint8_t(type), buf, true);
		//TODO DESCRIPTION?

		//Execute AT command and await response
		Response response;
		if (!await_at_command(cmd, response))
			return 0;
		if (!response.is_successful())
			return 0;
		// Successfully created service, parse response to get index
		return atoi(response.response());
	}

	// synchronously add a new characteristic
	// Return 0 in case of failure
	// Return >0 index to created characteristic (for later use in get/set)
	// Refactor both add_GATT_characteristic() functions
	uint8_t add_GATT_characteristic(UUID128 uuid, uint8_t properties, GattDataType type, uint8_t min_len, uint8_t max_len)
	{
		//TODO Check a service was added before!
		// Prepare AT command
		char cmd[128];
		char *buf = copy(F("AT+GATTADDCHAR=UUID128="), cmd);
		buf = convert_hex(uuid, sizeof(UUID128), buf);
		buf = copy(F(",PROPERTIES="), buf);
		buf = convert_hex(properties, buf, true);
		buf = copy(F(",MIN_LEN="), buf);
		buf = convert_int(min_len, buf);
		buf = copy(F(",MAX_LEN="), buf);
		buf = convert_int(max_len, buf);
		//TODO VALUE?
		buf = copy(F(",DATATYPE="), buf);
		buf = convert_int(uint8_t(type), buf, true);

		//Execute AT command and await response
		Response response;
		if (!await_at_command(cmd, response))
			return 0;
		if (!response.is_successful())
			return 0;
		// Successfully created service, parse response to get index
		return atoi(response.response());
	}

	// synchronously set GATT characteristic
	bool set_GATT_characteristic(uint8_t index, const char* value)
	{
		char cmd[128];
		char* buf = copy(F("AT+GATTCHAR="), cmd);
		buf = convert_int(index, buf);
		*buf++ = ',';
		buf = copy(value, buf, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		return response.is_successful();
	}

	// synchronously get GATT characteristic
	bool get_GATT_characteristic(uint8_t index, char* value, uint8_t size)
	{
		//TODO use AT+GATTCHARRAW instead (more efficient?)
		char cmd[128];
		char* buf = copy(F("AT+GATTCHAR="), cmd);
		buf = convert_int(index, buf, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		if (!response.is_successful())
			return false;
		// Get value and return as string (converted by caller)
		// Only keep first line
		const char* result = response.response();
		const uint8_t len = strchr(result, '\r') - result + 1;
		strncpy(value, response.response(), utils::min(size, len) - 1);
		// Ensure string is properly 0-terminated (not ensured by strncpy())
		value[size - 1] = '\0';
		return true;
	}

	// synchronously set BLE GAP Advertise data
	bool advertise(const uint8_t* data, uint8_t size)
	{
		char cmd[128];
		char* buf = copy(F("AT+GAPSETADVDATA="), cmd);
		buf = convert_hex(data, size, buf, true);
		Response response;
		if (!await_at_command(cmd, response))
			return false;
		return response.is_successful();
	}

	// Functions for events handling
	//-------------------------------
	// snchronously register events
	bool register_events(bool connect, bool disconnect, std::initializer_list<uint8_t> characteristics)
	{
		const uint8_t global_events = (connect ? 0x01 : 0) | (disconnect ? 0x02 : 0);
		uint32_t gatt_events = 0;
		for (uint8_t index: characteristics)
			gatt_events |= bits::BV32(index - 1);
		char cmd[128];
		char* buf = copy(F("AT+EVENTENABLE="), cmd);
		buf = convert_hex(global_events, buf, true);
		*buf++ = ',';
		buf = convert_hex(gatt_events, buf, true);

		Response response;
		if (!await_at_command(cmd, response))
			return false;
		return response.is_successful();
	}

	class Events
	{
	public:
		Events() = default;
		Events(const Events&) = default;
		Events& operator=(const Events&) = default;

		bool has_events() const
		{
			return (global_events_ != 0) || (gatt_events_ != 0);
		}
		bool is_connected() const
		{
			return global_events_ & 0x01;
		}
		bool is_disconnected() const
		{
			return global_events_ & 0x02;
		}
		bool is_GATT_characteristic_event(uint8_t index)
		{
			return gatt_events_ & bits::BV32(index);
		}
	private:
		Events(uint8_t global_events, uint32_t gatt_events)
			: global_events_{global_events}, gatt_events_{gatt_events} {}

		uint8_t global_events_ = 0;
		uint32_t gatt_events_ = 0;

		friend THIS;
	};

	bool check_events(Events& events)
	{
		Response response;
		if (!await_at_command("AT+EVENTSTATUS", response))
			return false;
		if (!response.is_successful())
			return false;
		// First line contains 0xXX,0xYY (2 hex numbers that are bitfields, 
		// XX is for system events and YY for GATT events)
		const char* buf = response.response();
		char* buf2;
		uint8_t global_events = strtoul(buf, &buf2, 16);
		uint32_t gatt_events = strtoul(++buf2, 0, 16);
		events = Events(global_events, gatt_events);
		return true;
	}

	//TODO If we make these functions public, we shall have timeout argument in other API!
	static constexpr uint16_t loop_count_for_cs(uint16_t timeout_us)
	{
		return timeout_us / DELAY_TIMER_US;
	}
	static constexpr uint16_t loop_count_for_irq(uint16_t timeout_us)
	{
		return timeout_us / DELAY_TIMER_US;
	}
	static constexpr uint16_t loop_count_for_await_response(uint16_t timeout_us)
	{
		return timeout_us / DELAY_AWAIT_RESPONSE_US;
	}

	enum class Error : uint8_t
	{
		OK = 0,
		BUSY,
		TIMEOUT_DEVICE_NOT_READY,
		TIMEOUT_IRQ,
		TIMEOUT_DEVICE_NO_DATA,
		RESPONSE_WITH_ERROR,
		RESPONSE_SIZE_OVERFLOW,
		RESPONSE_TYPE_ALERT,
		RESPONSE_TYPE_ERROR,
		RESPONSE_TYPE_UNEXPECTED,
		EXCHANGE_PHASE_OUT
	};

	//TODO put outside class? But need to templatize then (buffer size)!
	class Response
	{
	public:
		Response() = default;
		Response(const Response&) = default;
		Response& operator=(const Response&) = default;

		bool is_usable() const
		{
			return usable_;
		}

		bool is_successful() const
		{
			return usable_ && (error_ == Error::OK);
		}

		uint8_t response_size() const
		{
			return size_;
		}

		const char* response() const
		{
			return (is_successful() ? (const char*) response_ : 0);
		}

		Error error() const
		{
			return (usable_ ? error_ : Error::BUSY);
		}

		bool is_error() const
		{
			return usable_ && (error_ != Error::RESPONSE_TYPE_ERROR);
		}

		bool is_alert() const
		{
			return usable_ && (error_ == Error::RESPONSE_TYPE_ALERT);
		}

		uint16_t alert_id() const
		{
			return (is_alert() ? id_ : 0);
		}

		uint16_t error_id() const
		{
			return (is_error() ? id_ : 0);
		}

	private:
		Response(Error error, uint16_t id, uint8_t size, const uint8_t* response)
			:	usable_{true}, error_{error}, id_{id}, size_{size}
		{
			memcpy(response_, response, size);
			response_[size] = 0;
		}

		bool usable_ = false;
		Error error_;
		uint16_t id_;
		uint8_t size_;
		uint8_t response_[THIS::BUFFER_SIZE + 1];

		friend THIS;
	};

private:
	// Events bits for AT+EVENTENABLE and other event-related AT commands
	static constexpr const uint8_t EVENT_CONNECT = 0;
	static constexpr const uint8_t EVENT_DISCONNECT = 1;
	static constexpr const uint8_t EVENT_UART_RX = 8;
	  
	// Time settings
	//---------------
	// Hardware reset
	static constexpr const uint16_t DELAY_RESET_CLEAR_MS = 10;
	static constexpr const uint16_t DELAY_AFTER_RESET_MS = 1000;

	// Retry times (in microseconds) in loops (for 1st byte OK and for IRQ wait)
	static constexpr const uint16_t DELAY_TIMER_US = 60;

	// Retry times (in microseconds) in await loop
	static constexpr const uint16_t DELAY_AWAIT_RESPONSE_US = 500;
	static constexpr const uint16_t TIMEOUT_AWAIT_RESPONSE_US = 40000;

	// Timeouts (in microseconds) for aborting waits on 1st byte OK or received IRQ 
	static constexpr const uint16_t TIMEOUT_DEVICE_READY_US = 1000;
	static constexpr const uint16_t TIMEOUT_IRQ_WAIT_US = 30000;

	// Possible values for type byte
	// SDEP MessageType (1st header byte)
	static constexpr const uint8_t TYPE_COMMAND = 0x10;
	static constexpr const uint8_t TYPE_RESPONSE = 0x20;
	static constexpr const uint8_t TYPE_ALERT = 0x30;
	static constexpr const uint8_t TYPE_ERROR = 0x40;
	static constexpr const uint8_t DEVICE_NOT_READY = 0xFE;
	static constexpr const uint8_t DEVICE_NO_DATA = 0xFF;
	// SDEP Command IDs
	static constexpr const uint16_t COMMAND_INIT = 0xBEEF;
	static constexpr const uint16_t COMMAND_AT = 0x0A00;
	static constexpr const uint16_t COMMAND_UART_TX = 0x0A01;
	static constexpr const uint16_t COMMAND_UART_RX = 0x0A02;
	// SDEP Alert IDs TODO replace with public enum class!
	static constexpr const uint16_t ALERT_RESERVED = 0x0000;
	static constexpr const uint16_t ALERT_SYSTEM_RESET = 0x0001;
	static constexpr const uint16_t ALERT_BATTERY_LOW = 0x0002;
	static constexpr const uint16_t ALERT_BATTERY_CRITICAL = 0x0003;
	// SDEP Error IDs TODO replace with public enum class!
	static constexpr const uint16_t ERROR_RESERVED = 0x0000;
	static constexpr const uint16_t ERROR_INVALID_CMD_ID = 0x0001;
	static constexpr const uint16_t ERROR_INVALID_PAYLOAD = 0x0003;
	// SDEP payload size masks
	static constexpr const uint8_t MASK_MORE_DATA = 0x80;
	static constexpr const uint8_t MASK_PAYLOAD_SIZE = 0x1F;

	// Status of an exchange operation
	enum class OperationStatus : uint8_t
	{
		NO_OP = 0,
		SENDING_PACKET,
		WAITING_IRQ,
		RECEIVING_PACKET,
		FINISHED
	};

	static constexpr const uint8_t SDEP_PAYLOAD_SIZE = 16;

	// Enable CS, transfer 1st byte and check received byte is OK
	// Return true if 1st bytes received is OK, then timer is suspended
	// Return false if 1st byte received is not OK, then CS is disabled
	// Return false also if iteration count is reached (then timer is suspended,
	// and error_ is updated)
	bool send_type(uint8_t& type, bool accept_no_data)
	{
		start_transfer();
		type = transfer(type);
		bool ok = true;
		Error error;
		switch (type)
		{
			case DEVICE_NOT_READY:
			ok = false;
			error = Error::TIMEOUT_DEVICE_NOT_READY;
			break;

			case DEVICE_NO_DATA:
			ok = accept_no_data;
			error = Error::TIMEOUT_DEVICE_NO_DATA;
			break;

			default:
			break;
		}
		if (!ok)
		{
			// Disable CS and let timer trigger another attempt in a few dozen us
			end_transfer();
			// Check if timeout reached
			if (--wait_loop_count_ == 0)
			{
				error_ = error;
				timer_.suspend_interrupts_();
			}
		}
		return ok;
	}

	// Return true if packet successfully transmitted
	// Return false if 1st byte received is not OK, then CS is disabled
	// Return false also if iteration count is reached (then timer is suspended,
	// and error_ is updated)
	bool send_packet()
	{
		// Start SPI transfer, send message type and ensure we get a non error return byte (0xFE)
		uint8_t type = TYPE_COMMAND;
		if (!send_type(type, true))
			return false;
	
		// Command ID
		transfer(utils::low_byte(COMMAND_AT));
		transfer(utils::high_byte(COMMAND_AT));

		// Payload size
		const uint8_t size = (size_ > SDEP_PAYLOAD_SIZE ? SDEP_PAYLOAD_SIZE : size_);
		const uint8_t more_data = (size_ > SDEP_PAYLOAD_SIZE ? MASK_MORE_DATA : 0);
		transfer(size | more_data);

		// Payload
		transfer(current_, size);

		// End SPI transfer
		end_transfer();

		// Was it the last packet to send?
		if (more_data)
		{
			current_ += size;
			size_ -= size;
			wait_loop_count_ = loop_count_for_cs(TIMEOUT_DEVICE_READY_US);
		}
		else
		{
			// Update operation status
			operation_ = OperationStatus::WAITING_IRQ;
			current_ = buffer_;
			wait_loop_count_ = loop_count_for_irq(TIMEOUT_IRQ_WAIT_US);
			size_ = 0;
			signal_.enable_();
		}

		return true;
	}

	// Return true if packet successfully received, even though a buffer overflow
	// may have occurred (visible in error_ field)
	// Return false if 1st byte received is not OK, then CS is disabled
	// Return false also if iteration count is reached (then timer is suspended,
	// and error_ is updated)
	bool get_packet()
	{
		// Start SPI transfer, send 0xFF and ensure we get a non error return byte (0xFE or 0xFF)
		uint8_t type = 0xFF;
		if (!send_type(type, false))
			return false;
	
		// Command ID
		const uint8_t low = transfer(0xFF);
		const uint8_t high = transfer(0xFF);
		message_id_ = utils::as_uint16_t(high, low);

		// Payload size
		const uint8_t size_byte = transfer(0xFF);
		const uint8_t size = size_byte & MASK_PAYLOAD_SIZE;
		const bool more_data = size_byte & MASK_MORE_DATA;

		// Check total size is not over buffer_ size!
		if (size_ + size <= BUFFER_SIZE)
		{
			// Payload
			transfer(current_, size, 0xFF);
			size_ += size;
		}
		else
		{
			// Ensure packet is fully transferred even though we will not fill buffer
			transfer(size, 0xFF);
			// Report overflow error
			error_ = Error::RESPONSE_SIZE_OVERFLOW;
		}

		// End SPI transfer
		end_transfer();

		// Check response type is expected
		switch (type)
		{
			case TYPE_RESPONSE:
			// passthrough
			break;

			case TYPE_ALERT:
			error_ = Error::RESPONSE_TYPE_ALERT;
			break;

			case TYPE_ERROR:
			error_ = Error::RESPONSE_TYPE_ERROR;
			break;

			default:
			error_ = Error::RESPONSE_TYPE_UNEXPECTED;
			break;
		}

		// Was it the last packet to receive?
		if (error_ == Error::OK && more_data)
		{
			current_ += size;
			wait_loop_count_ = loop_count_for_cs(TIMEOUT_DEVICE_READY_US);
		}
		else
		{
			// Update operation status
			operation_ = OperationStatus::FINISHED;
			timer_.suspend_interrupts_();
		}

		return true;
	}

	// Return Error::OK if response contains "OK\r\n"
	// Return Error::RESPONSE_WITH_ERROR otherwise
	Error check_ok(const char* response)
	{
		char* found = strstr(response, "OK\r\n");
		return (found != 0 ? Error::OK : Error::RESPONSE_WITH_ERROR);
	}

	// ISR callbacks
	//---------------
	void on_irq_high()
	{
		// Double check IRQ level (you never know)
		if (irq_.value())
		{
			operation_ = OperationStatus::RECEIVING_PACKET;
			wait_loop_count_ = loop_count_for_cs(TIMEOUT_DEVICE_READY_US);
			signal_.disable_();
			// Try to get 1st packet immediately
			get_packet();
			if (operation_ != OperationStatus::FINISHED)
				timer_.resume_interrupts_();
		}
	}

	void on_timer_compare()
	{
		// First check current operation?
		switch (operation_)
		{
			case OperationStatus::SENDING_PACKET:
			send_packet();
			break;

			case OperationStatus::RECEIVING_PACKET:
			get_packet();
			break;

			case OperationStatus::WAITING_IRQ:
			// Check count loop IRQ
			if (--wait_loop_count_ == 0)
			{
				error_ = Error::TIMEOUT_IRQ;
				operation_ = OperationStatus::FINISHED;
				timer_.suspend_interrupts_();
			}
			break;

			default:
			// Unexpected operation status: phasing error?
			error_ = Error::EXCHANGE_PHASE_OUT;
			operation_ = OperationStatus::FINISHED;
			timer_.suspend_interrupts_();
			break;
		}
	}

	// Data members
	//--------------
	// Data for on going exchange operation
	// Current operation phase
	volatile OperationStatus operation_ = OperationStatus::NO_OP;
	// Latest error occurred during operation
	volatile Error error_ = Error::OK;
	// Alert or Error ID if received
	volatile uint16_t message_id_ = 0;
	// Count of wait loops for acceptable 1st received byte
	uint16_t wait_loop_count_;
	// Buffer used both for sending command and received response
	uint8_t buffer_[BUFFER_SIZE];
	// For command sending: buffer size remaining to transfer
	// For response receiving: response size received sofar
	uint8_t size_;
	// Pointer to next buffer_ byte to read/write
	uint8_t* current_;

	using TIMER = timer::Timer<BLE_TIMER>;
	using CALC = TIMER::CALCULATOR;
	static constexpr const TIMER::PRESCALER PRESCALER = CALC::CTC_prescaler(DELAY_TIMER_US);
	static_assert(CALC::is_adequate_for_CTC(PRESCALER, DELAY_TIMER_US));
	static constexpr const uint16_t COUNTER = CALC::CTC_counter(PRESCALER, DELAY_TIMER_US);
	TIMER timer_ = TIMER{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};

	gpio::FAST_EXT_PIN<IRQ> irq_ = gpio::FAST_EXT_PIN<IRQ>{gpio::PinMode::INPUT};
	interrupt::INTSignal<IRQ> signal_ = interrupt::INTSignal<IRQ>{interrupt::InterruptTrigger::RISING_EDGE};
	gpio::FAST_PIN<RESET> reset_ = gpio::FAST_PIN<RESET>{gpio::PinMode::OUTPUT, true};

	DECL_TIMER_COMP_FRIENDS;
	DECL_INT_ISR_FRIENDS;
	friend class Response;
	friend class Events;

	friend int main();
};

REGISTER_INT_ISR_METHOD(IRQ_PIN, IRQ, PublicDevice, &PublicDevice::on_irq_high)
REGISTER_TIMER_COMPARE_ISR_METHOD(NTIMER, PublicDevice, &PublicDevice::on_timer_compare)

using streams::endl;
using streams::flush;
using streams::dec;
using streams::hex;
using Error = PublicDevice::Error;

streams::ostream& operator<<(streams::ostream& o, Error error)
{
	const flash::FlashStorage* label = 0;
	switch (error)
	{
		case Error::OK:
		label = F("OK");
		break;
		
		case Error::BUSY:
		label = F("BUSY");
		break;
		
		case Error::TIMEOUT_DEVICE_NOT_READY:
		label = F("TIMEOUT_DEVICE_NOT_READY");
		break;
		
		case Error::TIMEOUT_IRQ:
		label = F("TIMEOUT_IRQ");
		break;
		
		case Error::TIMEOUT_DEVICE_NO_DATA:
		label = F("TIMEOUT_DEVICE_NO_DATA");
		break;
		
		case Error::RESPONSE_SIZE_OVERFLOW:
		label = F("RESPONSE_SIZE_OVERFLOW");
		break;
		
		case Error::RESPONSE_WITH_ERROR:
		label = F("RESPONSE_WITH_ERROR");
		break;

		case Error::RESPONSE_TYPE_ALERT:
		label = F("RESPONSE_TYPE_ALERT");
		break;

		case Error::RESPONSE_TYPE_ERROR:
		label = F("RESPONSE_TYPE_ERROR");
		break;

		case Error::RESPONSE_TYPE_UNEXPECTED:
		label = F("RESPONSE_TYPE_UNEXPECTED");
		break;

		case Error::EXCHANGE_PHASE_OUT:
		label = F("EXCHANGE_PHASE_OUT");
		break;
	}
	return o << label << flush;
}

// Service to control blink activity: 44013301-C3DA-4962-B6E5-AF262E392263
static UUID128 BLINK_SERVICE_UUID = {
	0x44, 0x01,   // service number: blink service
	0x33, 0x01,   // 0x01 reserved for service
	0xC3, 0xDA, 0x49, 0x62, 0xB6, 0xE5, 0xAF, 0x26, 0x2E, 0x39, 0x22, 0x63
};
  
// Characteristic to control blink activity: R/W one byte (00/FF inactive/active)
static UUID16 BLINK_STATUS_CHAR_UUID = 0x0233;

// Service to setup blink timing: 44023301-C3DA-4962-B6E5-AF262E392263
static UUID128 CONFIG_SERVICE_UUID = {
	0x44, 0x02,   // service number: blink timing setup
	0x33, 0x01,   // 0x01 reserved for service
	0xC3, 0xDA, 0x49, 0x62, 0xB6, 0xE5, 0xAF, 0x26, 0x2E, 0x39, 0x22, 0x63
};

// Characteristic to configure blinking timings: RW 4 bytes (2 UInt: low time ms, high time ms)
// UUID 44023302-C3DA-4962-B6E5-AF262E392263
static UUID16 BLINK_TIMINGS_CHAR_UUID = 0x0233;

// Characteristic to load/dump timings to/from NVRAM (W 1byte: 0x0F load, 0xF0 dump)
// UUID 44023303-C3DA-4962-B6E5-AF262E392263
static UUID16 NVRAM_TIMINGS_CHAR_UUID = 0x0333;

// Service to monitor blink LED status: 44033301-C3DA-4962-B6E5-AF262E392263
static UUID128 MONITOR_SERVICE_UUID = {
	0x44, 0x03,   // service number: blink LED status
	0x33, 0x01,   // 0x01 reserved for service
	0xC3, 0xDA, 0x49, 0x62, 0xB6, 0xE5, 0xAF, 0x26, 0x2E, 0x39, 0x22, 0x63
};
  
// Characteristic to monitor status: N 1 byte (00/FF off/on)
// UUID: 44033302-C3DA-4962-B6E5-AF262E392263
static UUID16 MONITOR_CHAR_UUID = 0x0233;
	
// Advertising record
// NOTE: there seem to be many limitations in this record:
// - size is limited (could not advertise more than one 128 bits UUID service!)
// - only one call allowed (next calls fully override previous records!)
static uint8_t ADV[] = {
	0x02, 0x01, 0x06,   // Flags: do not change
	0x02, 0x0A, 0x00,   // TX Power level (0dBm): do not change
	// Incomplete list of 128 bits UUID services: UART service (to be changed)
	// Service to control blink activity: 44013301-C3DA-4962-B6E5-AF262E392263 (must be reversed...)
	0x11, 0x06, 0x63, 0x22, 0x39, 0x2E, 0x26, 0xAF, 0xE5, 0xB6, 0x62, 0x49, 0xDA, 0xC3, 0x01, 0x33, 0x01, 0x44,
	// 0x11, 0x06, 0x63, 0x22, 0x39, 0x2E, 0x26, 0xAF, 0xE5, 0xB6, 0x62, 0x49, 0xDA, 0xC3, 0x01, 0x33, 0x02, 0x44,
	// 0x11, 0x06, 0x63, 0x22, 0x39, 0x2E, 0x26, 0xAF, 0xE5, 0xB6, 0x62, 0x49, 0xDA, 0xC3, 0x01, 0x33, 0x03, 0x44,
};

// Timing values for blinking
struct BlinkTimings
{
  uint16_t high_time_ms = 500;
  uint16_t low_time_ms = 500;
};

// Blinking status at runtime (internal)
struct BlinkStatus
{
  uint32_t latest_change = 0;
  bool current_level = false;
};

// Global variables (I know, it is ugly, but hey, this is only a POC here!)
static bool blinkActive = false;
static BlinkTimings blinkTimings = BlinkTimings();
static BlinkStatus blinkStatus = BlinkStatus();

static constexpr const board::DigitalPin LED = board::DigitalPin::D2_PD2;

static void on_write_status(const char* value)
{
	//TODO
}
static void on_write_config(const char* value)
{
	//TODO
}
static void on_write_nvram(const char* value)
{
	//TODO
}
  
int main()
{
	board::init();
	sei();

	// Init UART output for traces
	serial::hard::UATX<UART> uart{output_buffer};
	uart.begin(115200);
	streams::ostream out = uart.out();
	out.width(2);

	// Prepare LED pin
	gpio::FAST_PIN<LED> led{gpio::PinMode::OUTPUT, false};

	// Start SPI interface
	//TODO Improve init() to reduce CS high/low
	spi::init();
	PublicDevice device;
	device.hard_reset();
	out << F("SPI & device initialized") << endl;

	bool ok = device.factory_reset();
	if (!ok)
		out << F("ERROR during factory_reset()") << endl;

	// Add services and characteristics
	uint8_t index = device.add_GATT_service(BLINK_SERVICE_UUID);
	if (index == 0)
		out << F("ERROR during add_GATT_service(BLINK_SERVICE_UUID)") << endl;
	uint8_t idCharBlinkStatus = device.add_GATT_characteristic(
		BLINK_STATUS_CHAR_UUID, 
		GATTProperty::WriteWithoutResponse | GATTProperty::Read,
		GattDataType::BYTEARRAY, 1, 1);
	if (idCharBlinkStatus == 0)
		out << F("ERROR during add_GATT_characteristic(BLINK_STATUS_CHAR_UUID)") << endl;
	out << F("idCharBlinkStatus=") << idCharBlinkStatus << endl;

	index = device.add_GATT_service(CONFIG_SERVICE_UUID);
	if (index == 0)
		out << F("ERROR during add_GATT_service(CONFIG_SERVICE_UUID)") << endl;
	uint8_t idCharBlinkTiming = device.add_GATT_characteristic(
		BLINK_TIMINGS_CHAR_UUID, 
		GATTProperty::WriteWithoutResponse | GATTProperty::Read,
		GattDataType::BYTEARRAY, 4, 4);
	if (idCharBlinkTiming == 0)
		out << F("ERROR during add_GATT_characteristic(BLINK_TIMINGS_CHAR_UUID)") << endl;
	out << F("idCharBlinkTiming=") << idCharBlinkTiming << endl;
	uint8_t idCharBlinkNVRAM = device.add_GATT_characteristic(
		NVRAM_TIMINGS_CHAR_UUID, 
		GATTProperty::WriteWithoutResponse,
		GattDataType::BYTEARRAY, 1, 1);
	if (idCharBlinkNVRAM == 0)
		out << F("ERROR during add_GATT_characteristic(NVRAM_TIMINGS_CHAR_UUID)") << endl;
	out << F("idCharBlinkNVRAM=") << idCharBlinkNVRAM << endl;

	index = device.add_GATT_service(MONITOR_SERVICE_UUID);
	if (index == 0)
		out << F("ERROR during add_GATT_service(MONITOR_SERVICE_UUID)") << endl;
	uint8_t idCharBlinkLED = device.add_GATT_characteristic(
		MONITOR_CHAR_UUID, 
		GATTProperty::Notify,
		GattDataType::BYTEARRAY, 1, 1);
	if (idCharBlinkLED == 0)
		out << F("ERROR during add_GATT_characteristic(MONITOR_CHAR_UUID)") << endl;
	out << F("idCharBlinkLED=") << idCharBlinkLED << endl;

	// Change device name
	PublicDevice::Response response;
	ok = device.await_at_command("AT+GAPDEVNAME=Vader Sr One", response);
	if (!ok)
		out << F("ERROR during await_at_command('AT+GAPDEVNAME=...')") << endl;
	if (!response.is_successful())
		out << F("Reponse ERROR during await_at_command('AT+GAPDEVNAME=...')") << endl;

	// Ensure new UUID services get advertised!
	ok = device.advertise(ADV, sizeof(ADV));
	if (!ok)
		out << F("ERROR during advertise()") << endl;

	// Reset device to ensure accounting for latest config changes
	ok = device.reset();
	if (!ok)
		out << F("ERROR during reset()") << endl;
	
	// Enable system events
	//TODO Replace with register_events() and fix endianness if needed!
	// ok = device.await_at_command("AT+EVENTENABLE=0x3,0x3", response);
	ok = device.await_at_command("AT+EVENTENABLE=0x3,0xFFFF", response);
	// ok = device.register_events(true, true, {idCharBlinkStatus});
	if (!ok)
		out << F("ERROR during register_events()") << endl;
	if (!response.is_successful())
	{
		out << F("ERROR response during register_events():") << endl;
		out << response.error() << endl;
		out << response.response() << endl;
	}

	// Loop to check connect/disconnect events status
	while (true)
	{
		time::delay_ms(1000);
		out << '.' << flush;
		PublicDevice::Events events;
		ok = device.check_events(events);
		if (!ok)
		{
			out << endl;
			out << F("ERROR during check_events()") << endl;
		}
		else
		{
			if (events.has_events())
				out << endl;
			// Display events
			if (events.is_connected())
				out << F("CONNECTED!") << endl;
			if (events.is_disconnected())
				out << F("DISCONNECTED!") << endl;
			if (events.is_GATT_characteristic_event(idCharBlinkStatus))
			{
				out << F("GATT idCharBlinkStatus write event!") << endl;
				char value[3];
				if (device.get_GATT_characteristic(idCharBlinkStatus, value, 3))
					on_write_status(value);
				else
					out << F("ERROR during get_GATT_characteristic(idCharBlinkStatus)") << endl;
			}
			if (events.is_GATT_characteristic_event(idCharBlinkTiming))
			{
				out << F("GATT idCharBlinkTiming write event!") << endl;
				char value[9];
				if (device.get_GATT_characteristic(idCharBlinkTiming, value, 9))
					on_write_config(value);
				else
					out << F("ERROR during get_GATT_characteristic(idCharBlinkTiming)") << endl;
			}
			if (events.is_GATT_characteristic_event(idCharBlinkNVRAM))
			{
				out << F("GATT idCharBlinkNVRAM write event!") << endl;
				char value[3];
				if (device.get_GATT_characteristic(idCharBlinkNVRAM, value, 3))
					on_write_nvram(value);
				else
					out << F("ERROR during get_GATT_characteristic(idCharBlinkNVRAM)") << endl;
			}
		}
	}
	// Stop SPI device if needed
	// out << F("End") << endl;
}
