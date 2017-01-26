/*
 * NRF24L01+ ping/pong example.
 * This program shows usage of FastArduino support for SPI and NRF24L01+ device. It also uses FastArduino RTT and
 * time support.
 * The program should be uploaded to 2 boards (these can be 2 different boards).
 * One board will act as "master" (initiates all exchanges), the other one as a "slave", will wait for
 * master requests and will send reply after each received request.
 * Master/Slave selection is performed by grounding PIN_CONFIG (if slave) or keep it floating (if master).
 * For boards having a hardware USART, traces of all exchanges (and errors) are sent to it.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1 (TX) used for tracing program activities
 *   - D7 master/slave configuration pin
 *   - D13 (SCK), D12 (MISO), D11 (MOSI), D8 (CSN): SPI interface to NRF24L01+
 *   - D9 (CE): interface to NRF24L01+
 *   - D2 (EXT0, IRQ): interface to NRF24L01+
 * - on Arduino MEGA:
 *   - D1 (TX) used for tracing program activities
 *   - D7 master/slave configuration pin
 *   - D52 (SCK), D50 (MISO), D51 (MOSI), D8 (CSN): SPI interface to NRF24L01+
 *   - D9 (CE): interface to NRF24L01+
 *   - D21 (EXT0, IRQ): interface to NRF24L01+
 * - on ATtinyX4 based boards:
 *   - D7 master/slave configuration pin
 *   - D4 (SCK), D6 (MISO), D5 (MOSI), D2 (CSN): SPI interface to NRF24L01+
 *   - D3 (CE): interface to NRF24L01+
 *   - D10 (EXT0, IRQ): interface to NRF24L01+
 * 
 * Note: this example does use NRF24L01+ IRQ pin to wake up the active waiting loop during reception.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>
#include <fastarduino/time.hh>
#include <fastarduino/devices/NRF24L01.hh>
#include <fastarduino/uart.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HAS_TRACE 1
static const constexpr Board::DigitalPin PIN_IRQ = Board::ExternalInterruptPin::EXT0;
static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D8;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D9;
static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER2;

// Define vectors we need in the example
REGISTER_RTT_ISR(2)
#elif defined(ARDUINO_MEGA)
#define HAS_TRACE 1
static const constexpr Board::DigitalPin PIN_IRQ = Board::ExternalInterruptPin::EXT0;
static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D8;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D9;
static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER2;

// Define vectors we need in the example
REGISTER_RTT_ISR(2)
#elif defined (BREADBOARD_ATTINYX4)
#define HAS_TRACE 0
static const constexpr Board::DigitalPin PIN_IRQ = Board::ExternalInterruptPin::EXT0;
static const constexpr Board::DigitalPin PIN_CONFIG = Board::DigitalPin::D7;
static const constexpr Board::DigitalPin PIN_CSN = Board::DigitalPin::D2;
static const constexpr Board::DigitalPin PIN_CE = Board::DigitalPin::D3;
static const constexpr Board::Timer RTT_TIMER = Board::Timer::TIMER0;

// Define vectors we need in the example
REGISTER_RTT_ISR(0)
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

// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0)

static bool is_master()
{
	typename FastPinType<PIN_CONFIG>::TYPE config{PinMode::INPUT_PULLUP};
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
	rtt.register_rtt_handler();
	rtt.begin();
	// Set RTT instance as default clock from now
	Time::set_clock(rtt);
	trace << "RTT started\n" << flush;

	// Start SPI and setup NRF24
	SPI::init();
	IRQ_NRF24L01<PIN_CSN, PIN_CE, PIN_IRQ> rf{NETWORK, self_device};
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
