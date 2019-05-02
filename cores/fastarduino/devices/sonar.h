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
 * @param TRIGGER the `board::DigitalPin` connected to the 1st sonar trigger pin
 * @param ECHO the `board::DigitalPin` connected to the 1st sonar echo pin
 * @param ... other pairs of (trigger pin, echo pin) for other `HCSR04`
 * 
 * @sa devices::sonar::HCSR04
 * @sa REGISTER_HCSR04_PCI_ISR()
 */
#define REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)                             \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                     \
	{                                                                                                    \
		devices::sonar::isr_handler::sonar_distinct_pci<PCI_NUM, TIMER, TRIGGER, ECHO, ##__VA_ARGS__>(); \
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
 * `board::InterruptPin`,  along with a callback function that will be 
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
 * @param TIMER the `board::Timer` type used to instantiate the related
 * `devices::sonar::HCSR04` template classes.
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
#define REGISTER_HCSR04_RTT_TIMEOUT(TIMER, SONAR, ...)                                    \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                              \
	{                                                                                     \
		devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>(); \
	}

//TODO document!
#define REGISTER_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, HANDLER, CALLBACK, SONAR, ...)          \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                  \
	{                                                                                         \
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>()) \
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();                \
	}

//TODO document!
#define REGISTER_HCSR04_RTT_TIMEOUT_FUNCTION(TIMER_NUM, CALLBACK, SONAR, ...)                 \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                  \
	{                                                                                         \
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>()) \
			interrupt::CallbackHandler<void (*)(), CALLBACK>::call();                         \
	}

//TODO document!
#define REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK) \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                           \
	{                                                                                                          \
		using devices::sonar::isr_handler::multi_sonar_pci_method;                                             \
		multi_sonar_pci_method<PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK>();            \
	}

//TODO document!
#define REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK) \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                    \
	{                                                                                                   \
		using devices::sonar::isr_handler::multi_sonar_pci_method;                                      \
		multi_sonar_pci_function<PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK>();            \
	}

//TODO document!
#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT(TIMER, SONAR)                                 \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                            \
	{                                                                                   \
		using EVENT = typename SONAR::EVENT;                                            \
		devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR, EVENT>(); \
	}

//TODO document!
#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, SONAR, HANDLER, CALLBACK)                            \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                                                     \
	{                                                                                                            \
		using EVENT = typename SONAR::EVENT;                                                                     \
		EVENT event = devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR, EVENT>();            \
		if (event.timeout()) interrupt::CallbackHandler<void (HANDLER::*)(const EVENT&), CALLBACK>::call(event); \
	}

//TODO document!
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

	/// @cond notdocumented
	protected:
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
		HCSR04(const RTT& rtt) : PARENT{rtt}, trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT} {}

		/**
		 * Register this HCSR04 with the matching ISR that should have been
		 * registered with REGISTER_HCSR04_INT_ISR(), REGISTER_HCSR04_PCI_ISR(),
		 * REGISTER_DISTINCT_HCSR04_PCI_ISR(), REGISTER_HCSR04_RTT_TIMEOUT(), or
		 * any derived macros (with function or method callback).
		 * This method must be called if `SONAR_TYPE` is not `SonarType::BLOCKING`.
		 * This method shall not be called if `SONAR_TYPE` is `SonarType::BLOCKING`,
		 * otherwise compilation will fail.
		 */
		inline void register_handler()
		{
			static_assert(SONAR_TYPE != SonarType::BLOCKING,
						  "register_handler() must not be called with SonarType::BLOCKING");
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

	//TODO document!
	template<board::Timer NTIMER_> struct SonarEvent
	{
	public:
		using RTT = timer::RTT<NTIMER_>;
		using RAW_TIME = typename RTT::RAW_TIME;

		SonarEvent(bool timeout = false) : timeout_{timeout}, started_{}, ready_{}, time_{RAW_TIME::EMPTY_TIME} {}
		SonarEvent(uint8_t started, uint8_t ready, const RAW_TIME& time) : started_{started}, ready_{ready}, time_{time}
		{}

		bool timeout() const
		{
			return timeout_;
		}
		uint8_t started() const
		{
			return started_;
		}
		uint8_t ready() const
		{
			return ready_;
		}
		RAW_TIME time() const
		{
			return time_;
		}

	private:
		bool timeout_;
		uint8_t started_;
		uint8_t ready_;
		RAW_TIME time_;
	};

	//TODO document!
	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_>
	class MultiHCSR04
	{
	public:
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		static constexpr const board::Port ECHO_PORT = ECHO_PORT_;
		static constexpr const uint8_t ECHO_MASK = ECHO_MASK_;

	private:
		using PTRAIT = board_traits::Port_trait<ECHO_PORT>;
		static_assert(PTRAIT::PCINT != 0xFF, "ECHO_PORT_ must support PCINT");
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK) == ECHO_MASK, "ECHO_MASK_ must contain available PORT pins");

	public:
		using RTT = timer::RTT<NTIMER_>;
		using EVENT = SonarEvent<NTIMER_>;

		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		MultiHCSR04(RTT& rtt)
			: rtt_{rtt}, started_{}, ready_{}, active_{false},
			  timeout_time_ms_{}, trigger_{gpio::PinMode::OUTPUT}, echo_{0}
		{
			interrupt::register_handler(*this);
		}

		uint8_t ready() const
		{
			return ready_;
		}

		bool all_ready() const
		{
			return ready_ == ECHO_MASK;
		}

		void set_ready()
		{
			if (active_)
			{
				ready_ = ECHO_MASK;
				active_ = false;
			}
		}

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
		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_>
		static bool sonar_int()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::DigitalPin_trait<ECHO_>::IS_INT, "ECHO must be an INT pin.");
			static_assert(board_traits::ExternalInterruptPin_trait<ECHO_>::INT == INT_NUM_,
						  "ECHO INT number must match INT_NUM");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO_, SonarType::ASYNC_INT>;
			return interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
				 typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_int_method()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
				 void (*CALLBACK_)()>
		static void sonar_int_function()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>()) CALLBACK_();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_> static bool sonar_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO1_,
				 board::DigitalPin... ECHOS_>
		static bool sonar_pci()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			static_assert(board_traits::DigitalPin_trait<ECHO1_>::PORT == board_traits::PCI_trait<PCI_NUM_>::PORT,
						  "ECHO port must match PCI_NUM port");
			static_assert(
				_BV(board_traits::DigitalPin_trait<ECHO1_>::BIT) & board_traits::PCI_trait<PCI_NUM_>::PCI_MASK,
				"ECHO must be a PCINT pin");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO1_, SonarType::ASYNC_PCINT>;
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

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
				 typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_pci_method()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
				 void (*CALLBACK_)()>
		static void sonar_pci_function()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>()) CALLBACK_();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_> static bool sonar_distinct_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
				 board::DigitalPin... TRIGGER_ECHOS_>
		static bool sonar_distinct_pci()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			static_assert(board_traits::DigitalPin_trait<ECHO_>::PORT == board_traits::PCI_trait<PCI_NUM_>::PORT,
						  "ECHO port must match PCI_NUM port");
			static_assert(_BV(board_traits::DigitalPin_trait<ECHO_>::BIT) & board_traits::PCI_trait<PCI_NUM_>::PCI_MASK,
						  "ECHO must be a PCINT pin");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO_, SonarType::ASYNC_PCINT>;
			bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			// handle other echo pins
			return result || sonar_pci<PCI_NUM_, TIMER_, TRIGGER_ECHOS_...>();
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
