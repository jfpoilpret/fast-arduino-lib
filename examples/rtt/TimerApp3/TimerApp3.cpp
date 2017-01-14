/*
 * Timer compilation example.
 * TODO
 */

#include <fastarduino/FastIO.hh>
#include <fastarduino/Timer.hh>

constexpr const Board::Timer TIMER = Board::Timer::TIMER1;
// Define vectors we need in the example
USE_TIMERS(1);

using TIMER_TYPE = Timer<TIMER>;
constexpr const uint32_t PERIOD_US = 1000000;

constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::prescaler(PERIOD_US);
static_assert(TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US), "TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US)");
constexpr const TIMER_TYPE::TIMER_TYPE COUNTER = TIMER_TYPE::counter(PRESCALER, PERIOD_US);

class Handler: public TimerCallback
{
public:
	Handler(): _led{PinMode::OUTPUT, false} {}
	
	virtual void on_timer() override
	{
		_led.toggle();
	}
	
private:
	FastPinType<Board::DigitalPin::LED>::TYPE _led;
};

int main()
{
	sei();
	Handler handler;
	TIMER_TYPE timer{handler};
	timer.begin(PRESCALER, COUNTER);
	
	while (true) ;
}
