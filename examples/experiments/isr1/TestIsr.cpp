/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/uart.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>
#include <fastarduino/flash.h>

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)

// Conversion method
static constexpr uint16_t distance_mm(uint16_t echo_us)
{
	return uint16_t(echo_us * 1000UL / 2UL / 340UL);
}

//TODO non blocking ultrasonic sensor
// - use RTT timer (template arg)
// - use pin interrupt (either EXT or PCI, both should be supported)
template<board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO>
class HCSR04
{
public:
	static constexpr const uint16_t DEFAULT_TIMEOUT_US = 4 * 2 * 1000000UL / 340 + 1;
	
	HCSR04(timer::RTT<TIMER>& rtt)
		:	rtt_{rtt}, 
			trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}, 
			start_{}, echo_pulse_{0}, ready_{false} {}

	uint16_t echo_us(uint16_t timeout_us = DEFAULT_TIMEOUT_US)
	{
		// Pulse TRIGGER for 10us
		trigger_.set();
		time::delay_us(TRIGGER_PULSE_US);
		trigger_.clear();
		// Read current time (need RTT)
		time::RTTTime start = rtt_.time();
		while (!echo_.value()) ;
		// Read current time (need RTT)
		time::RTTTime end = rtt_.time();
		time::RTTTime delta = time::delta(start, end);
		return uint16_t(delta.millis * 1000 + delta.micros);
	}
	
	//TODO later: async API
	void async_echo();
	bool ready() const
	{
		return ready_;
	}
	uint16_t await_echo_us(uint16_t timeout = DEFAULT_TIMEOUT_US);
	//TODO callback from PCI/EXT
	void on_echo();
	
private:
	static constexpr const uint16_t TRIGGER_PULSE_US = 10;
	
	timer::RTT<TIMER>& rtt_;
	typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
	typename gpio::FastPinType<ECHO>::TYPE echo_;
	time::RTTTime start_;
	volatile uint16_t echo_pulse_;
	volatile bool ready_;
};

static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO = board::DigitalPin::D3_PD3;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<TIMER>;
using PROXI = HCSR04<TIMER, TRIGGER, ECHO>;

int main() __attribute__((OS_main));
int main()
{
	sei();
	
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.register_handler();
	uart.begin(115200);
	auto out = uart.fout();
	
	RTT rtt;
	rtt.register_rtt_handler();
	PROXI sensor{rtt};
	rtt.begin();
	
	out << F("Starting...\n") << streams::flush;
	
	while (true)
	{
		uint16_t pulse = sensor.echo_us();
		uint16_t mm = distance_mm(pulse);
		//TODO trace value to output
		out << F("Distance: ") << mm << F(" mm\n") << streams::flush;
		time::delay_ms(1000);
	}
}
