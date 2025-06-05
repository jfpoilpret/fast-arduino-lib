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

//TODO POC NEXT
// 30'	- improve timing: 
//			- start send_packet immediately (at_command)
//			- start get packet immediately (on_irq
// 1h+	- better error handling 
//			- handling of bad message types, bad commands, size overflows...
// 2h+	- implement GATT API (how?) with callbacks: use Builder pattern?
// 1h	- implement "basic" commands (e.g. factory reset)
// 1h	- do we need to call the init command? when? timing?
// 1h	- add ostream for AT commands (how to trigger sending?)
// 1h	- add istream for AT responses (how to trigger end of reception?)
// 2h+	- implement UART API (to be inferred thoroughly first)
//			- support data/command mode?

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
	static constexpr const uint16_t DELAY_TIMER_US = 60;

	// Retry times (in microseconds) in await loop
	static constexpr const uint16_t DELAY_AWAIT_US = 500;

	// Timeouts (in microseconds) for aborting waits on 1st byte OK or received IRQ 
	static constexpr const uint16_t TIMEOUT_DEVICE_READY_US = 1000;
	static constexpr const uint16_t TIMEOUT_IRQ_WAIT_US = 10000;

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

	bool get_packet()
	{
		// Start SPI transfer, send 0xFF and ensure we get a non error return byte (0xFE or 0xFF)
		uint8_t type = 0xFF;
		if (!send_type(type, false))
			return false;
	
		// Command ID
		UNUSED const uint8_t low = transfer(0xFF);
		UNUSED const uint8_t high = transfer(0xFF);

		// Payload size
		const uint8_t size_byte = transfer(0xFF);
		const uint8_t size = size_byte & MASK_PAYLOAD_SIZE;
		const bool more_data = size_byte & MASK_MORE_DATA;

		//TODO Check total size is not over buffer_ size! => Error!

		// Payload
		transfer(current_, size, 0xFF);
	
		// End SPI transfer
		end_transfer();

		// Was it the last packet to receive?
		size_ += size;
		if (more_data)
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

	Error check_ok(const char* response)
	{
		char* found = strstr(response, "OK\r\n");
		return (found != 0 ? Error::OK : Error::RESPONSE_WITH_ERROR);
	}

public:
	static constexpr uint16_t loop_count_for_cs(uint16_t timeout_us)
	{
		return timeout_us / DELAY_TIMER_US;
	}
	static constexpr uint16_t loop_count_for_irq(uint16_t timeout_us)
	{
		return timeout_us / DELAY_TIMER_US;
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

	// Launch AT command synchronously
	Error await_at_command(const char* command)
	{
		char response[BUFFER_SIZE + 1];
		return await_at_command(command, response, BUFFER_SIZE);
	}

	// Launch AT command synchronously
	Error await_at_command(const char* command, char* response, uint8_t size)
	{
		if (!at_command(command))
			return Error::BUSY;
		Error error = error_;
		await_response(response, size, error);
		return error;
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

		//TODO Try to send 1st packet immediately!
		// Start timer to send 1st byte of 1st packet
		timer_.resume_interrupts();
		return true;
	}

	// Return false if there is nothing to await for (typically because method called twice for 1 exchange)
	bool await_response(char* response, UNUSED uint8_t max_size, Error& error)
	{
		if (operation_ == OperationStatus::NO_OP)
			return false;
		while (operation_ != OperationStatus::FINISHED)
		{
			//FIXME wait timeout? to avoid infinite loop (even though it shall never happen... normally)
			time::delay_us(DELAY_AWAIT_US);
		}
		// Calculate everything
		operation_ = OperationStatus::NO_OP;
		//TODO check size overflow!!!
		memcpy(response, buffer_, size_);
		response[size_] = 0;
		// Check if OK in response
		error = (error_ == Error::OK ? check_ok(response) : error_);
		return true;
	}

private:
	void on_irq_high()
	{
		// Double check IRQ level (you never know)
		if (irq_.value())
		{
			operation_ = OperationStatus::RECEIVING_PACKET;
			wait_loop_count_ = loop_count_for_cs(TIMEOUT_DEVICE_READY_US);
			signal_.disable_();
			//TODO Try to get 1st packet immediately?
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
			//TODO ERROR
			return;
		}
	}

	// Data for on going exchange operation
	// Current operation phase
	volatile OperationStatus operation_ = OperationStatus::NO_OP;
	// Latest error occurred during operation
	volatile Error error_ = Error::OK;
	// Count of wait loops for acceptable 1st received byte
	uint16_t wait_loop_count_;
	// Buffer used both for sending command and received response
	static constexpr const uint8_t BUFFER_SIZE = 64;
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
	return o << label << flush;
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
	
	// Enable system events
	Error error = device.await_at_command("AT+EVENTENABLE=0x3");
	if (error != Error::OK)
		out << F("ERROR during await_at_command('AT+EVENTENABLE=0x3'): ") << endl << error << endl;
	// Loop to check connect/disconnect events status
	while (true)
	{
		time::delay_ms(1000);
		static constexpr const uint8_t SIZE = 64;
		char response[SIZE+1];
		error = device.await_at_command("AT+EVENTSTATUS", response, SIZE);
		if (error != Error::OK)
			out << F("ERROR during await_at_command('AT+EVENTSTATUS'): ") << endl << error << endl;
		out << F("EVENTSTATUS = ") << response << endl;
	}
	// Stop SPI device if needed
	// out << F("End") << endl;
}
