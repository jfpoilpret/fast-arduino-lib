//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Example that checks compilation failures (static_assert) due to bad usage of
 * FastArduino library.
 * Build is only for target UNO.
 * This example shall never compile successfully but only expose a list of 
 * compile-time errors.
 */

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/pci.h>
#include <fastarduino/soft_uart.h>
#include <fastarduino/uart.h>

#include <fastarduino/realtime_timer.h>
#include <fastarduino/timer.h>
#include <fastarduino/pulse_timer.h>
#include <fastarduino/watchdog.h>
#include <fastarduino/eeprom.h>

#include <fastarduino/devices/sonar.h>

#ifndef ARDUINO_UNO
#error "Current target is not yet supported!"
#endif

using namespace board;

constexpr Timer NTIMER0 = Timer::TIMER0;
constexpr Timer NTIMER1 = Timer::TIMER1;
constexpr Timer NTIMER3 = (Timer) 3;

void callback() {}
void callback8(uint8_t) {}
void callback16(uint16_t) {}
void callback32(uint32_t) {}
void sonar_callback(const devices::sonar::SonarEvent<NTIMER0>&) {}
void sonar_callback(const devices::sonar::SonarEvent<NTIMER1>&) {}
struct Callback
{
	private:
	void callback() {}
	void callback8(uint8_t) {}
	void callback16(uint16_t) {}
	void callback32(uint32_t) {}
	void sonar_callback(const devices::sonar::SonarEvent<NTIMER0>&) {}
	void sonar_callback(const devices::sonar::SonarEvent<NTIMER1>&) {}

	DECL_INT_ISR_HANDLERS_FRIEND
	DECL_PCI_ISR_HANDLERS_FRIEND
	DECL_RTT_ISR_HANDLERS_FRIEND
	DECL_SONAR_ISR_HANDLERS_FRIEND
	DECL_TIMER_ISR_HANDLERS_FRIEND
	DECL_EEPROM_ISR_HANDLERS_FRIEND
	DECL_WATCHDOG_ISR_HANDLERS_FRIEND
};

// // Check PulseTimer is limited to uint8_t and uint16_t
// #pragma message "CHECK: timer::PulseTimer with bad type"
// using BAD_PULSE_TIMER = timer::PulseTimer<NTIMER0, timer::Timer<NTIMER0>::PRESCALER::NO_PRESCALING, char>;
// BAD_PULSE_TIMER stuff{2000};

// // Check all PTMF callback macros and friends declaration macros
// #pragma message "CHECK: private callback as friends (11 checks, should generate NO error)"
// REGISTER_HCSR04_INT_ISR_METHOD(NTIMER0, 0, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0, Callback, &Callback::callback)
// REGISTER_HCSR04_PCI_ISR_METHOD(NTIMER0, 0, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, Callback, &Callback::callback)
// REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER0, 1, DigitalPin::D0_PD0, Port::PORT_C, 0x3F, Callback, &Callback::sonar_callback)
// REGISTER_INT_ISR_METHOD(1, ExternalInterruptPin::D3_PD3_EXT1, Callback, &Callback::callback)
// REGISTER_PCI_ISR_METHOD(2, Callback, &Callback::callback, InterruptPin::D0_PD0_PCI2)
// REGISTER_RTT_ISR_METHOD(0, Callback, &Callback::callback32)
// REGISTER_TIMER_CAPTURE_ISR_METHOD(1, Callback, &Callback::callback16)
// REGISTER_TIMER_COMPARE_ISR_METHOD(1, Callback, &Callback::callback)
// REGISTER_TIMER_OVERFLOW_ISR_METHOD(1, Callback, &Callback::callback)
// REGISTER_WATCHDOG_ISR_METHOD(Callback, &Callback::callback)
// REGISTER_EEPROM_ISR_METHOD(Callback, &Callback::callback)

