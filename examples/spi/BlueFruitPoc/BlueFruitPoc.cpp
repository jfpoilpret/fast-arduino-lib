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

#include <fastarduino/gpio.h>
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

//TODO POC
// 2h+	- add ISR for IRQ and Timer (CTC for 1st byte transmit retry)
// 15'	- better error handling (specific error() method)
// 2h+	- implement GATT API (how?) with callbacks: use Builder pattern?
// 2h+	- implement UART API (to be inferred thoroughly first)
//			- support data/command mode?
// 1h	- implement "basic" commands (e.g. factory reset)
// 1h	- add ostream for AT commands (how to trigger sending?)
// 1h	- add istream for AT responses (how to trigger end of reception?)

// Subclass SPIDevice to make protected methods available from main()
class PublicDevice: public spi::SPIDevice<CS, CHIP_SELECT, CLOCK_RATE, MODE, DATA_ORDER>
{
public:
	enum class Error : uint8_t
	{
		OK = 0,
		BUSY,
		TIMEOUT_DEVICE_NOT_READY,
		TIMEOUT_IRQ,
		TIMEOUT_DEVICE_NO_DATA,
		RESPONSE_WITH_ERROR,
		RESPONSE_SIZE_OVERFLOW,
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
	static constexpr const uint16_t DELAY_BEFORE_CS_US = 60;
	static constexpr const uint16_t DELAY_AFTER_CS_US = 0;
	static constexpr const uint16_t DELAY_LOOP_IRQ_US = 10;

	// Timeouts (in microseconds) for aborting waits on 1st byte OK or received IRQ 
	static constexpr const uint16_t TIMEOUT_DEVICE_READY_US = 1000;
	static constexpr const uint16_t TIMEOUT_IRQ_WAIT_US = 10000;

	//TODO Add flag when command sent and need to get a reply before any new command...

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
	// SDEP payload size masks
	static constexpr const uint8_t MASK_MORE_DATA = 0x80;
	static constexpr const uint8_t MASK_PAYLOAD_SIZE = 0x1F;

	// Status of an exchange operation
	enum class OperationStatus : uint8_t
	{
		NO_OP = 0,
		SENDING_TYPE,
		SENDING_ALL,
		WAITING_IRQ,
		RECEIVING_TYPE,
		RECEIVING_ALL
	};

	struct SDEPHeader
	{
		SDEPHeader() = default;
		SDEPHeader(uint8_t type, uint16_t command, uint8_t size, bool more_data)
			:	type{type}, 
				command{command}, 
				size{uint8_t((size & MASK_PAYLOAD_SIZE) | (more_data ? MASK_MORE_DATA : 0))} {}

		uint8_t type;
		uint16_t command;
		uint8_t size;
	};

	static constexpr const uint8_t SDEP_PAYLOAD_SIZE = 16;
	struct SDEPPacket
	{
		SDEPPacket() = default;
		SDEPPacket(uint8_t type, uint16_t command, uint8_t size, bool more_data, const uint8_t* payload)
			:	header{type, command, size, more_data}
		{
			memcpy(this->payload, payload, size & MASK_PAYLOAD_SIZE);
		}

		SDEPHeader header;
		uint8_t payload[SDEP_PAYLOAD_SIZE];
	};

	uint8_t wait_for_type(uint8_t sent_type, uint16_t loop_count, bool accept_no_data)
	{
		while (true)
		{
			start_transfer();
			if (DELAY_AFTER_CS_US != 0)
				time::delay_us(DELAY_AFTER_CS_US);
			uint8_t recv_type = transfer(sent_type);
			bool ok = true;
			switch (recv_type)
			{
				case DEVICE_NOT_READY:
				ok = false;
				break;

				case DEVICE_NO_DATA:
				ok = accept_no_data;
				break;

				default:
				break;
			}
			if (ok) return recv_type;
			end_transfer();
			if (--loop_count == 0) return recv_type;
			if (DELAY_BEFORE_CS_US != 0)
				time::delay_us(DELAY_BEFORE_CS_US);
		}
	}

	Error send_packet(const SDEPPacket& packet)
	{
		// Start SPI transfer, send message type and ensure we get a non error return byte (0xFE)
		uint8_t type = wait_for_type(packet.header.type, loop_count_for_cs(TIMEOUT_DEVICE_READY_US), true);
		if (type == DEVICE_NOT_READY)
			return Error::TIMEOUT_DEVICE_NOT_READY;
	
		// Command ID
		transfer(utils::low_byte(packet.header.command));
		transfer(utils::high_byte(packet.header.command));

		// Payload size
		transfer(packet.header.size);

		// Payload
		transfer(packet.payload, packet.header.size & MASK_PAYLOAD_SIZE);
	
		end_transfer();
		return Error::OK;
	}

