//   Copyright 2016-2017 Jean-Francois Poilpret
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
 * It is wired to several HCSR04, triggered at the same time, and as many feedback
 * LEDs, lit upon sonar echo value compared to a threshold.
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

#include <fastarduino/devices/hcsr04.h>

using board::DigitalPin;
using gpio::FastPinType;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define TIMER_NUM 1
#define PCI_NUM 0
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
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
//TODO also assert PCI_NUM for ECHO_PORT match (how without access to trait?)

static constexpr const board::Port LED_PORT = FastPinType<LFRONT>::PORT;
static_assert(LED_PORT == FastPinType<LREAR>::PORT, "LFRONT and LREAR must share the same PORT");
static_assert(LED_PORT == FastPinType<LLEFT>::PORT, "LFRONT and LLEFT must share the same PORT");
static_assert(LED_PORT == FastPinType<LRIGHT>::PORT, "LFRONT and LRIGHT must share the same PORT");

struct EchoLed
{
	template<DigitalPin ECHO, DigitalPin LED>
	static constexpr const EchoLed create()
	{
		return EchoLed{_BV(FastPinType<ECHO>::BIT), _BV(FastPinType<LED>::BIT)};
	}

	constexpr EchoLed(uint8_t echo, uint8_t led):echo{echo}, led{led} {}

	const uint8_t echo;
	const uint8_t led;
};

static constexpr const EchoLed ECHO_LEDS[] =
{
	EchoLed::create<SFRONT, LFRONT>(),
	EchoLed::create<SREAR, LREAR>(),
	EchoLed::create<SLEFT, LLEFT>()
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
static constexpr const uint8_t SONAR_MASK = echo_mask();
static constexpr const uint8_t LED_MASK = led_mask();

// Declate device type to handle all sonars
using SONAR = devices::sonar::MultiHCSR04<TIMER, TRIGGER, ECHO_PORT, SONAR_MASK>;

// Declare timer types and constants
using TIMER_TYPE = timer::Timer<TIMER>;
using CALC = timer::Calculator<TIMER>;
static constexpr const uint32_t PRECISION = SONAR::DEFAULT_TIMEOUT_MS * 1000UL;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::CTC_prescaler(PRECISION);

using devices::sonar::distance_mm_to_echo_us;

static constexpr const uint16_t DISTANCE_THRESHOLD_MM = 150;
static constexpr const SONAR::TYPE DISTANCE_THRESHOLD_TICKS = 
	CALC::us_to_ticks(PRESCALER, distance_mm_to_echo_us(DISTANCE_THRESHOLD_MM)); 

using QUEUE = containers::Queue<SONAR::EVENT>;

class SonarListener
{
public:
	SonarListener(QUEUE& queue):queue_{queue}
	{
		interrupt::register_handler(*this);
	}
	
	void on_sonar(SONAR::EVENT event)
	{
		// queue event for main loop to use it
		queue_._push(event);
	}
	
private:
	QUEUE& queue_;
};

REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(PCI_NUM, SONAR, SonarListener, &SonarListener::on_sonar)

static constexpr const uint8_t QUEUE_SIZE = 8;
static SONAR::EVENT event_buffer[QUEUE_SIZE];

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	gpio::FastMaskedPort<LED_PORT> leds{LED_MASK, 0xFF};

	QUEUE queue{event_buffer};

	// Start timer
	TIMER_TYPE timer{timer::TimerMode::NORMAL, PRESCALER};
	timer.begin();
	SONAR sonar{timer};

	SonarListener listener{queue};

	while (true)
	{
		sonar.trigger();
		while (!sonar.all_ready())
		{
			SONAR::EVENT event;
			SONAR::TYPE ticks[NUM_SONARS];
			//TODO use pull function that yields instead?
			while (queue.pull(event))
			{
				// Calculate new status of LEDs for finished sonar echoes
				uint8_t alarms = 0;
				for (uint8_t i = 0; i < NUM_SONARS; ++i)
				{
					if (ECHO_LEDS[i].echo & event.started)
						ticks[i] = event.ticks;
					else if (	(ECHO_LEDS[i].echo & event.ready)
							&&	(event.ticks - ticks[i] <= DISTANCE_THRESHOLD_TICKS))
						alarms |= ECHO_LEDS[i].led;
				}
				// Update LEDs as needed
				leds.set_PORT(alarms | (leds.get_PIN() & ~event.ready));
			}
		}
	}
}