// // Try to register Sonar with bad args
// #pragma message "CHECK: register sonar with not existing TIMER (9 checks)"
// REGISTER_HCSR04_INT_ISR(NTIMER3, 0, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0)
// REGISTER_HCSR04_INT_ISR_FUNCTION(NTIMER3, 0, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0, callback)
// REGISTER_HCSR04_INT_ISR_METHOD(NTIMER3, 0, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0, Callback, &Callback::callback)
// REGISTER_HCSR04_PCI_ISR(NTIMER3, 0, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0)
// REGISTER_HCSR04_PCI_ISR_FUNCTION(NTIMER3, 0, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, callback)
// REGISTER_HCSR04_PCI_ISR_METHOD(NTIMER3, 0, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, Callback, &Callback::callback)
// REGISTER_DISTINCT_HCSR04_PCI_ISR(NTIMER3, 0, SONAR_PINS(DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0), SONAR_PINS(DigitalPin::D1_PD1, InterruptPin::D9_PB1_PCI0))
// REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(NTIMER3, 0, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, sonar_callback)
// REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER3, 0, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, Callback, &Callback::sonar_callback)

// #pragma message "CHECK: register sonar with bad pin(s) (10 checks)"
// REGISTER_HCSR04_INT_ISR(NTIMER0, 1, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0)
// REGISTER_HCSR04_INT_ISR_FUNCTION(NTIMER0, 1, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0, callback)
// REGISTER_HCSR04_INT_ISR_METHOD(NTIMER0, 1, DigitalPin::D0_PD0, ExternalInterruptPin::D2_PD2_EXT0, Callback, &Callback::callback)
// REGISTER_HCSR04_PCI_ISR(NTIMER0, 1, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0)
// REGISTER_HCSR04_PCI_ISR(NTIMER0, 1, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, InterruptPin::D14_PC0_PCI1)
// REGISTER_HCSR04_PCI_ISR_FUNCTION(NTIMER0, 1, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, callback)
// REGISTER_HCSR04_PCI_ISR_METHOD(NTIMER0, 1, DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0, Callback, &Callback::callback)
// REGISTER_DISTINCT_HCSR04_PCI_ISR(NTIMER0, 1, SONAR_PINS(DigitalPin::D0_PD0, InterruptPin::D8_PB0_PCI0), SONAR_PINS(DigitalPin::D1_PD1, InterruptPin::D9_PB1_PCI0))
// REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(NTIMER0, 1, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, sonar_callback)
// REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER0, 1, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, Callback, &Callback::sonar_callback)

// #pragma message "CHECK: register multi sonar with bad callback (2 checks)"
// REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(NTIMER0, 0, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, callback)
// REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER0, 0, DigitalPin::D0_PD0, Port::PORT_B, 0xFF, Callback, &Callback::callback)

// #pragma message "CHECK: register multi sonar with bad mask for echo port (2 checks)"
// REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(NTIMER0, 1, DigitalPin::D0_PD0, Port::PORT_C, 0xFF, sonar_callback)
// REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(NTIMER0, 1, DigitalPin::D0_PD0, Port::PORT_C, 0xFF, Callback, &Callback::sonar_callback)

// // Try to register INT0 vector for a non INT pin
// #pragma message "CHECK: register INT0 vector for a non INT pin (3 checks)"
// REGISTER_INT_ISR_EMPTY(0, DigitalPin::D0_PD0)
// REGISTER_INT_ISR_METHOD(0, DigitalPin::D0_PD0, Callback, &Callback::callback)
// REGISTER_INT_ISR_FUNCTION(0, DigitalPin::D0_PD0, callback)
// // Try to register INT0 vector for an INT1 pin
// #pragma message "CHECK: register INT0 vector for an INT1 pin (3 checks)"
// REGISTER_INT_ISR_EMPTY(0, ExternalInterruptPin::D3_PD3_EXT1)
// REGISTER_INT_ISR_METHOD(0, ExternalInterruptPin::D3_PD3_EXT1, Callback, &Callback::callback)
// REGISTER_INT_ISR_FUNCTION(0, ExternalInterruptPin::D3_PD3_EXT1, callback)
// // Try to register INT2 (not existing) vector for an INT1 pin
// #pragma message "CHECK: register INT2 vector for an INT1 pin (3 checks)"
// REGISTER_INT_ISR_EMPTY(2, ExternalInterruptPin::D3_PD3_EXT1)
// REGISTER_INT_ISR_METHOD(2, ExternalInterruptPin::D3_PD3_EXT1, Callback, &Callback::callback)
// REGISTER_INT_ISR_FUNCTION(2, ExternalInterruptPin::D3_PD3_EXT1, callback)

