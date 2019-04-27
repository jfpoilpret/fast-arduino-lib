#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

constexpr const board::Timer NTIMER = board::Timer::TIMER1;
using CALCULATOR = timer::Calculator<NTIMER>;
using TIMER = timer::Timer<NTIMER>;
constexpr const uint32_t PERIOD_US = 1000000;

constexpr const TIMER::PRESCALER PRESCALER = CALCULATOR::CTC_prescaler(PERIOD_US);
constexpr const TIMER::TYPE COUNTER = CALCULATOR::CTC_counter(PRESCALER, PERIOD_US);

class Handler
{
public:
	Handler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};

// Define vectors we need in the example
REGISTER_TIMER_COMPARE_ISR_METHOD(1, Handler, &Handler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	Handler handler;
	interrupt::register_handler(handler);
	TIMER timer{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	timer.begin(COUNTER);
	
	while (true) ;
}
