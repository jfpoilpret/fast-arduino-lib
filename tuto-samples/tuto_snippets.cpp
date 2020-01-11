// This source code is not compilable it it here to gather all additional snippets
// shown in the tutorial but do not belong to any complete example.
// We include all necessary headers to ensure that code is understood by doxygen
#define ARDUINO_UNO
#include <fastarduino/boards/board.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/events.h>
#include <fastarduino/queue.h>
#include <fastarduino/scheduler.h>
#include <fastarduino/utilities.h>
#include <fastarduino/devices/hmc5883l.h>
#include <fastarduino/devices/ds1307.h>

void analoginput_8bits() {
//! [analoginput_8bits]
    analog::AnalogInput<board::AnalogPin::A0, board::AnalogReference::AVCC, uint8_t> sensor;
//! [analoginput_8bits]
}

void rtt_hcsr04() {
//! [rtt_hcsr04]
// Declare 2 pins connected to HC-SR04
gpio::FAST_PIN<board::DigitalPin::D0> trigger{gpio::PinMode::OUTPUT};
gpio::FAST_PIN<board::DigitalPin::D1> echo{gpio::PinMode::INPUT};

// Declare RTT (note: don't forget to call REGISTER_RTT_ISR(1) macro in your program)
timer::RTT<board::Timer::TIMER1>& rtt;

// Send a 10us pulse to the trigger pin
trigger.set();
time::delay_us(10);
trigger.clear();

// Wait for echo signal start
while (!echo.value()) ;
// Reset RTT time
rtt.millis(0);
// Wait for echo signal end
while (echo.value()) ;
// Read current time
time::RTTTime end = rtt.time();
// Calculate the echo duration in microseconds
uint16_t echo_us = uint16_t(end.millis() * 1000UL + end.micros());
//! [rtt_hcsr04]
}

using devices::magneto::MagneticFields;
//! [utils_swap_bytes]
    bool magnetic_fields(MagneticFields& fields)
    {
        if (	this->write(DEVICE_ADDRESS, OUTPUT_REG_1, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
            &&	this->read(DEVICE_ADDRESS, fields, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
        {
            utils::swap_bytes(fields.x);
            utils::swap_bytes(fields.y);
            utils::swap_bytes(fields.z);
            return true;
        }
        else
            return false;
    }
//! [utils_swap_bytes]


using devices::rtc::tm;
//! [utils_bcd_to_binary]
    bool getDateTime(tm& datetime)
    {
        // send register address to read from (0)
        // read datetime at address 0
        if (	write(DEVICE_ADDRESS, TIME_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
            &&	read(DEVICE_ADDRESS, datetime, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
        {
            // convert DS1307 output (BCD) to integer type
            datetime.tm_sec = utils::bcd_to_binary(datetime.tm_sec);
            datetime.tm_min = utils::bcd_to_binary(datetime.tm_min);
            datetime.tm_hour = utils::bcd_to_binary(datetime.tm_hour);
            datetime.tm_mday = utils::bcd_to_binary(datetime.tm_mday);
            datetime.tm_mon = utils::bcd_to_binary(datetime.tm_mon);
            datetime.tm_year = utils::bcd_to_binary(datetime.tm_year);
        }
    }
//! [utils_bcd_to_binary]

using utils::UnitPrefix;
using utils::map_physical_to_raw;
//! [utils_map_physical_to_raw]
	static constexpr const int16_t ACCEL_1 = map_physical_to_raw(500, UnitPrefix::MILLI, 2, 15);
//! [utils_map_physical_to_raw]

using utils::map_raw_to_physical;
void utils_map_raw_to_physical() {
	int raw = 12345;
//! [utils_map_raw_to_physical]
	int16_t rotation = map_raw_to_physical(raw, UnitPrefix::CENTI, 250, 15);
//! [utils_map_raw_to_physical]
}

//! [events_types]
		const uint8_t NO_EVENT = 0;
		const uint8_t WDT_TIMER = 1;
		const uint8_t RTT_TIMER = 2;

		// User-defined events start here (in range [128-255]))
		const uint8_t USER_EVENT = 128;
//! [events_types]


void events_1_blink_push() {
	using EVENT = events::Event<void>;
	const uint8_t BUTTON_EVENT = events::Type::USER_EVENT;
	gpio::FastPort<board::Port::PORT_B> buttons_;
	EVENT buffer[32];
	containers::Queue<EVENT> event_queue_{buffer};
//! [events_1_blink_push]
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
//! [events_1_blink_push]


	containers::Queue<EVENT>& event_queue = event_queue_;
	const uint8_t EVENT_TYPE_1 = events::Type::USER_EVENT + 1;
	const uint8_t EVENT_TYPE_2 = events::Type::USER_EVENT + 2;
	const uint8_t EVENT_TYPE_3 = events::Type::USER_EVENT + 3;
//! [events_loop_example]
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		switch (event.type())
        {
            case EVENT_TYPE_1:
            // Do something
            break;

            case EVENT_TYPE_2:
            // Do something else
            break;

            case EVENT_TYPE_3:
            // Do yet something else
            break;

            ...
        }
	}
//! [events_loop_example]


using events::EventHandler;
using namespace events::Type;
//! [events_handler_example]
class MyHandler: public EventHandler<EVENT>
{
public:
	MyHandler() : EventHandler<EVENT>{Type::USER_EVENT} {}
	virtual void on_event(const EVENT& event) override
	{
        // Do something
	}
};

int main()
{
    ...
	// Prepare Handlers
	MyHandler handler;
    ...
	
	// Prepare Dispatcher and register Handlers
	Dispatcher<EVENT> dispatcher;
	dispatcher.insert(handler);
    ...
}
//! [events_handler_example]


void events_dispatcher_loop() {
//! [events_dispatcher_loop]
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
//! [events_dispatcher_loop]
}


//! [events_rtt_scheduler]
#include <fastarduino/realtime_timer.h>
...
static const uint16_t RTT_EVENT_PERIOD = 1024;
REGISTER_RTT_EVENT_ISR(0, EVENT, RTT_EVENT_PERIOD)
...
void main()
{
    ...
	timer::RTT<board::Timer::TIMER0> rtt;
	timer::RTTEventCallback<EVENT, RTT_EVENT_PERIOD> callback{event_queue};
	interrupt::register_handler(callback);
    ...
	Scheduler<timer::RTT<board::Timer::TIMER0>, EVENT> scheduler{rtt, Type::RTT_TIMER};
    ...
	rtt.begin();
    ...
}
//! [events_rtt_scheduler]


//! [events_rtt_scheduler_snip1]
	timer::RTT<board::Timer::TIMER0> rtt;
    ...
	rtt.begin();
//! [events_rtt_scheduler_snip1]


//! [events_rtt_scheduler_snip2]
	timer::RTTEventCallback<EVENT, RTT_EVENT_PERIOD> callback{event_queue};
	interrupt::register_handler(callback);
//! [events_rtt_scheduler_snip2]


//! [events_rtt_scheduler_snip3]
REGISTER_RTT_EVENT_ISR(0, EVENT, RTT_EVENT_PERIOD)
//! [events_rtt_scheduler_snip3]


//! [events_rtt_scheduler_snip4]
	Scheduler<timer::RTT<board::Timer::TIMER0>, EVENT> scheduler{rtt, Type::RTT_TIMER};
//! [events_rtt_scheduler_snip4]


void eeprom_tone_play_stop() {
	while (true) {
//! [eeprom_tone_play_stop]
		EEPROM::read(play, tone);
		if (tone.tone == Tone::END)
			break;
//! [eeprom_tone_play_stop]
	}
}
