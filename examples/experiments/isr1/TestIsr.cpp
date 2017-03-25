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

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)

// Conversion method
static constexpr uint16_t distance_mm(uint16_t echo_us)
{
	// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
	return uint16_t(echo_us * 340UL / 1000UL / 2UL);
}

// Utilities to handle ISR callbacks
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)								\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");	\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,			\
	"PIN INT number must match INT_NUM");													\
ISR(CAT3(INT, INT_NUM, _vect))																\
{																							\
	using SERVO_HANDLER = HCSR04<TIMER, TRIGGER, ECHO >;									\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_echo)();								\
}

#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO)								\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
ISR(CAT3(PCINT, PCI_NUM, _vect))															\
{																							\
	using SERVO_HANDLER = HCSR04<TIMER, TRIGGER, ECHO >;									\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_echo)();								\
}

//TODO non blocking ultrasonic sensor
// - use RTT timer (template arg)
// - use pin interrupt (either EXT or PCI, both should be supported)
template<board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO>
class HCSR04
{
public:
	static constexpr const uint16_t DEFAULT_TIMEOUT_US = 4 * 2 * 1000000UL / 340 + 1;
	
	HCSR04(timer::RTT<TIMER>& rtt, bool async = true)
		:	rtt_{rtt}, 
			trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}, 
			start_{}, echo_pulse_{0}, ready_{false}
	{
		if (async)
			interrupt::register_handler(*this);
	}

	uint16_t echo_us(uint16_t timeout_us = DEFAULT_TIMEOUT_US)
	{
		rtt_.millis(0);
		// Pulse TRIGGER for 10us
		trigger_.set();
		time::delay_us(TRIGGER_PULSE_US);
		trigger_.clear();
		// Wait for echo signal start
		uint16_t timeout_ms = rtt_.millis() + timeout_us / 1000 + 1;
		while (!echo_.value())
			if (rtt_.millis() >= timeout_ms) return 0;
		// Read current time (need RTT)
		time::RTTTime start = rtt_.time();
		// Wait for echo signal end
		while (echo_.value())
			if (rtt_.millis() >= timeout_ms) return 0;
		// Read current time (need RTT)
		time::RTTTime end = rtt_.time();
		time::RTTTime delta = time::delta(start, end);
		return uint16_t(delta.millis * 1000UL + delta.micros);
	}
	
	void async_echo()
	{
		ready_ = false;
		rtt_.millis(0);
		// Pulse TRIGGER for 10us
		trigger_.set();
		time::delay_us(TRIGGER_PULSE_US);
		trigger_.clear();
	}
	
	bool ready() const
	{
		return ready_;
	}
	
	uint16_t await_echo_us(uint16_t timeout_us = DEFAULT_TIMEOUT_US)
	{
		// Wait for echo signal start
		uint16_t timeout_ms = rtt_.millis() + timeout_us / 1000 + 1;
		while (!ready_)
			if (rtt_.millis() >= timeout_ms)
			{
				ready_ = true;
				return 0;
			}
		return echo_pulse_;
	}
	
	//TODO callback from PCI/EXT
	void on_echo()
	{
		if (echo_.value())
		{
			// pulse started
			start_ = rtt_.time();
			started_ = true;
		}
		else if (started_)
		{
			// pulse ended
			time::RTTTime end = rtt_.time();
			time::RTTTime delta = time::delta(start_, end);
			echo_pulse_ = uint16_t(delta.millis * 1000UL + delta.micros);
			ready_ = true;
			started_ = false;
		}
	}
	
private:
	static constexpr const uint16_t TRIGGER_PULSE_US = 10;

	timer::RTT<TIMER>& rtt_;
	typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
	typename gpio::FastPinType<ECHO>::TYPE echo_;
	time::RTTTime start_;
	volatile uint16_t echo_pulse_;
	//TODO optimize space: only 2 bits needed here!
	volatile bool ready_;
	volatile bool started_;
};

static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO = board::ExternalInterruptPin::D3_PD3_EXT1;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

using RTT = timer::RTT<TIMER>;
using PROXI = HCSR04<TIMER, TRIGGER, ECHO>;

REGISTER_HCSR04_INT_ISR(TIMER, 1, TRIGGER, ECHO)

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
	rtt.begin();
	interrupt::INTSignal<ECHO> signal;
	signal.enable();
	PROXI sensor{rtt};
	
	out << F("Starting...\n") << streams::flush;
	
	while (true)
	{
//		uint16_t pulse = sensor.echo_us();
		sensor.async_echo();
		uint16_t pulse = sensor.await_echo_us();
		uint32_t timing = rtt.millis();
		uint16_t mm = distance_mm(pulse);
		// trace value to output
		out << F("Pulse: ") << pulse  << F(" us. Distance: ") << mm << F(" mm (duration = ") << timing << F(" ms)\n") << streams::flush;
		time::delay_ms(1000);
	}
}
