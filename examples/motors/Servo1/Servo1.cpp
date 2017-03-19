/*
 * Use potentiometer to set servo arm angle through Servo API.
 * This example uses an 8-bit timer.
 * The servo I use in this example is a TowerPro SG90.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A1: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D6: connected to servo signal pin (orange wire)
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/pulse_timer.h>
#include <fastarduino/devices/servo.h>

constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TCALC = timer::Calculator<TIMER>;
using TPRESCALER = TCALC::TIMER_PRESCALER;

// Constants for servo and prescaler to be used for TIMER
//constexpr const uint16_t MAX_PULSE_US = 2400;
//constexpr const uint16_t MIN_PULSE_US = 900;
constexpr const uint16_t MAX_PULSE_US = 2400;
constexpr const uint16_t MIN_PULSE_US = 544;
constexpr const uint16_t NEUTRAL_PULSE_US = 1500;
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const TPRESCALER PRESCALER = TCALC::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);

// PIN connected to servo signal
constexpr const board::DigitalPin SERVO_PIN1 = board::PWMPin::D6_PD6_OC0A;

// Predefine types used for Timer and Servo
using PULSE_TIMER = timer::PulseTimer<TIMER, PRESCALER>;
using SERVO1 = devices::servo::Servo<PULSE_TIMER, SERVO_PIN1>;

constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;
using ANALOG1_INPUT = analog::AnalogInput<POT1, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;

// Register ISR needed for PulseTimer (8 bits specific)
//REGISTER_PULSE_TIMER8_AB_ISR(0, PRESCALER, SERVO_PIN1, SERVO_PIN2)
REGISTER_PULSE_TIMER8_A_ISR(0, PRESCALER, SERVO_PIN1)

int main() __attribute__((OS_main));
int main()
{
	// Instantiate pulse timer for servo
	PULSE_TIMER servo_timer{PULSE_FREQUENCY};
	// Instantiate servo
	SERVO1 servo1{servo_timer, MIN_PULSE_US, MAX_PULSE_US, NEUTRAL_PULSE_US};
	// Start pulse timer
	servo_timer._begin();
	// Enable interrupts
	sei();

	ANALOG1_INPUT pot1;
	
//	servo1.detach();
	while (true)
	{
		uint16_t input1 = pot1.sample();
		// 3 API methods are available to set the Servo signal
		// 1. Direct timer counter value (0..255 on 8-bits timer, constrained to servo range)
//		servo1.set_counter(input1);
		// 2. Pulse duration in us (MIN_PULSE_US..MAX_PULSE_US)
//		servo1.set_pulse(MIN_PULSE_US + input1 * 4);
		// 3. Angle in degrees (-90..+90)
		servo1.rotate(int16_t(input1) - 128);
		
		time::delay_ms(100);
	}
}
