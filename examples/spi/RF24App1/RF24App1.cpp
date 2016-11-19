/*
 * NRF24L01+ test example.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>
#include <fastarduino/time.hh>
#include <fastarduino/NRF24L01.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HAS_TRACE 1
static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D8;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D9;
static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER2;

USE_RTT_TIMER2();
#elif defined (BREADBOARD_ATTINYX4)
#define HAS_TRACE 0
static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D2;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D3;
static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER0;

// Define vectors we need in the example
USE_RTT_TIMER0();
#else
#error "Current target is not yet supported!"
#endif

#if HAS_TRACE
#include <fastarduino/uart.hh>
// Buffers for UART
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
USE_UATX0();
#else
#include <fastarduino/empty_streams.hh>
#endif

static const uint16_t NETWORK = 0xFFFF;
static const uint8_t MASTER = 0x01;
static const uint8_t SLAVE = 0x02;

static const uint32_t REPLY_MAX_WAIT_MS = 1000L;
static const uint32_t RECEIVE_MAX_WAIT_MS = 10000L;
static const uint32_t DELAY_BETWEEN_2_FRAMES_MS = 100L;

static bool is_master()
{
	FastPin<PIN_CONFIG> config{PinMode::INPUT_PULLUP};
	return config.value();
}

int main()
{
	// Enable interrupts at startup time
	sei();

#if HAS_TRACE
	// Setup traces
	UATX<Board::USART::USART0> uatx{output_buffer};
	uatx.begin(115200);
	auto trace = uatx.fout();
#else
	EmptyOutput trace;
#endif
	
	bool master = is_master();
	uint8_t self_device = master ? MASTER : SLAVE;
	uint8_t other_device = master ? SLAVE : MASTER;
	trace << "RF24App1 started as " << (master ? "Master" : "Slave") << endl << flush;

	// Setup RTT
	RTT<RTT_TIMER> rtt;
	rtt.begin();
	// Set RTT instance as default clock from now
	Time::set_clock(rtt);
	trace << "RTT started\n" << flush;

	// Start SPI and setup NRF24
	FastSPI::init();
	NRF24L01<PIN_CSN, PIN_CE> rf{NETWORK, self_device};
	rf.begin();
	trace << "NRF24L01+ started\n" << flush;
	
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
			trace << "S " << sent_port << flush;
			int result = rf.send(other_device, sent_port, 0, 0);
			if (result < 0)
				trace	<< "\nError " << result << "! #Trans=" << rf.get_trans() 
						<< " #Retrans=" << rf.get_retrans() 
						<< " #Drops=" << rf.get_drops() << '\n' << flush;
			
			// Then wait for slave reply
			trace << " R " << flush;
			uint8_t src, port;
			result = rf.recv(src, port, 0, 0, REPLY_MAX_WAIT_MS);
			if (result < 0)
				trace << "\nError " << result << "!\n" << flush;
			else
				trace << uint16_t(port) << " (" << uint16_t(src) << ") " << flush;
			
			// Wait 1 second before doing it again
			++sent_port;
			Time::delay(DELAY_BETWEEN_2_FRAMES_MS);
		}
		else
		{
			// Wait for master payload
			trace << "R " << flush;
			uint8_t src, port;
			int result = rf.recv(src, port, 0, 0);
			if (result < 0)
				trace << "\nError " << result << "!\n" << flush;
			else
			{
				trace << uint16_t(port) << " (" << uint16_t(src) << ") RR " << flush;
				// Reply to master with same content
				result = rf.send(src, port, 0, 0);
				if (result < 0)
					trace	<< "\nError " << result << "! #Trans=" << rf.get_trans() 
							<< " #Retrans=" << rf.get_retrans() 
							<< " #Drops=" << rf.get_drops() << '\n' << flush;
			}
		}
		if (++count % 1000 == 0)
			trace << "\n count = " << count << "\n#Trans=" << rf.get_trans() 
					<< " #Retrans=" << rf.get_retrans() 
					<< " #Drops=" << rf.get_drops() << '\n' << flush;
	}
}
