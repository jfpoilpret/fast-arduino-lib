//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * Asynchronous multiple sonar sensor read and threshold detection.
 * This program shows usage of FastArduino HCSR04 device API with PCI ISR and callbacks.
 * It is wired to 4 HCSR04, triggered at the same time, and as many feedback
 * LEDs, lit upon sonar echo value compared to a threshold.
 * Compared to Sonar10 example, this example auto triggers itself at a fixed
 * frequency. All logic is coded inside a dedicated class.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 * 		- D0-D3: 4 echo pins of HCSR04
 * 		- D4-D7: 4 LEDs (in series with 330 Ohm resistors)
 * 		- D8: connected to all 4 trigger pins of HCSR04
 */

#include <fastarduino/boards/board.h>

#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/queue.h>
#include <fastarduino/utilities.h>
#include <fastarduino/pci.h>
#include <fastarduino/devices/sonar.h>

using board::DigitalPin;
using gpio::FastPinType;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define TIMER_NUM 1
#define PCI_NUM 2
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const DigitalPin TRIGGER = DigitalPin::D8;
// Pins connected to each sonar echo
static constexpr const DigitalPin SFRONT = DigitalPin::D0;
static constexpr const DigitalPin SREAR = DigitalPin::D1;
static constexpr const DigitalPin SLEFT = DigitalPin::D2;
static constexpr const DigitalPin SRIGHT = DigitalPin::D3;
// Pins connected to LED
static constexpr const DigitalPin LFRONT = DigitalPin::D4;
static constexpr const DigitalPin LREAR = DigitalPin::D5;
static constexpr const DigitalPin LLEFT = DigitalPin::D6;
static constexpr const DigitalPin LRIGHT = DigitalPin::D7;
#else
#error "Current target is not yet supported!"
#endif

// perform static checks here to ensure all pins/ports are proper
static constexpr const board::Port ECHO_PORT = FastPinType<SFRONT>::PORT;
static_assert(ECHO_PORT == FastPinType<SREAR>::PORT, "SFRONT and SREAR must share the same PORT");
static_assert(ECHO_PORT == FastPinType<SLEFT>::PORT, "SFRONT and SLEFT must share the same PORT");
static_assert(ECHO_PORT == FastPinType<SRIGHT>::PORT, "SFRONT and SRIGHT must share the same PORT");

static constexpr const board::Port LED_PORT = FastPinType<LFRONT>::PORT;
static_assert(LED_PORT == FastPinType<LREAR>::PORT, "LFRONT and LREAR must share the same PORT");
static_assert(LED_PORT == FastPinType<LLEFT>::PORT, "LFRONT and LLEFT must share the same PORT");
static_assert(LED_PORT == FastPinType<LRIGHT>::PORT, "LFRONT and LRIGHT must share the same PORT");

struct EchoLed
{
	template<DigitalPin ECHO, DigitalPin LED>
	static constexpr const EchoLed create()
	{
		return EchoLed{FastPinType<ECHO>::MASK, FastPinType<LED>::MASK};
	}

	constexpr EchoLed(uint8_t echo, uint8_t led):echo{echo}, led{led} {}

	const uint8_t echo;
	const uint8_t led;
};

static constexpr const EchoLed ECHO_LEDS[] =
{
	// EchoLed::create<SFRONT, LFRONT>(),
	// EchoLed::create<SREAR, LREAR>(),
	EchoLed::create<SLEFT, LLEFT>(),
	EchoLed::create<SRIGHT, LRIGHT>()
};
static constexpr const uint8_t NUM_SONARS = sizeof(ECHO_LEDS) / sizeof(EchoLed);

static constexpr uint8_t echo_mask(uint8_t index = 0, uint8_t mask = 0)
{
	return (index < NUM_SONARS ? echo_mask(index + 1, ECHO_LEDS[index].echo | mask) : mask);
}
static constexpr uint8_t led_mask(uint8_t index = 0, uint8_t mask = 0)
{
	return (index < NUM_SONARS ? led_mask(index + 1, ECHO_LEDS[index].led | mask) : mask);
}

// define masks to use for ports dealing with sonar echo pins and LED pins
static constexpr const uint8_t ECHO_MASK = echo_mask();
static constexpr const uint8_t LED_MASK = led_mask();

using RTT = timer::RTT<NTIMER>;

// Declare device type to handle all sonars
using SONAR = devices::sonar::MultiHCSR04<NTIMER, TRIGGER, ECHO_PORT, ECHO_MASK>;

// Declare timer types and constants
static constexpr const uint16_t TIMEOUT_MAX = SONAR::DEFAULT_TIMEOUT_MS;
static constexpr const uint32_t TRIGGER_REPEAT_MS = 50UL;

using devices::sonar::distance_mm_to_echo_us;

