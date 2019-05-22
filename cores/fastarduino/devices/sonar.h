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

/// @cond api

/**
 * @file
 * API to handle ultrasonic distance rangers (aka "sonar") such as the HC-SR04.
 */
#ifndef SONAR_H
#define SONAR_H

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::ExternalInterruptPin`.
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param INT_NUM the number of the `INT` vector for the `board::ExternalInterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * 
 * @sa devices::sonar::HCSR04
 */
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)                   \
	ISR(CAT3(INT, INT_NUM, _vect))                                               \
	{                                                                            \
		devices::sonar::isr_handler::sonar_int<INT_NUM, TIMER, TRIGGER, ECHO>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a set of several
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::InterruptPin`.
 * This macro supports registration of an ISR for several `HCSR04`, sharing one 
 * single trigger pin, and having all echo pins on the same port.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * @param ... other echo pins for other `HCSR04`
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_DISTINCT_HCSR04_PCI_ISR()
 */
#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)                             \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                            \
	{                                                                                           \
		devices::sonar::isr_handler::sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHO, ##__VA_ARGS__>(); \
	}

//TODO DOCS
#define SONAR_PINS(TRIGGER, ECHO) devices::sonar::isr_handler::TriggerEcho<TRIGGER, ECHO>

/**
 * Register the necessary ISR (Interrupt Service Routine) for a set of several
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::InterruptPin`.
 * This macro supports registration of an ISR for several `HCSR04`, using 
 * distinct trigger pins, but having all echo pins on the same port.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param SONAR_PINS the pair of pins connected to the 1st sonar; this is a 
 * specific type, created by `SONAR_PINS()` macro.
 * @param ... other pairs of (trigger pin, echo pin) for other `HCSR04`
 * 
 * @sa devices::sonar::HCSR04
 * @sa SONAR_PINS()
 * @sa REGISTER_HCSR04_PCI_ISR()
 */
#define REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, SONAR_PINS1, ...)                             \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                   \
	{                                                                                                  \
		devices::sonar::isr_handler::sonar_distinct_pci<PCI_NUM, TIMER, SONAR_PINS1, ##__VA_ARGS__>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::ExternalInterruptPin`, and call back a handler's method if the sonar
 * has finished receiving the echo pulse.
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param INT_NUM the number of the `INT` vector for the `board::ExternalInterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the sonar
 * has received the echo pulse; this must be a proper PTMF (pointer to member 
 * function).
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_HCSR04_INT_ISR()
 * @sa REGISTER_HCSR04_INT_ISR_FUNCTION()
 */
#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)                   \
	ISR(CAT3(INT, INT_NUM, _vect))                                                                         \
	{                                                                                                      \
		devices::sonar::isr_handler::sonar_int_method<INT_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::ExternalInterruptPin`, along with a callback function that will be 
 * notified when the sonar has finished receiving the echo pulse.
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param INT_NUM the number of the `INT` vector for the `board::ExternalInterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * @param CALLBACK the function that will be called when the sonar has received
 * the echo pulse
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_HCSR04_INT_ISR()
 * @sa REGISTER_HCSR04_INT_ISR_METHOD()
 */
#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)                   \
	ISR(CAT3(INT, INT_NUM, _vect))                                                                  \
	{                                                                                               \
		devices::sonar::isr_handler::sonar_int_function<INT_NUM, TIMER, TRIGGER, ECHO, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::InterruptPin`, and call back a handler's method if the sonar
 * has finished receiving the echo pulse.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the sonar
 * has received the echo pulse; this must be a proper PTMF (pointer to member 
 * function).
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_DISTINCT_HCSR04_PCI_ISR()
 * @sa REGISTER_HCSR04_PCI_ISR()
 * @sa REGISTER_HCSR04_PCI_ISR_FUNCTION()
 */
#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)                   \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                       \
	{                                                                                                      \
		devices::sonar::isr_handler::sonar_pci_method<PCI_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::HCSR04` to listen to echo pulses when the echo pin is a
 * `board::InterruptPin`, along with a callback function that will be 
 * notified when the sonar has finished receiving the echo pulse.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the `devices::sonar::HCSR04`
 * template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the sonar echo pin
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the function that will be called when the sonar has received
 * the echo pulse
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_HCSR04_PCI_ISR()
 * @sa REGISTER_HCSR04_PCI_ISR_METHOD()
 */
#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)                   \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                \
	{                                                                                               \
		devices::sonar::isr_handler::sonar_pci_function<PCI_NUM, TIMER, TRIGGER, ECHO, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a set of
 * `devices::sonar::HCSR04` to be notified when a timeout occurs; this ISR is 
 * also in charge of the associated `timer::RTT` time update.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param SONAR the actual type of the first sonar to notify (instantiated
 * template of `devices::sonar::HCSR04`)
 * @param ... the actual types of other sonars to notify
 * 
 * @sa devices::sonar::HCSR04
 * @sa HCSR04::echo_us()
 * @sa HCSR04::await_echo_us()
 * @sa HCSR04::async_echo()
 * @sa REGISTER_RTT_ISR()
 */
#define REGISTER_HCSR04_RTT_TIMEOUT(TIMER_NUM, SONAR, ...)                                \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                              \
	{                                                                                     \
		devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a set of
 * `devices::sonar::HCSR04` to be notified, and call back a handler's method,
 * when a timeout occurs; this ISR is also in charge of the associated 
 * `timer::RTT` time update.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the function that will be called when the timeout has occurred
 * @param SONAR the actual type of the first sonar to notify (instantiated
 * template of `devices::sonar::HCSR04`)
 * @param ... the actual types of other sonars to notify
 * 
 * @sa devices::sonar::HCSR04
 * @sa HCSR04::echo_us()
 * @sa HCSR04::await_echo_us()
 * @sa HCSR04::async_echo()
 * @sa REGISTER_HCSR04_RTT_TIMEOUT()
 */
#define REGISTER_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, HANDLER, CALLBACK, SONAR, ...)          \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                  \
	{                                                                                         \
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>()) \
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();                \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a set of
 * `devices::sonar::HCSR04` to be notified, and call back a function,
 * when a timeout occurs; this ISR is also in charge of the associated 
 * `timer::RTT` time update.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the timeout has occurred
 * @param SONAR the actual type of the first sonar to notify (instantiated
 * template of `devices::sonar::HCSR04`)
 * @param ... the actual types of other sonars to notify
 * 
 * @sa devices::sonar::HCSR04
 * @sa HCSR04::echo_us()
 * @sa HCSR04::await_echo_us()
 * @sa HCSR04::async_echo()
 * @sa REGISTER_HCSR04_RTT_TIMEOUT()
 */
#define REGISTER_HCSR04_RTT_TIMEOUT_FUNCTION(TIMER_NUM, CALLBACK, SONAR, ...)                 \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                  \
	{                                                                                         \
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>()) \
			interrupt::CallbackHandler<void (*)(), CALLBACK>::call();                         \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::MultiHCSR04` to listen to echo pulses on all sonars connected
 * to it, and call back a handler's method.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the 
 * `devices::sonar::MultiHCSR04` template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonars trigger pins
 * @param ECHO_PORT the `board::Port` connected to all sonar echo pins
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when one sonar
 * echo pin changes level, i.e. when a leading or trailing edge of the echo pulse
 * is received; this must be a proper PTMF (pointer to member function) which
 * takes one argument of type `const SonarEvent<TIMER>&`.
 * 
 * @sa devices::sonar::MultiHCSR04
 * @sa devices::sonar::SonarEvent
 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION()
 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD()
 */
#define REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK) \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                           \
	{                                                                                                          \
		using isr = devices::sonar::isr_handler;                                                               \
		isr::multi_sonar_pci_method<PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK>();       \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * `devices::sonar::MultiHCSR04` to listen to echo pulses on all sonars connected
 * to it, and call back a function.
 * 
 * @param TIMER the `board::Timer` type used to instantiate the 
 * `devices::sonar::MultiHCSR04` template class.
 * @param PCI_NUM the number of the `PCINT` vector for the `board::InterruptPin`
 * connected to the echo pin
 * @param TRIGGER the `board::DigitalPin` connected to the sonars trigger pins
 * @param ECHO_PORT the `board::Port` connected to all sonar echo pins
 * @param CALLBACK the function that will be called when one sonar echo pin 
 * changes level, i.e. when a leading or trailing edge of the echo pulse
 * is received; this must take one argument of type `const SonarEvent<TIMER>&`.
 * 
 * @sa devices::sonar::MultiHCSR04
 * @sa devices::sonar::SonarEvent
 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_METHOD()
 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION()
 */
#define REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK) \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                    \
	{                                                                                                   \
		using isr = devices::sonar::isr_handler;                                                        \
		isr::multi_sonar_pci_function<PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK>();       \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a
 * `devices::sonar::MultiHCSR04` to be notified, and call back a handler's method,
 * when a timeout occurs; this ISR is also in charge of the associated 
 * `timer::RTT` time update.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param SONAR the actual type of the `devices::sonar::MultiHCSR04` for which
 * this ISR is registered
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when a timeout
 * occurs; this must be a proper PTMF (pointer to member function) which
 * takes one argument of type `const SonarEvent<TIMER>&`.
 * 
 * @sa devices::sonar::MultiHCSR04
 * @sa devices::sonar::SonarEvent
 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION()
 */
#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, SONAR, HANDLER, CALLBACK)                            \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                                     \
	{                                                                                                            \
		using EVENT = typename SONAR::EVENT;                                                                     \
		EVENT event = devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR, EVENT>();            \
		if (event.timeout()) interrupt::CallbackHandler<void (HANDLER::*)(const EVENT&), CALLBACK>::call(event); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a
 * `devices::sonar::MultiHCSR04` to be notified, and call back a function,
 * when a timeout occurs; this ISR is also in charge of the associated 
 * `timer::RTT` time update.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param SONAR the actual type of the `devices::sonar::MultiHCSR04` for which
 * this ISR is registered
 * @param CALLBACK the function that will be called when a timeout
 * occurs; this must take one argument of type `const SonarEvent<TIMER>&`.
 * 
 * @sa devices::sonar::MultiHCSR04
 * @sa devices::sonar::SonarEvent
 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD()
 */
#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION(TIMER_NUM, SONAR, CALLBACK)                          \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                            \
	{                                                                                                   \
		using EVENT = typename SONAR::EVENT;                                                            \
		EVENT event = devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR, EVENT>();   \
		if (event.timeout()) interrupt::CallbackHandler<void (*)(const EVENT&), CALLBACK>::call(event); \
	}

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by one (or more) of:
 * - `REGISTER_HCSR04_INT_ISR_METHOD`
 * - `REGISTER_HCSR04_PCI_ISR_METHOD`
 * - `REGISTER_HCSR04_RTT_TIMEOUT_METHOD`
 * - `REGISTER_MULTI_HCSR04_PCI_ISR_METHOD`
 * - `REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD`
 * 
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_SONAR_ISR_HANDLERS_FRIEND         \
	friend struct devices::sonar::isr_handler; \
	DECL_INT_ISR_FRIENDS                       \
	DECL_PCINT_ISR_FRIENDS                     \
	DECL_TIMER_COMP_FRIENDS

/**
 * Defines the API for sonar support.
 * Supported ultrasonic sensors have 2 pins:
 * - one "trigger" pin that, upon a short pulse, will generate ultrasonic waves
 * to be emitted by the sensor
 * - one "echo" pin that, upon reception of the echoed ultrasonic wave, will
 * generate a pulse which duration is the time during which the ultrasonic wave
 * has travelled from the sensor back to the sensor, after reflecting on some
 * obstacle.
 * This API has been tested on HC-SR04 sensors (cheap ultrasonic sensors with
 * a range of 4 meters).
 */
namespace devices::sonar
{
	/**
	 * The approximate speed of sound (and ultrasonic) waves, in the air,
	 * expressed in meters per second.
	 * This constant is useful everytime we need to convert echo durations from
	 * the ultrasonic sensor to a concrete distance.
	 */
	static constexpr const uint32_t SPEED_OF_SOUND = 340UL;

	/**
	 * This method converts the echo duration, in microseconds, to the distance
	 * between the sensor and the reflecting obstacle, in millimeters.
	 * This method is `constexpr` hence it can be evaluated at compile-time (for
	 * more code size and speed efficiency) when provided a constant argument.
	 * 
	 * Note that the calculation accounts for the fact that @p echo_us is the time
	 * for a complete roundtrip of the ultrasonic wave, i.e. the time needed for
	 * the wave to cover twice the distance between the sensor and the reflecting
	 * obstacle.
	 * 
	 * @param echo_us the echo pulse duration, in microseconds
	 * @return the distance, in millimeters, between the sensor and the
	 * obstacle
	 */
	static constexpr uint16_t echo_us_to_distance_mm(uint16_t echo_us)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Divide by 2 as echo time includes full sound round-trip
		return uint16_t(echo_us * SPEED_OF_SOUND / 1000UL / 2UL);
	}

	/**
	 * This method converts the disatnce, in millimeters, between the sensor and
	 * a reflecting object, into the expected echo duration, in microseconds.
	 * This method is `constexpr` hence it can be evaluated at compile-time (for
	 * more code size and speed efficiency) when provided a constant argument.
	 * It can thus be used to calculate constant echo durations based on "threshold"
	 * distances that your program may need to specifically address.
	 * 
	 * Note that the calculation accounts for the fact that the echo duration is
	 * the time for a complete roundtrip of the ultrasonic wave, i.e. the time
	 * needed for the wave to cover twice the distance between the sensor and
	 * the reflecting obstacle.
	 * 
	 * @param distance_mm the distance, in millimeters, between the sensor and the
	 * obstacle
	 * @return the echo pulse duration, in microseconds, expected for @p distance_mm
	 */
	static constexpr uint16_t distance_mm_to_echo_us(uint16_t distance_mm)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Multiply by 2 as echo time must include full sound round-trip
		return uint16_t(distance_mm * 1000UL * 2UL / SPEED_OF_SOUND);
	}

	/**
	 * This enum defines the different modes, supported by `HCSR04`, to
	 * calculate the echo pin pulse duration.
	 * @sa HCSR04
	 * @sa HCSR04::echo_us()
	 */
	enum class SonarType : uint8_t
	{
		/** In this mode, the `HCSR04` will block until the echo pulse is received. */
		BLOCKING,
		/**
		 * In this mode, the echo pin is a `board::ExternalInterruptPin` and the
		 * HCSR04 will use interrupts to calculate the echo pulse duration.
		 * When this mode is used, one registration macro must be called among
		 * `REGISTER_HCSR04_INT_ISR*`.
		 */
		ASYNC_INT,
		/**
		 * In this mode, the echo pin is a `board::InterruptPin` and the
		 * HCSR04 will use interrupts to calculate the echo pulse duration.
		 * When this mode is used, one registration macro must be called among
		 * `REGISTER_HCSR04_PCI_ISR*`.
		 */
		ASYNC_PCINT
	};

	/**
	 * Am abstract base class for some sonar classes defined as part of this API.
	 * You should not need to subclass `AbstractSonar` yourself in general.
	 * @tparam NTIMER_ the AVR timer of the `timer::RTT` to use for this sonar
	 * 
	 * @sa board::Timer
	 * @sa timer::RTT
	 * @sa HCSR04
	 */
	template<board::Timer NTIMER_> class AbstractSonar
	{
	public:
		/** The type of `timer::RTT` used by this sonar instance. */
		using RTT = timer::RTT<NTIMER_>;

		/**
		 * Indicate if an echo pulse measure is ready to read.
		 * This can be useful when using asynchronous modes, and checking from 
		 * time to time if, after a trigger pulse, an echo pulse has already been 
		 * received or not yet.
		 * @retval true an echo pulse duration is available
		 * @retval false an echo pulse duration is not yet available
		 * @sa latest_echo_us()
		 */
		inline bool ready() const
		{
			return status_ == READY;
		}

		/**
		 * Get the latest measured echo pulse duration.
		 * If a trigger pulse was sent but no echo received yet, then the method
		 * immediataly returns `0`.
		 * It also returns `0`, as a convention, if a timeout occurred, i.e. no 
		 * echo pulse was received in expected time.
		 * @return the latest measured echo pulse duration in microseconds
		 * @sa HCSR04::echo_us()
		 * @sa HCSR04::await_echo_us()
		 */
		inline uint16_t latest_echo_us() const
		{
			synchronized
			{
				if (status_ == READY)
					return echo_time_();
				else
					return 0;
			}
		}

	private:
		using RAW_TIME = typename RTT::RAW_TIME;

	protected:
		/// @cond notdocumented
		AbstractSonar(const RTT& rtt)
			: rtt_{rtt}, status_{UNKNOWN}, timeout_time_ms_{},
			  echo_start_{RAW_TIME::EMPTY_TIME}, echo_end_{RAW_TIME::EMPTY_TIME}
		{}

		uint16_t async_echo_us(uint16_t timeout_ms)
		{
			uint32_t now = rtt_.millis();
			now += timeout_ms;
			// Wait for echo signal start
			while (status_ != READY)
				if (rtt_.millis() >= now)
				{
					synchronized
					{
						status_ = READY;
						echo_start_ = echo_end_ = RAW_TIME::EMPTY_TIME;
					}
					return 0;
				}
			return echo_time_();
		}

		template<board::DigitalPin ECHO>
		uint16_t blocking_echo_us(typename gpio::FastPinType<ECHO>::TYPE& echo, uint16_t timeout_ms)
		{
			uint32_t now = rtt_.millis();
			now += timeout_ms;
			while (!echo.value())
				if (rtt_.millis() >= now) return 0;
			synchronized
			{
				status_ = ECHO_STARTED;
				echo_start_ = rtt_.raw_time();
			}
			// Wait for echo signal end
			while (echo.value())
				if (rtt_.millis() >= now) return 0;
			synchronized
			{
				status_ = READY;
				echo_end_ = rtt_.raw_time();
				return echo_time_();
			}
		}

		inline void trigger_sent(uint16_t timeout_ms)
		{
			synchronized
			{
				status_ = TRIGGERED;
				timeout_time_ms_ = rtt_.millis_() + timeout_ms;
			}
		}

		inline bool pulse_edge(bool rising)
		{
			if (rising && status_ == TRIGGERED)
			{
				status_ = ECHO_STARTED;
				echo_start_ = rtt_.raw_time_();
			}
			else if ((!rising) && status_ == ECHO_STARTED)
			{
				status_ = READY;
				echo_end_ = rtt_.raw_time_();
				return true;
			}
			return false;
		}

		inline bool rtt_time_changed()
		{
			if (status_ != READY && rtt_.millis_() >= timeout_time_ms_)
			{
				status_ = READY;
				echo_start_ = echo_end_ = RAW_TIME::EMPTY_TIME;
				return true;
			}
			return false;
		}
		/// @endcond

	private:
		uint16_t echo_time_() const
		{
			return uint16_t((echo_end_.as_real_time() - echo_start_.as_real_time()).total_micros());
		}

		const RTT& rtt_;

		static constexpr const uint8_t UNKNOWN = 0x00;
		static constexpr const uint8_t TRIGGERED = 0x10;
		static constexpr const uint8_t ECHO_STARTED = 0x11;
		static constexpr const uint8_t READY = 0x20;

		volatile uint8_t status_;
		uint32_t timeout_time_ms_;

		RAW_TIME echo_start_;
		RAW_TIME echo_end_;
	};

	/**
	 * This template class supports one HC-SR04 sonar (or equivalent sensor), 
	 * connected to the MCU via 2 pins.
	 * 
	 * @tparam NTIMER_ the AVR timer of the `timer::RTT` to use for this sonar
	 * @tparam TRIGGER_ the `board::DigitalPin` connected to the sensor trigger 
	 * pin; that can be any available pin.
	 * @tparam ECHO_ the `board::DigitalPin` connected to the sensor echo pin;
	 * based on @p SONAR_TYPE_ value, this may be any available pin.
	 * (`SONAR_TYPE_ == SonarType::BLOCKING`), only a `board::InterruptPin`
	 * (`SONAR_TYPE_ == SonarType::ASYNC_PCINT`), or only a `board::ExternalInterruptPin`
	 * (`SONAR_TYPE_ == SonarType::ASYNC_INT`).
	 * @tparam SONAR_TYPE_ the mode used by this class to calculate the echo pin 
	 * pulse duration. This parameter has an impact on the asynchronicity of some 
	 * methods.
	 * 
	 * @sa timer::RTT
	 * @sa SonarType
	 */
	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
			 SonarType SONAR_TYPE_ = SonarType::BLOCKING>
	class HCSR04 : public AbstractSonar<NTIMER_>
	{
	public:
		/** The type of `timer::RTT` used by this sonar instance. */
		using RTT = timer::RTT<NTIMER_>;
		/** The `board::DigitalPin` connected to the sensor trigger pin. */
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		/** The `board::DigitalPin` connected to the sensor echo pin. */
		static constexpr const board::DigitalPin ECHO = ECHO_;
		/** The mode used by this class to calculate the echo pin pulse duration. */
		static constexpr const SonarType SONAR_TYPE = SONAR_TYPE_;

	private:
		using PARENT = AbstractSonar<NTIMER_>;
		using ECHO_PIN_TRAIT = board_traits::DigitalPin_trait<ECHO>;
		using ECHO_PORT_TRAIT = board_traits::Port_trait<ECHO_PIN_TRAIT::PORT>;
		static_assert(SONAR_TYPE != SonarType::ASYNC_INT || ECHO_PIN_TRAIT::IS_INT,
					  "SONAR_TYPE == ASYNC_INT but ECHO is not an INT pin");
		static_assert(SONAR_TYPE != SonarType::ASYNC_PCINT || ECHO_PORT_TRAIT::PCINT != 0xFF,
					  "SONAR_TYPE == ASYNC_PCINT but ECHO is not an PCI pin");

	public:
		/**
		 * The approximate maximum range, in meters, that this sonar sensor
		 * supports. Any obstacle beyond this distance will generate no echo 
		 * pulse from the sensor.
		 */
		static constexpr const uint16_t MAX_RANGE_M = 4;

		/**
		 * The default timeout duration, in milliseconds, to use if you want to
		 * cover the maximum range of the sensor.
		 * Using any greater timeout value would be pointless.
		 * @sa MAX_RANGE_M
		 * @sa echo_us()
		 * @sa await_echo_us()
		 * @sa async_echo()
		 */
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		/**
		 * Construct a new a sonar sensor handler.
		 * @param rtt a reference to an existing `timer::RTT` for echo pulse 
		 * duration counting; this RTT shall be started before using any other
		 * methods of this sonar.
		 */
		HCSR04(const RTT& rtt) : PARENT{rtt}, trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}
		{
			if (SONAR_TYPE != SonarType::BLOCKING)
				interrupt::register_handler(*this);
		}

		/**
		 * Send a trigger pulse on this sonar and wait until an echo pulse is
		 * received, or @p timeout_ms has elapsed.
		 * This method is blocking, whatever the value of `SONAR_TYPE` for this 
		 * sonar. If you want to start a sonar ranging asynchronously, then you
		 * should use `async_echo` instead.
		 * 
		 * @param timeout_ms the timeout, in milliseconds, after which the method
		 * will return if no echo pulse has been received
		 * @return the echo pulse duration in microseconds
		 * @retval 0 if no echo pulse was received before @p timeout_ms elapsed
		 * 
		 * @sa async_echo()
		 */
		uint16_t echo_us(uint16_t timeout_ms)
		{
			async_echo(timeout_ms);
			return await_echo_us(timeout_ms);
		}

		/**
		 * Send a trigger pulse on this sonar and return immediately, without 
		 * waiting for the echo pulse.
		 * There are several ways then to get the echo pulse duration:
		 * - call `await_echo_us()` and then wait for the echo pulse to be received
		 * - call `ready()` to check if echo pulse has been received already and
		 * then call `latest_echo_us()` to get the echo pulse duration
		 * - use callbacks to be notified when the echo pulse is received, then
		 * `latest_echo_us()` can be called to obtain the pulse duration
		 * 
		 * @param timeout_ms the timeout, in milliseconds, after which the ranging
		 * will stop if no echo pulse has been received
		 * @param trigger indicate if the method should generate a trigger pulse
		 * on the `TRIGGER` pin; by default it is `true`, but you may want to
		 * use `false` if you have several HCSR04 sensors, which you want to
		 * trigger all at the same time (i.e. all their trigger pins are connected
		 * to the same MCU pin).
		 * 
		 * @sa await_echo_us()
		 * @sa ready()
		 * @sa latest_echo_us()
		 */
		void async_echo(uint16_t timeout_ms, bool trigger = true)
		{
			this->trigger_sent(timeout_ms);
			if (trigger) this->trigger();
		}

		/**
		 * Wait until an echo pulse is received, or @p timeout_ms has elapsed.
		 * You must call `async_echo()` before calling this method.
		 * This method is blocking, whatever the value of `SONAR_TYPE` for this 
		 * sonar.
		 * 
		 * @param timeout_ms the timeout, in milliseconds, after which the method
		 * will return if no echo pulse has been received
		 * @return the echo pulse duration in microseconds
		 * @retval 0 if no echo pulse was received before @p timeout_ms elapsed
		 * 
		 * @sa async_echo()
		 */
		uint16_t await_echo_us(uint16_t timeout_ms)
		{
			if (SONAR_TYPE != SonarType::BLOCKING)
				return this->async_echo_us(timeout_ms);
			else
				return this->template blocking_echo_us<ECHO>(echo_, timeout_ms);
		}

	private:
		bool on_pin_change()
		{
			static_assert(SONAR_TYPE == SonarType::ASYNC_INT || SONAR_TYPE == SonarType::ASYNC_PCINT,
						  "on_pin_change() must be called only with SonarType::ASYNC_INT or ASYNC_PCINT");
			return this->pulse_edge(echo_.value());
		}

		bool on_rtt_change()
		{
			return this->rtt_time_changed();
		}

		inline void trigger()
		{
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		typename gpio::FastPinType<ECHO>::TYPE echo_;

		// Make friends with all ISR handlers
		friend struct isr_handler;
	};

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_>
	using BLOCKING_HCSR04 = HCSR04<NTIMER_, TRIGGER_, ECHO_, SonarType::BLOCKING>;

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::ExternalInterruptPin ECHO_>
	using ASYNC_INT_HCSR04 = HCSR04<NTIMER_, TRIGGER_, board::EXT_PIN<ECHO_>(), SonarType::ASYNC_INT>;

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::InterruptPin ECHO_>
	using ASYNC_PCINT_HCSR04 = HCSR04<NTIMER_, TRIGGER_, board::PCI_PIN<ECHO_>(), SonarType::ASYNC_PCINT>;

	/**
	 * This type holds information about events occurring within `MultiHCSR04`
	 * handler.
	 * One event can contain information for up to 8 sonars.
	 * 
	 * You need to register callbacks to `MultiHCSR04` in order to receive these
	 * events and process them. These events exist because `MultiHCSR04` does
	 * not process them by itself i.e. it does not calculate or hold pulse
	 * information about all connected sonar sensors. It is the responsibility
	 * of callbacks to manage this information, based on all received `SonarEvent`s.
	 * 
	 * @tparam NTIMER_ the AVR timer of the `timer::RTT` used by the `MultiHCSR04`
	 * producing this `SonarEvent`
	 * 
	 * @sa MultiHCSR04
	 * @sa MultiHCSR04::EVENT
	 */
	template<board::Timer NTIMER_> struct SonarEvent
	{
	public:
		/** 
		 * The type of `timer::RTT` used by the `MultiHCSR04` producing this `SonarEvent`. 
		 * @sa MultiHCSR04::RTT
		 */
		using RTT = timer::RTT<NTIMER_>;

		/**
		 * The `timer::RTTRawTime` type used by the `MultiHCSR04` producing this
		 * `SonarEvent`.
		 * @sa timer::RTTRawTime
		 */
		using RAW_TIME = typename RTT::RAW_TIME;

		/**
		 * Default constructor. This is here to allow direct declaration in your
		 * code:
		 * @code
		 * SonarEvent<NTIMER> event;
		 * ...
		 * @endcode
		 */
		SonarEvent() : timeout_{false}, started_{}, ready_{}, time_{RAW_TIME::EMPTY_TIME} {}

		/**
		 * Indicate if this event was produced by a timeout while waiting for 
		 * echo pulses. If so, no other field in this `SonarEvent` is relevant.
		 * Hence this is the first method you should call on a `SonarEvent` you
		 * need to handle.
		 */
		bool timeout() const
		{
			return timeout_;
		}

		/**
		 * Indicate if this event was produced due to an echo pulse leading edge
		 * just received by the related `MultiHCSR04`.
		 * Each bit maps to one sonar handled by the producing `MultHCSR04`;
		 * when `1`, the echo pulse just started on the matching sonar.
		 * `time()` will then provide the exact time at which the pulse egde 
		 * occurred.
		 * 
		 * @sa time()
		 */
		uint8_t started() const
		{
			return started_;
		}

		/**
		 * Indicate if this event was produced tdue to an echo pulse trailing edge
		 * just received by the related `MultiHCSR04`.
		 * Each bit maps to one sonar handled by the producing `MultHCSR04`;
		 * when `1`, the echo pulse just ended on the matching sonar.
		 * `time()` will then provide the exact time at which the pulse egde 
		 * occurred.
		 * For a given bit (sonar), the difference of `time()` between `started()`
		 * and `ready()` will determine the echo pulse duration.
		 * 
		 * @sa started()
		 * @sa time()
		 */
		uint8_t ready() const
		{
			return ready_;
		}

		/**
		 * The `timer::RTTRawTime` at which this event occurred.
		 * This is not relevant when `timeout()` is `true`.
		 * 
		 * @sa started()
		 * @sa ready()
		 */
		RAW_TIME time() const
		{
			return time_;
		}

	private:
		SonarEvent(bool timeout) : timeout_{timeout}, started_{}, ready_{}, time_{RAW_TIME::EMPTY_TIME} {}
		SonarEvent(uint8_t started, uint8_t ready, const RAW_TIME& time)
			: timeout_{}, started_{started}, ready_{ready}, time_{time}
		{}

		bool timeout_;
		uint8_t started_;
		uint8_t ready_;
		RAW_TIME time_;

		template<board::Timer, board::DigitalPin, board::Port, uint8_t> friend class MultiHCSR04;
	};

	/**
	 * This template class supports up to 8 HC-SR04 sonars (or equivalent sensors),
	 * with their trigger pins gathered and connected to only one MCU pin, and all
	 * echo pins connected to the same MCU `board::Port`.
	 * With this class, all connected sonars start ranging at the same time.
	 * This method works exclusively in asynchronous mode.
	 * 
	 * Note that, contrarily to the `HCSR04` class, this class does not handle 
	 * calculation or storeage of echo pulse duration for all connected sonars;
	 * instead, it produces `SonarEvent`s upon each sonar event occurring:
	 * - echo pulse leading edge received
	 * - echo pulse trailing edge received
	 * - timeout occurred while waiting for echo pulse reception
	 * You need to register proper callbacks in order to receive these events and
	 * process them. It is the responsibility your callbacks to calculate (and
	 * optionally store) echo pulse duration, based on all received `SonarEvent`s.
	 * 
	 * @tparam NTIMER_ the AVR timer of the `timer::RTT` to use for this `MultiHCSR04`
	 * @tparam TRIGGER_ the `board::DigitalPin` connected to the sensors trigger 
	 * pins; that can be any available pin.
	 * @tparam ECHO_PORT_ the MCU port to which all echo pins of sonars handled
	 * by this class are connected; this port must support PCINT interrupts.
	 * @tparam ECHO_MASK_ the mask determining which pins of @p ECHO_PORT_ are
	 * actually connected to a real sonar echo pin; for each bit set, the matching
	 * pin must be able to generate a PCINT interrupt when its level changes.
	 * 
	 * @sa SonarEvent
	 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_METHOD()
	 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD()
	 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION()
	 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION()
	 */
	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_>
	class MultiHCSR04
	{
	public:
		/** The `board::DigitalPin` connected to the sensors trigger pins. */
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		/** The MCU port to which all echo pins of sonars handled by this class are connected. */
		static constexpr const board::Port ECHO_PORT = ECHO_PORT_;
		/** The mask determining which pins of `ECHO_PORT` are actually connected to a real sonar echo pin. */
		static constexpr const uint8_t ECHO_MASK = ECHO_MASK_;

	private:
		using PTRAIT = board_traits::Port_trait<ECHO_PORT>;
		static_assert(PTRAIT::PCINT != 0xFF, "ECHO_PORT_ must support PCINT");
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK) == ECHO_MASK, "ECHO_MASK_ must contain available PORT pins");

	public:
		/** The type of `timer::RTT` used by this `MultiHCSR04` instance. */
		using RTT = timer::RTT<NTIMER_>;
		/** The exact `SonarEvent` type produced by this `MultiHCSR04` instance. */
		using EVENT = SonarEvent<NTIMER_>;

		/**
		 * The approximate maximum range, in meters, that this sonar sensor
		 * supports. Any obstacle beyond this distance will generate no echo 
		 * pulse from the sensor.
		 */
		static constexpr const uint16_t MAX_RANGE_M = 4;

		/**
		 * The default timeout duration, in milliseconds, to use if you want to
		 * cover the maximum range of the sensor.
		 * Using any greater timeout value would be pointless.
		 * @sa MAX_RANGE_M
		 * @sa trigger()
		 */
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		/**
		 * Construct a new a multi-sonar sensors handler.
		 * @param rtt a reference to an existing `timer::RTT` for echo pulse 
		 * duration counting; this RTT shall be started before using any other
		 * methods of this sonar.
		 */
		MultiHCSR04(RTT& rtt)
			: rtt_{rtt}, started_{}, ready_{}, active_{false},
			  timeout_time_ms_{}, trigger_{gpio::PinMode::OUTPUT}, echo_{0}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Start ranging on all sonars connected to this `MultiHCSR04`.
		 * When calling this method, a trigger pulse is sent to all connected 
		 * sonars. After this call, `SonarEvent`s will be generated and propagated
		 * to callbacks whenever any of the following occurs:
		 * - an echo pulse leading edge is detected on a sonar
		 * - an echo pulse trailing edge is detected on a sonar
		 * - timeout occurred while waiting for echo pulses
		 * 
		 * @param timeout_ms the timeout, in milliseconds, after which the ranging
		 * will stop if no echo pulse has been received
		 * 
		 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_METHOD()
		 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD()
		 * @sa REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION()
		 * @sa REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION()
		 */
		void trigger(uint16_t timeout_ms)
		{
			started_ = 0;
			ready_ = 0;
			timeout_time_ms_ = rtt_.millis() + timeout_ms;
			active_ = true;
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

		/**
		 * Tell, for which of the connected sonars, the latest ranging, started
		 * by `trigger()`, is finished, ie the echo pulse has been received.
		 * If you want to know if ranging for all connected sonars is finished,
		 * then use `all_ready()` instead.
		 * 
		 * @return a bit mask where each set bit indicates that ranging is 
		 * finished for the corresponding sonar.
		 * 
		 * @sa trigger()
		 * @sa all_ready()
		 * @sa set_ready()
		 */
		uint8_t ready() const
		{
			return ready_;
		}

		/**
		 * Tell if the latest ranging, started by `trigger()`, is finished for
		 * all connected sonars, ie the echo pulse has been received.
		 * If you want to know for which connected sonars ranging is finished,
		 * then use `ready()` instead.
		 */
		bool all_ready() const
		{
			return ready_ == ECHO_MASK;
		}

		/**
		 * Force readiness of all connected sensors, ie the end of current ranging.
		 * This can be used by callbacks e.g. to stop current ranging when a timeout
		 * has occurred.
		 */
		void set_ready()
		{
			if (active_)
			{
				active_ = false;
				ready_ = ECHO_MASK;
			}
		}

	private:
		EVENT on_pin_change()
		{
			if (!active_) return EVENT{};
			// Compute the newly started echoes
			uint8_t pins = echo_.get_PIN();
			uint8_t started = pins & ~started_;
			// Compute the newly finished echoes
			uint8_t ready = ~pins & started_ & ~ready_;
			// Update status of all echo pins
			started_ |= started;
			ready_ |= ready;
			if (ready_ == ECHO_MASK) active_ = false;
			return EVENT{started, ready, rtt_.raw_time_()};
		}

		EVENT on_rtt_change()
		{
			if (active_ && rtt_.millis_() >= timeout_time_ms_)
			{
				active_ = false;
				return EVENT{true};
			}
			return EVENT{};
		}

		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		RTT& rtt_;
		volatile uint8_t started_;
		volatile uint8_t ready_;
		volatile bool active_;
		uint32_t timeout_time_ms_;
		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		gpio::FastMaskedPort<ECHO_PORT, ECHO_MASK> echo_;

		// Make friends with all ISR handlers
		friend struct isr_handler;
	};

	/// @cond notdocumented
	// All sonar-related methods called by pre-defined ISR are defined here
	struct isr_handler
	{
		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::ExternalInterruptPin ECHO_>
		static bool sonar_int()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::ExternalInterruptPin_trait<ECHO_>::INT == INT_NUM_,
						  "ECHO INT number must match INT_NUM");
			using SONAR = ASYNC_INT_HCSR04<TIMER_, TRIGGER_, ECHO_>;
			return interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::ExternalInterruptPin ECHO_,
				 typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_int_method()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::ExternalInterruptPin ECHO_,
				 void (*CALLBACK_)()>
		static void sonar_int_function()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>()) CALLBACK_();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_> static bool sonar_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::InterruptPin ECHO1_,
				 board::InterruptPin... ECHOS_>
		static bool sonar_pci()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			static_assert(	board_traits::DigitalPin_trait<board::PCI_PIN<ECHO1_>()>::PORT ==
							board_traits::PCI_trait<PCI_NUM_>::PORT,
							"ECHO port must match PCI_NUM port");
			using SONAR = ASYNC_PCINT_HCSR04<TIMER_, TRIGGER_, ECHO1_>;
			bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			// handle other echo pins
			return result || sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHOS_...>();
		}

		template<bool DUMMY_> static bool sonar_rtt_change_helper()
		{
			return false;
		}

		template<bool DUMMY_, typename SONAR1_, typename... SONARS_> static bool sonar_rtt_change_helper()
		{
			bool result = interrupt::HandlerHolder<SONAR1_>::handler()->on_rtt_change();
			// handle other sonars
			return result || sonar_rtt_change_helper<DUMMY_, SONARS_...>();
		}

		template<uint8_t TIMER_NUM_, typename... SONARS_> static bool sonar_rtt_change()
		{
			// Update RTT time
			timer::isr_handler_rtt::rtt<TIMER_NUM_>();
			// Ask each sonar to check if timeout is elapsed
			return sonar_rtt_change_helper<false, SONARS_...>();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::InterruptPin ECHO_,
				 typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_pci_method()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::InterruptPin ECHO_,
				 void (*CALLBACK_)()>
		static void sonar_pci_function()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>()) CALLBACK_();
		}

		template<board::DigitalPin TRIGGER_, board::InterruptPin ECHO_> struct TriggerEcho
		{
			static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
			static constexpr const board::InterruptPin ECHO = ECHO_;
		};

		template<uint8_t PCI_NUM_, board::Timer TIMER_> static bool sonar_distinct_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::InterruptPin ECHO_>
		static bool sonar_distinct_pci_one()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			constexpr const board::DigitalPin ECHO_PIN = board::PCI_PIN<ECHO_>();
			static_assert(board_traits::DigitalPin_trait<ECHO_PIN>::PORT == board_traits::PCI_trait<PCI_NUM_>::PORT,
						  "ECHO port must match PCI_NUM port");
			using SONAR = ASYNC_PCINT_HCSR04<TIMER_, TRIGGER_, ECHO_>;
			return interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, typename TRIGGER_ECHO1_, typename ...TRIGGER_ECHOS_>
		static bool sonar_distinct_pci()
		{
			bool result = sonar_distinct_pci_one<PCI_NUM_, TIMER_, TRIGGER_ECHO1_::TRIGGER, TRIGGER_ECHO1_::ECHO>();
			// handle other echo pins
			return result || sonar_distinct_pci<PCI_NUM_, TIMER_, TRIGGER_ECHOS_...>();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_,
				 uint8_t ECHO_MASK_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(const SonarEvent<TIMER_>&)>
		static void multi_sonar_pci_method()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT == ECHO_PORT_, "ECHO_PORT must match PCI_NUM");
			using PTRAIT = board_traits::Port_trait<ECHO_PORT_>;
			static_assert((PTRAIT::DPIN_MASK & ECHO_MASK_) == ECHO_MASK_, "ECHO_MASK must contain available PORT pins");
			using SONAR = MultiHCSR04<TIMER_, TRIGGER_, ECHO_PORT_, ECHO_MASK_>;
			SonarEvent<TIMER_> event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			if (event.ready() || event.started())
				interrupt::CallbackHandler<void (HANDLER_::*)(const SonarEvent<TIMER_>&), CALLBACK_>::call(event);
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_,
				 uint8_t ECHO_MASK_, void (*CALLBACK_)(const SonarEvent<TIMER_>&)>
		static void multi_sonar_pci_function()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT == ECHO_PORT_, "ECHO_PORT must match PCI_NUM");
			using PTRAIT = board_traits::Port_trait<ECHO_PORT_>;
			static_assert((PTRAIT::DPIN_MASK & ECHO_MASK_) == ECHO_MASK_, "ECHO_MASK must contain available PORT pins");
			using SONAR = MultiHCSR04<TIMER_, TRIGGER_, ECHO_PORT_, ECHO_MASK_>;
			SonarEvent<TIMER_> event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			if (event.ready() || event.started()) CALLBACK_(event);
		}

		template<uint8_t TIMER_NUM_, typename SONAR_, typename EVENT_> static EVENT_ multi_sonar_rtt_change()
		{
			// Update RTT time
			timer::isr_handler_rtt::rtt<TIMER_NUM_>();
			return interrupt::HandlerHolder<SONAR_>::handler()->on_rtt_change();
		}
	};
	/// @endcond
}

#endif /* SONAR_H */
/// @endcond