	Error get_packet(SDEPPacket& packet)
	{
		// Start SPI transfer, send message type and ensure we get a non error return byte (0xFE)
		uint8_t type = wait_for_type(0xFF, loop_count_for_cs(TIMEOUT_DEVICE_READY_US), false);
		packet.header.type = type;
		switch (type)
		{
			case DEVICE_NOT_READY:
			return Error::TIMEOUT_DEVICE_NOT_READY;

			case DEVICE_NO_DATA:
			return Error::TIMEOUT_DEVICE_NO_DATA;
			
			default:
			break;
		}
	
		// Command ID
		uint8_t low = transfer(0xFF);
		uint8_t high = transfer(0xFF);
		packet.header.command = utils::as_uint16_t(high, low);

		// Payload size
		uint8_t size = transfer(0xFF);
		packet.header.size = size;

		// Payload
		transfer(packet.payload, size & MASK_PAYLOAD_SIZE, 0xFF);
	
		end_transfer();
		return Error::OK;
	}

public:
	static constexpr uint16_t loop_count_for_cs(uint16_t timeout_us)
	{
		return timeout_us / (DELAY_BEFORE_CS_US + DELAY_AFTER_CS_US);
	}
	static constexpr uint16_t loop_count_for_irq(uint16_t timeout_us)
	{
		return timeout_us / DELAY_LOOP_IRQ_US;
	}

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

	// Functions for events handling
	// void register_callbacks()
	// {
	// 	// Register events connect/disconnect
	// 	await_at_command("AT+EVENTENABLE=0x3");
	// }

	// bool check_events()
	// {
	// 	static constexpr const uint8_t SIZE = 64;
	// 	char response[SIZE+1];
	// 	bool ok = await_at_command("AT+EVENTSTATUS", response, SIZE);
	// 	if (!ok) return false;
	// 	//TODO Need to get response and check if event is present
	// 	// First line contains XX,YY (2 hex numbers that are bitfields, 
	// 	// XX is for system events and YY for GATT events)
	// }

	void reset()
	{
		reset_.clear();
		time::delay_ms(DELAY_RESET_CLEAR_MS);
		reset_.set();
		time::delay_ms(DELAY_AFTER_RESET_MS);
	}

	Error await_at_command(const char* command)
	{
		static constexpr const uint8_t SIZE = 64;
		char response[SIZE+1];
		return await_at_command(command, response, SIZE);
	}

	Error await_at_command(const char* command, char* response, uint8_t size)
	{
		Error error = send_command(command);
		if (error != Error::OK)
			return error;
		// reduce size to keep final string 0
		--size;
		error = get_response((uint8_t*) response, size);
		if (error != Error::OK) return error;
		response[size] = 0;
		return check_ok(response);
	}

	Error send_command(const char* command)
	{
		// Prepare count of packets
		const uint8_t len = strlen(command);
		const uint8_t last_packet_size = len % SDEP_PAYLOAD_SIZE;
		const uint8_t count_packets = (len / SDEP_PAYLOAD_SIZE) + (last_packet_size ? 1 : 0);
		const uint8_t* payload = (const uint8_t*) command;
		// Transmit each packet
		for (uint8_t num_packet = 0; num_packet < count_packets; ++num_packet)
		{
			const bool last = (num_packet == count_packets - 1);
			const uint8_t packet_size = (last && (last_packet_size != 0) ? last_packet_size : SDEP_PAYLOAD_SIZE);
			SDEPPacket packet{TYPE_COMMAND, COMMAND_AT, packet_size, !last, payload};
			Error error = send_packet(packet);
			if (error != Error::OK)
				return error;
			payload += packet_size;
		}
		return Error::OK;
	}

	Error wait_for_irq(uint16_t loop_count)
	{
		while (!irq_.value())
		{
			if (loop_count-- == 0)
				return Error::TIMEOUT_IRQ;
			time::delay_us(DELAY_LOOP_IRQ_US);
		}
		return Error::OK;
	}

