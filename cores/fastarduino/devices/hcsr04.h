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

#ifndef HCSR04_H
#define HCSR04_H

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

// Utilities to handle ISR callbacks
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)								\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "ECHO must be an INT pin.");	\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,			\
	"ECHO INT number must match INT_NUM");													\
ISR(CAT3(INT, INT_NUM, _vect))																\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_pin_change)();							\
}

#define REGISTER_HCSR04_PCI_ISR(TIMER_NUM, PCI_NUM, TRIGGER, ECHO)							\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
ISR(CAT3(PCINT, PCI_NUM, _vect))															\
{																							\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);						\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_pin_change)();							\
}

#define REGISTER_HCSR04_ICP_ISR(TIMER_NUM, TRIGGER, ECHO)									\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))														\
{																							\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);						\
	using TRAIT = board_traits::Timer_trait<TIMER>;											\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");						\
	TRAIT::TYPE capture = TRAIT::ICR;														\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, true >;				\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_capture, uint16_t)(capture);			\
}
	// CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_capture, TRAIT::TYPE)(capture);			

#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)	\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");	\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,			\
	"PIN INT number must match INT_NUM");													\
ISR(CAT3(INT, INT_NUM, _vect))																\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);									\
	auto handler = SERVO_HOLDER::handler();													\
	handler->on_pin_change();																\
	if (handler->ready())																	\
		CALL_HANDLER_(HANDLER, CALLBACK, uint16_t)(handler->latest_echo_us());				\
}

#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)			\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");	\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,			\
	"PIN INT number must match INT_NUM");													\
ISR(CAT3(INT, INT_NUM, _vect))																\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);									\
	auto handler = SERVO_HOLDER::handler();													\
	handler->on_pin_change();																\
	if (handler->ready())																	\
		CALLBACK (handler->latest_echo_us());												\
}

#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)	\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
ISR(CAT3(PCINT, PCI_NUM, _vect))															\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);									\
	auto handler = SERVO_HOLDER::handler();													\
	handler->on_pin_change();																\
	if (handler->ready())																	\
		CALL_HANDLER_(HANDLER, CALLBACK, uint16_t)(handler->latest_echo_us());				\
}

#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)			\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
ISR(CAT3(PCINT, PCI_NUM, _vect))															\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);									\
	auto handler = SERVO_HOLDER::handler();													\
	handler->on_pin_change();																\
	if (handler->ready())																	\
		CALLBACK (handler->latest_echo_us());												\
}

namespace devices
{
namespace sonar
{
	static constexpr const uint32_t SPEED_OF_SOUND = 340UL;
	
	// Conversion methods
	static constexpr uint16_t echo_us_to_distance_mm(uint16_t echo_us)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Divide by 2 as echo time includes full sound round-trip
		return uint16_t(echo_us * SPEED_OF_SOUND / 1000UL / 2UL);
	}

	static constexpr uint16_t distance_mm_to_echo_us(uint16_t distance_mm)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Multiply by 2 as echo time must include full sound round-trip
		return uint16_t(distance_mm * 1000UL * 2UL / SPEED_OF_SOUND);
	}

	//TODO (undocumented) abstract sonar class that handles all common stuff.
	//TODO must be able to deal with RTT+PCI/INT and Timer input capture
	template<board::Timer TIMER>
	class AbstractSonar
	{
	protected:
		using TIMER_TYPE = timer::Timer<TIMER>;
		using TYPE = typename TIMER_TYPE::TIMER_TYPE;

	public:
		inline bool ready() const
		{
			return status_ & READY;
		}
		
		inline TYPE latest_echo_ticks() const
		{
			if (sizeof(TYPE) > 1)
				synchronized return echo_pulse_;
			else
				return echo_pulse_;
		}

		TYPE await_echo_ticks(TYPE timeout_ticks)
		{
			// Wait for echo signal start
			timer_.reset();
			while (!(status_ & READY))
				if (timer_.ticks() >= timeout_ticks)
				{
					status_ = READY;
					return 0;
				}
			return echo_pulse_;
		}

	protected:
		AbstractSonar(TIMER_TYPE& timer):timer_{timer}, echo_pulse_{0}, status_{0} {}

		inline void trigger_sent(bool capture)
		{
			if (capture) timer_.set_input_capture(timer::TimerInputCapture::RISING_EDGE);
			status_ = 0;
		}
		inline void pulse_started()
		{
			timer_._reset();
			status_ = STARTED;
		}
		inline void pulse_finished()
		{
			echo_pulse_ = timer_._ticks();
			status_ = READY;
		}
		inline void pulse_captured(TYPE capture)
		{
			if (timer_.input_capture() == timer::TimerInputCapture::RISING_EDGE)
			{
				timer_._reset();
				timer_.set_input_capture(timer::TimerInputCapture::FALLING_EDGE);
				status_ = STARTED;
			}
			else
			{
				echo_pulse_ = capture;
				status_ = READY;
			}
		}

		TIMER_TYPE& timer_;
		volatile TYPE echo_pulse_;

		static constexpr const uint8_t READY = 0x01;
		static constexpr const uint8_t STARTED = 0x02;
		volatile uint8_t status_;
	};

	//TODO create abstract parent with all common code
	//TODO improve API to allow several types instances sharing the same TRIGGER pin
	template<board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO, bool USE_CAPTURE = false>
	class HCSR04: public AbstractSonar<TIMER>
	{
	private:
		using PARENT = AbstractSonar<TIMER>;
		using TIMER_TYPE = timer::Timer<TIMER>;
		
	public:
		using TYPE = typename TIMER_TYPE::TIMER_TYPE;
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000 / SPEED_OF_SOUND + 1;

		HCSR04(timer::Timer<TIMER>& timer)
			:	PARENT{timer}, 
				trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT} {}

		inline void register_handler()
		{
			interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		TYPE echo_ticks(TYPE timeout_ticks)
		{
			async_echo();
			return this->await_echo_ticks(timeout_ticks);
		}
		
		// We want to avoid using await_echo_us() to handle state & timeout!
		void async_echo(bool trigger = true)
		{
			this->trigger_sent(USE_CAPTURE);
			if (trigger) this->trigger();
		}

		void on_pin_change()
		{
			if (echo_.value())
				this->pulse_started();
			// else if (this->ready())
			else
				this->pulse_finished();
		}

		void on_capture(TYPE capture)
		{
			this->pulse_captured(capture);
		}

	private:
		void trigger()
		{
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}
		
		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		typename gpio::FastPinType<ECHO>::TYPE echo_;
	};
}
}

#endif /* HCSR04_H */
