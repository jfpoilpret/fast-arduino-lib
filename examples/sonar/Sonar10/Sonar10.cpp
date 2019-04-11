//   Copyright 2016-2019 Jean-Francois Poilpret
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
static constexpr const uint8_t ECHO_MASK = echo_mask();
static constexpr const uint8_t LED_MASK = led_mask();

REGISTER_RTT_ISR(TIMER_NUM)

using RTT = timer::RTT<NTIMER>;

// Declate device type to handle all sonars
using SONAR = devices::sonar::MultiHCSR04<NTIMER, TRIGGER, ECHO_PORT, ECHO_MASK>;

// Declare timer types and constants
static constexpr const uint16_t TIMEOUT_MAX = SONAR::DEFAULT_TIMEOUT_MS;

using devices::sonar::distance_mm_to_echo_us;

static constexpr const uint16_t DISTANCE_THRESHOLD_MM = 150;
static constexpr const uint16_t DISTANCE_THRESHOLD = 
	distance_mm_to_echo_us(DISTANCE_THRESHOLD_MM);

using devices::sonar::SonarEvent;
using QUEUE = containers::Queue<SonarEvent>;

class SonarListener
{
public:
	SonarListener(SONAR& sonar, QUEUE& queue):sonar_{sonar}, queue_{queue}
	{
		interrupt::register_handler(*this);
	}
	
	void on_sonar(const SonarEvent& event)
	{
		// queue event for main loop to use it
		queue_.push_(event);
	}

	//TODO not usable yet
	void on_timeout()
	{
		sonar_.set_ready();
	}
	
private:
	SONAR& sonar_;
	QUEUE& queue_;
};

// Register ISR callbacks
//TODO handle timeout!
// REGISTER_TIMER_COMPARE_ISR_METHOD(TIMER_NUM, SonarListener, &SonarListener::on_timeout)
REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK,
	SonarListener, &SonarListener::on_sonar)

// Queue buffer for SonarEvent triggered by MultiHCSR04: 8 is needed to handle 4 sonars 
// (max 2 events/sonar until restart trigger)
static constexpr const uint8_t QUEUE_SIZE = 8;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Define queue for SonarEvent generated by MultiHCSR04
	SonarEvent event_buffer[QUEUE_SIZE];
	QUEUE queue{event_buffer};
	
	// Setup LED outputs
	gpio::FastMaskedPort<LED_PORT, LED_MASK> leds{0xFF};

	// Start RTT & sonar
	RTT rtt;
	SONAR sonar{rtt};
	rtt.register_rtt_handler();
	rtt.begin_();

	// Setup PCI for all sonar echo pins
	interrupt::PCISignal<ECHO_PORT> signal;
	signal.set_enable_pins_(ECHO_MASK);
	signal.enable_();

	SonarListener listener{sonar, queue};

	// Enable interrupts now
	sei();
	
	// Infinite loop to trigger sonar and light LEDs when near object is detected 
	while (true)
	{
		sonar.trigger();

		time::RTTTime times[NUM_SONARS];
		while (!sonar.all_ready())
		{
			SonarEvent event;
			while (queue.pull(event))
			{
				// Calculate new status of LEDs for finished sonar echoes
				uint8_t alarms = 0;
				uint8_t ready_leds = 0;
				for (uint8_t i = 0; i < NUM_SONARS; ++i)
				{
					if (ECHO_LEDS[i].echo & event.started())
						times[i] = event.time();
					else if (ECHO_LEDS[i].echo & event.ready())
					{
						ready_leds |= ECHO_LEDS[i].led; 
						if	(event.time() - times[i] <= DISTANCE_THRESHOLD)
							alarms |= ECHO_LEDS[i].led;
					}
				}
				// Update LEDs as needed
				leds.set_PORT(alarms | (leds.get_PIN() & ~ready_leds));
			}
		}
		//The following line seems required, otherwise LEDs lighting seems unstable
		time::delay_ms(10);
	}
}