static constexpr const uint16_t DISTANCE_THRESHOLD_MM = 100;
static constexpr const uint32_t DISTANCE_THRESHOLD = distance_mm_to_echo_us(DISTANCE_THRESHOLD_MM);

// The event pushed by SonarListener when sonars ranging is finished or timed out
class Event
{
public:
	Event() : sonar_{-1}, start_{RTT::RAW_TIME::EMPTY_TIME}, end_{RTT::RAW_TIME::EMPTY_TIME} {}
	Event(int8_t sonar, RTT::RAW_TIME start, RTT::RAW_TIME end) : sonar_{sonar}, start_{start}, end_{end} {}

	// the sonar index for the sonar that received an echo, or -1 if timeout occurred
	int8_t sonar() const
	{
		return sonar_;
	}
	// the time in us of the echo ranging for sonar()
	uint32_t time() const
	{
		return (end_.as_real_time() - start_.as_real_time()).total_micros();
	}

private:
	int8_t sonar_;
	RTT::RAW_TIME start_;
	RTT::RAW_TIME end_;
};

using QUEUE = containers::Queue<Event>;

// The handler of multi-sonar, including sonar callbacks
// This handler simply pushes events to a queue when something happens
class SonarListener
{
	using EVENT = devices::sonar::SonarEvent<NTIMER>;

public:
	SonarListener(QUEUE& queue)
	: signal_{}, rtt_{}, sonar_{rtt_}, queue_{queue}, times_buffer_{}
	{
		for (uint8_t i = 0; i < NUM_SONARS; ++i)
			times_[i] = RTT::RAW_TIME::EMPTY_TIME;
		interrupt::register_handler(*this);
	}

	// This starts sonar ranging process, it must be called only once, afterwards
	// new range triggers are auto-generated
	void start()
	{
		synchronized
		{
			signal_.set_enable_pins_(ECHO_MASK);
			signal_.enable_();
			rtt_.begin_();
			trigger();
		}
	}

private:
	void trigger()
	{
		sonar_.trigger(TIMEOUT_MAX);
		next_trigger_time_ = rtt_.millis_() + TRIGGER_REPEAT_MS;
	}

	void on_sonar(const EVENT& event)
	{
		// Calculate new status of LEDs for finished sonar echoes
		for (int8_t i = 0; i < NUM_SONARS; ++i)
		{
			if (ECHO_LEDS[i].echo & event.ready())
				queue_.push_(Event{i, times_[i], event.time()});
			if (ECHO_LEDS[i].echo & event.started())
				times_[i] = event.time();
		}
	}

	void on_timeout(const EVENT&)
	{
		queue_.push_(Event{});
	}

	void on_rtt()
	{
		if (rtt_.millis_() >= next_trigger_time_)
			trigger();
	}
	
	interrupt::PCISignal<PCI_NUM> signal_;
	RTT rtt_;
	SONAR sonar_;
	QUEUE& queue_;
	// This union is an ugly hack to allow 0 initialization of all times_
	// which is not possible otherwise because default RAW_TIME ctor is private.
	// otherwise, we would have to use times_{RTT::RAW_TIME::EMPTY_TIME, RTT::RAW_TIME::EMPTY_TIME}
	// as initializer but this must be changed if NUM_SONARS change...
	union
	{
		RTT::RAW_TIME times_[NUM_SONARS];
		uint8_t times_buffer_[sizeof(times_)];
	};
	uint32_t next_trigger_time_;

	DECL_SONAR_ISR_HANDLERS_FRIEND
};

// Register ISR callbacks
REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK,
	SonarListener, &SonarListener::on_sonar)

REGISTER_MULTI_HCSR04_RTT_TIMEOUT_TRIGGER_METHOD(TIMER_NUM, SONAR, 
	SonarListener, &SonarListener::on_timeout, &SonarListener::on_rtt)

// Queue buffer for Events sent by SonarListener once a sonar has finish ranging 
static constexpr const uint8_t QUEUE_SIZE = 8;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	Event event_buffer[QUEUE_SIZE];
	QUEUE queue{event_buffer};
	
	// Setup LED outputs
	gpio::FastMaskedPort<LED_PORT, LED_MASK> leds{0xFF};

	// Enable interrupts now
	sei();
	
	SonarListener listener{queue};
	listener.start();

	// Infinite loop to trigger sonar and light LEDs when near object is detected
	while (true)
	{
		Event event;
		if (queue.pull(event))
		{
			if (event.sonar() == -1)
				leds.set_PORT(0);
			else if (event.time() > DISTANCE_THRESHOLD)
				leds.set_PORT(leds.get_PIN() & ~ECHO_LEDS[event.sonar()].led);
			else
				leds.set_PORT(leds.get_PIN() | ECHO_LEDS[event.sonar()].led);
		}
	}
}
