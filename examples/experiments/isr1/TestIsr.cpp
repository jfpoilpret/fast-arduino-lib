/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>
#include <fastarduino/flash.h>
#include <fastarduino/devices/hcsr04.h>
#include <fastarduino/int.h>
#include <fastarduino/pci.h>

REGISTER_RTT_ISR(0)

static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin ECHO = board::InterruptPin::D3_PD3_PCI2;
static constexpr const board::Timer TIMER = board::Timer::TIMER0;

using RTT = timer::RTT<TIMER>;
using SONAR = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO>;

//#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)	\
//CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
//ISR(CAT3(PCINT, PCI_NUM, _vect))															\
//{																							\
//	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
//	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);									\
//	auto handler = SERVO_HOLDER::handler();													\
//	handler->on_echo();																		\
//	if (handler->ready())																	\
//		CALL_HANDLER_(HANDLER, CALLBACK, uint16_t)(handler->latest_echo_us());				\
//}

class SonarListener
{
public:
	SonarListener(uint16_t min_mm):MIN_US{devices::sonar::distance_mm_to_echo_us(min_mm)}, led_{gpio::PinMode::OUTPUT}
	{
		interrupt::register_handler(*this);
	}
	
	void on_sonar(uint16_t echo_us)
	{
		if (echo_us && echo_us <= MIN_US)
			led_.set();
		else
			led_.clear();
	}
	
private:
	const uint16_t MIN_US;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE led_;
};

REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, 2, TRIGGER, ECHO, SonarListener, &SonarListener::on_sonar)

int main() __attribute__((OS_main));
int main()
{
	sei();
	
	SonarListener listener{150};
	
	RTT rtt;
	rtt.register_rtt_handler();
	rtt.begin();
	typename interrupt::PCIType<ECHO>::TYPE signal;
	signal.enable_pin<ECHO>();
	signal.enable();
	SONAR sensor{rtt};
	
	while (true)
	{
		sensor.async_echo();
		sensor.await_echo_us();
	}
}
