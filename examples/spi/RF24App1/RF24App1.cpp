/*
 * NRF24L01+ test example.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>
#include <fastarduino/time.hh>
#include <fastarduino/NRF24L01.hh>
#include <fastarduino/uart.hh>

static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;

static const constexpr Board::ExternalInterruptPin PIN_IRQ = Board::ExternalInterruptPin::EXT0;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D8;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D9;

static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER2;

// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static const uint16_t NETWORK = 0xFFFF;
static const uint8_t MASTER = 0x01;
static const uint8_t SLAVE = 0x02;

static const uint32_t RECEIVE_MAX_WAIT_MS = 1000L;

// Define vectors we need in the example
USE_EMPTY_INT0();
USE_RTT_TIMER2();
USE_UATX0()

static bool is_master()
{
	FastPin<PIN_CONFIG> config{PinMode::INPUT_PULLUP};
	return config.value();
}

int main()
{
	// Enable interrupts at startup time
	sei();

	// Setup traces
	UATX<Board::USART::USART0> uatx{output_buffer};
	uatx.begin(115200);
	auto trace = uatx.fout();
	
	bool master = is_master();
	uint8_t self_device = master ? MASTER : SLAVE;
	uint8_t other_device = master ? SLAVE : MASTER;
	trace << "RF24App1 started as " << (master ? "Master" : "Slave") << endl << flush;

	// Setup RTT
	RTT<Board::Timer::TIMER2> rtt;
	rtt.begin();
	// Set RTT instance as default clock from now
	Time::set_clock(rtt);
	trace << "RTT started\n" << flush;

	// Start SPI and setup NRF24
	SPI::SPIDevice::init();
	NRF24L01<Board::ExternalInterruptPin::EXT0> rf{NETWORK, self_device, PIN_CSN, PIN_CE};
	rf.begin();
	trace << "NRF24L01+ started\n" << flush;
	
	// Event Loop
	uint8_t sent_port = 0;
	uint16_t count_loops = 0;
	bool send = master;
	while (true)
	{
		// Reset RTT milliseconds counter to avoid overflow
		rtt.millis(0);
		if (send)
		{
			// Send
			trace << "Sending... " << flush;
			int result = rf.send(other_device, sent_port, 0, 0);
			if (result < 0)
			{
				trace	<< "Error " << result << "! #Trans=" << rf.get_trans() 
						<< " #Retrans=" << rf.get_retrans() 
						<< " #Drops=" << rf.get_drops() << '\n' << flush;
			}
			else
			{
				trace << "OK\n" << flush;
			}
			++sent_port;
		}
		else
		{
			// Receive
			trace << "Receiving...\n" << flush;
			uint8_t src, port;
			int result = rf.recv(src, port, 0, 0, RECEIVE_MAX_WAIT_MS);
			if (result < 0)
			{
				trace << "Error " << result << "!\n" << flush;
			}
			else
			{
				trace << "Received port " << uint16_t(port) << " from source " << uint16_t(src) << endl << flush;
			}
		}
		send = !send;
		if (!sent_port)
		{
			++count_loops;
			trace << "Total loops: " << count_loops << endl << flush;
		}
		Time::delay_ms(5000);
	}
}