// // Try to register PCINT0 vector for a PCINT2 pin
// #pragma message "CHECK: register PCINT0 vector for a PCINT2 pin (3 checks)"
// REGISTER_PCI_ISR_EMPTY(0, InterruptPin::D0_PD0_PCI2)
// REGISTER_PCI_ISR_METHOD(0, Callback, &Callback::callback, InterruptPin::D0_PD0_PCI2)
// REGISTER_PCI_ISR_FUNCTION(0, callback, InterruptPin::D0_PD0_PCI2)
// // Try to register PCINT0 vector for several PCINT0 pins and one PCINT2 pin
// #pragma message "CHECK: register PCINT0 vector for several PCINT0 and one PCINT2 pin (3 checks)"
// REGISTER_PCI_ISR_EMPTY(0, InterruptPin::D8_PB0_PCI0, InterruptPin::D10_PB2_PCI0, InterruptPin::D0_PD0_PCI2)
// REGISTER_PCI_ISR_METHOD(0, Callback, &Callback::callback, InterruptPin::D8_PB0_PCI0, InterruptPin::D10_PB2_PCI0, InterruptPin::D0_PD0_PCI2)
// REGISTER_PCI_ISR_FUNCTION(0, callback, InterruptPin::D8_PB0_PCI0, InterruptPin::D10_PB2_PCI0, InterruptPin::D0_PD0_PCI2)

// // Try to register SW UART with bad PCINT pin
// #pragma message "CHECK: register SW UART with bad PCINT pin (1 check)"
// REGISTER_UART_PCI_ISR(InterruptPin::D0_PD0_PCI2, 0)
// // Try to register SW UART with a non INT pin
// #pragma message "CHECK: register SW UART with non INT pin (1 check)"
// REGISTER_UART_INT_ISR(DigitalPin::D0_PD0, 0)
// // Try to register SW UART with a bad INT pin
// #pragma message "CHECK: register SW UART with bad INT pin (1 check)"
// REGISTER_UART_INT_ISR(ExternalInterruptPin::D3_PD3_EXT1, 0)

// //IMPORTANT NOTE the following checks generate each a whole bunch of errors because there is no static_assert
// // but only "normal" compilation errors, due to use of non existing value for an enum, with plenty of consequent errors

// // Try to register HW UART for non existing UART NUM
// #pragma message "CHECK: register HW UATX for non existing UART NUM (1 check)"
// REGISTER_UATX_ISR(1)
// #pragma message "CHECK: register HW UARX for non existing UART NUM (1 check)"
// REGISTER_UARX_ISR(1)
// #pragma message "CHECK: register HW UART for non existing UART NUM (1 check)"
// REGISTER_UART_ISR(1)

// // Try to register TIMER vector for non existing TIMER NUM
// #pragma message "CHECK: register TIMER ISR for non existing TIMER NUM (10 checks)"
// REGISTER_TIMER_COMPARE_ISR_EMPTY(3)
// REGISTER_TIMER_COMPARE_ISR_METHOD(3, Callback, &Callback::callback)
// REGISTER_TIMER_COMPARE_ISR_FUNCTION(3, callback)
// REGISTER_TIMER_OVERFLOW_ISR_EMPTY(3)
// REGISTER_TIMER_OVERFLOW_ISR_METHOD(3, Callback, &Callback::callback)
// REGISTER_TIMER_OVERFLOW_ISR_FUNCTION(3, callback)
// REGISTER_TIMER_CAPTURE_ISR_EMPTY(3)
// REGISTER_TIMER_CAPTURE_ISR_METHOD(3, Callback, &Callback::callback8)
// REGISTER_TIMER_CAPTURE_ISR_METHOD(3, Callback, &Callback::callback16)
// REGISTER_TIMER_CAPTURE_ISR_FUNCTION(3, callback16)