	Error check_ok(const char* response)
	{
		char* found = strstr(response, "OK\r\n");
		return (found != 0 ? Error::OK : Error::RESPONSE_WITH_ERROR);
	}

	//TODO add expected command value?
	Error get_response(uint8_t* response, uint8_t& max_size)
	{
		Error error = wait_for_irq(loop_count_for_irq(TIMEOUT_IRQ_WAIT_US));
		if (error != Error::OK)
			return error;

		// Read all packet until last one
		uint8_t* current = response;
		uint8_t total = 0;
		while (true)
		{
			SDEPPacket packet;
			error = get_packet(packet);
			if (error != Error::OK)
				return error;
			// Copy payload part to response
			uint8_t size = packet.header.size & MASK_PAYLOAD_SIZE;
			if (total + size > max_size)
				return Error::RESPONSE_SIZE_OVERFLOW;
			memcpy(current, packet.payload, size);
			current += size;
			total += size;
			// Is this last packet to receive?
			if (!(packet.header.size & MASK_MORE_DATA))
				break;
		}
		max_size = total;
		return Error::OK;
	}

	void on_irq_high()
	{
		//TODO
	}

	void on_timer_compare()
	{
		//TODO
	}

	volatile OperationStatus operation_ = OperationStatus::NO_OP;

	using TIMER = timer::Timer<BLE_TIMER>;
	using CALC = TIMER::CALCULATOR;
	static constexpr const TIMER::PRESCALER PRESCALER = CALC::CTC_prescaler(DELAY_BEFORE_CS_US);
	static_assert(CALC::is_adequate_for_CTC(PRESCALER, DELAY_BEFORE_CS_US));
	static constexpr const uint16_t COUNTER = CALC::CTC_counter(PRESCALER, DELAY_BEFORE_CS_US);
	TIMER timer_ = TIMER{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};

	gpio::FAST_EXT_PIN<IRQ> irq_ = gpio::FAST_EXT_PIN<IRQ>{gpio::PinMode::INPUT};
	interrupt::INTSignal<IRQ> signal_ = interrupt::INTSignal<IRQ>{interrupt::InterruptTrigger::RISING_EDGE};
	gpio::FAST_PIN<RESET> reset_ = gpio::FAST_PIN<RESET>{gpio::PinMode::OUTPUT, true};

	DECL_TIMER_COMP_FRIENDS;
	DECL_INT_ISR_FRIENDS;
	friend int main();
};

// REGISTER_INT_ISR_METHOD(IRQ_PIN, IRQ, PublicDevice, &PublicDevice::on_irq_high)
// REGISTER_TIMER_COMPARE_ISR_METHOD(NTIMER, PublicDevice, &PublicDevice::on_timer_compare)

using streams::endl;
using streams::flush;
using streams::dec;
using streams::hex;
using Error = PublicDevice::Error;

streams::ostream& operator<<(streams::ostream& o, Error error)
{
	const flash::FlashStorage* label;
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
	}
	return o << label;
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

	// Start SPI interface
	//TODO Improve init() to reduce CS high/low
	spi::init();
	PublicDevice device;
	device.reset();
	out << F("SPI & device initialized") << endl;
	
	// // Try to send a minimal SDEP packet
	// bool ok = device.send_command("ATZ");
	// if (!ok)
	// 	out << F("ERROR during send_command()") << endl;

	// // Read response here
	// uint8_t response[64];
	// uint8_t size = 64;
	// ok = device.get_response(response, size);
	// if (!ok)
	// 	out << F("ERROR during get_response()") << endl;

	// out << F("Response (") << dec << size << ')' << endl;
	// for (uint8_t i = 0; i < size; ++i)
	// 	out << ' ' << hex << response[i];
	// out << endl;

	// Enable system events
	Error error = device.await_at_command("AT+EVENTENABLE=0x3");
	if (error != Error::OK)
		out << F("ERROR during await_at_command('AT+EVENTENABLE=0x3'): ") << error << endl;
	// Loop to check connect/disconnect events status
	while (true)
	{
		time::delay_ms(1000);
		static constexpr const uint8_t SIZE = 64;
		char response[SIZE+1];
		error = device.await_at_command("AT+EVENTSTATUS", response, SIZE);
		if (error != Error::OK)
			out << F("ERROR during await_at_command('AT+EVENTSTATUS'): ") << error << endl;
		out << F("EVENTSTATUS = ") << response << flush;
	}
	// Stop SPI device if needed
	// out << F("End") << endl;
}