// #pragma message "CHECK: register TIMER CAPTURE ISR for TIMER NUM without ICP (4 checks)"
// REGISTER_TIMER_CAPTURE_ISR_METHOD(0, Callback, &Callback::callback8)
// REGISTER_TIMER_CAPTURE_ISR_METHOD(0, Callback, &Callback::callback16)
// REGISTER_TIMER_CAPTURE_ISR_FUNCTION(0, callback8)
// REGISTER_TIMER_CAPTURE_ISR_FUNCTION(0, callback16)

// #pragma message "CHECK: register TIMER CAPTURE ISR for TIMER NUM with wrong callback argument size (2 checks)"
// REGISTER_TIMER_CAPTURE_ISR_METHOD(1, Callback, &Callback::callback8)
// REGISTER_TIMER_CAPTURE_ISR_FUNCTION(1, callback8)

// // Try to register TIMER vector for RTT for non existing TIMER NUM
// #pragma message "CHECK: register TIMER ISR for RTT for non existing TIMER NUM (3 checks)"
// REGISTER_RTT_ISR(3)
// REGISTER_RTT_ISR_METHOD(3, Callback, &Callback::callback32)
// REGISTER_RTT_ISR_FUNCTION(3, callback32)

// Try to register PulseTimer8 ISR for a 16bits timer
constexpr timer::Calculator<NTIMER0>::PRESCALER PRESCALER0 = timer::Calculator<NTIMER0>::PRESCALER::NO_PRESCALING;
constexpr timer::Calculator<NTIMER1>::PRESCALER PRESCALER1 = timer::Calculator<NTIMER1>::PRESCALER::NO_PRESCALING;
using PRESCALER_NONE = board_traits::TimerPrescalers_trait<
	board_traits::TimerPrescalers::PRESCALERS_NONE>::TimerPrescaler;
constexpr PRESCALER_NONE PRESCALER3 = PRESCALER_NONE(0);

// #pragma message "CHECK: register PulseTimer8 ISR for a 16 bits TIMER (3 checks)"
// REGISTER_PULSE_TIMER8_AB_ISR(1, PRESCALER1, PWMPin::D9_PB1_OC1A, PWMPin::D10_PB2_OC1B)
// REGISTER_PULSE_TIMER8_A_ISR(1, PRESCALER1, PWMPin::D9_PB1_OC1A)
// REGISTER_PULSE_TIMER8_B_ISR(1, PRESCALER1, PWMPin::D10_PB2_OC1B)

// #pragma message "CHECK: register PulseTimer8 ISR for a non existing TIMER(3 checks)"
// REGISTER_PULSE_TIMER8_AB_ISR(3, PRESCALER3, PWMPin::D9_PB1_OC1A, PWMPin::D10_PB2_OC1B)
// REGISTER_PULSE_TIMER8_A_ISR(3, PRESCALER3, PWMPin::D9_PB1_OC1A)
// REGISTER_PULSE_TIMER8_B_ISR(3, PRESCALER3, PWMPin::D10_PB2_OC1B)

// #pragma message "CHECK: register PulseTimer8 ISR for bad pins (6 checks)"
// REGISTER_PULSE_TIMER8_AB_ISR(0, PRESCALER0, PWMPin::D9_PB1_OC1A, PWMPin::D10_PB2_OC1B)
// REGISTER_PULSE_TIMER8_A_ISR(0, PRESCALER0, PWMPin::D9_PB1_OC1A)
// REGISTER_PULSE_TIMER8_B_ISR(0, PRESCALER0, PWMPin::D10_PB2_OC1B)

// REGISTER_PULSE_TIMER8_AB_ISR(0, PRESCALER0, PWMPin::D5_PD5_OC0B, PWMPin::D6_PD6_OC0A)
// REGISTER_PULSE_TIMER8_A_ISR(0, PRESCALER0, PWMPin::D5_PD5_OC0B)
// REGISTER_PULSE_TIMER8_B_ISR(0, PRESCALER0, PWMPin::D6_PD6_OC0A)

int main()
{
	return 0;
}
