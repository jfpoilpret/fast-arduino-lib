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
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)										\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "ECHO must be an INT pin.");			\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,					\
	"ECHO INT number must match INT_NUM");															\
ISR(CAT3(INT, INT_NUM, _vect))																		\
{																									\
	using TRAIT = board_traits::Timer_trait<TIMER>;													\
	TRAIT::TYPE counter = TRAIT::TCNT;																\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_INT>;		\
	CALL_HANDLER_RETURN_(SERVO_HANDLER, &SERVO_HANDLER::on_pin_change, bool, TRAIT::TYPE)(counter);	\
}

//TODO Allow multiple echo pins for the same PCINT
#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO)										\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																		\
ISR(CAT3(PCINT, PCI_NUM, _vect))																	\
{																									\
	using TRAIT = board_traits::Timer_trait<TIMER>;													\
	TRAIT::TYPE counter = TRAIT::TCNT;																\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_PCINT>;		\
	CALL_HANDLER_RETURN_(SERVO_HANDLER, &SERVO_HANDLER::on_pin_change, bool, TRAIT::TYPE)(counter);	\
}

#define REGISTER_HCSR04_ICP_ISR(TIMER_NUM, TRIGGER, ECHO)										\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_ICP>;	\
	CALL_HANDLER_RETURN_(SERVO_HANDLER, &SERVO_HANDLER::on_capture, bool, TRAIT::TYPE)(capture);\
}

#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)		\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");		\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,				\
	"PIN INT number must match INT_NUM");														\
ISR(CAT3(INT, INT_NUM, _vect))																	\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_INT >;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(counter);									\
}

#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)				\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");		\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,				\
	"PIN INT number must match INT_NUM");														\
ISR(CAT3(INT, INT_NUM, _vect))																	\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_INT >;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALLBACK (counter);																		\
}

#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)		\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																	\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_PCINT>;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(counter);									\
}

#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)				\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																	\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_PCINT>;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALLBACK (counter);																		\
}

#define REGISTER_HCSR04_ICP_ISR_METHOD(TIMER_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)				\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_ICP>;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_capture(capture))															\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(capture);									\
}

#define REGISTER_HCSR04_ICP_ISR_FUNCTION(TIMER_NUM, TRIGGER, ECHO, CALLBACK)					\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_ICP>;	\
	using SERVO_HOLDER = HANDLER_HOLDER_(SERVO_HANDLER);										\
	auto handler = SERVO_HOLDER::handler();														\
	if (handler->on_capture(capture))															\
		CALLBACK (capture);																		\
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

	enum class SonarType
	{
		BLOCKING,
		ASYNC_INT,
		ASYNC_PCINT,
		ASYNC_ICP
	};

	//TODO (undocumented) abstract sonar class that handles all common stuff.
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

	protected:
		AbstractSonar(TIMER_TYPE& timer):timer_{timer}, echo_pulse_{0}, status_{0} {}

		TYPE async_echo_ticks(TYPE timeout_ticks)
		{
			// Wait for echo signal start
			while (!(status_ & READY))
				if (timer_.ticks() >= timeout_ticks)
				{
					status_ = READY;
					return 0;
				}
			return echo_pulse_;
		}

		template<board::DigitalPin ECHO>
		TYPE blocking_echo_ticks(typename gpio::FastPinType<ECHO>::TYPE& echo, TYPE timeout_ticks)
		{
			timer_.reset();
			while (!echo.value())
				if (timer_.ticks() >= timeout_ticks)
					return 0;
			echo_pulse_ = timer_.ticks();
			status_ = STARTED;
			// Wait for echo signal end
			while (echo.value())
				if (timer_.ticks() >= timeout_ticks)
					return 0;
			echo_pulse_ = timer_.ticks() - echo_pulse_;
			status_ = READY;
			return echo_pulse_;
		}

		inline void trigger_sent(bool capture, bool reset)
		{
			if (capture) timer_.set_input_capture(timer::TimerInputCapture::RISING_EDGE);
			if (reset) timer_.reset();
			status_ = 0;
		}

		inline bool pulse_edge(bool rising, TYPE ticks)
		{
			if (rising && status_ == 0)
			{
				echo_pulse_ = ticks;
				status_ = STARTED;
			}
			else if ((!rising) && status_ == STARTED)
			{
				echo_pulse_ = ticks - echo_pulse_;
				status_ = READY;
				return true;
			}
			return false;
		}

		inline bool pulse_captured(TYPE capture)
		{
			bool rising = (timer_.input_capture() == timer::TimerInputCapture::RISING_EDGE);
			if (rising)
				timer_.set_input_capture(timer::TimerInputCapture::FALLING_EDGE);
			return pulse_edge(rising, capture);
		}

	private:
		TIMER_TYPE& timer_;
		volatile TYPE echo_pulse_;

		static constexpr const uint8_t READY = 0x01;
		static constexpr const uint8_t STARTED = 0x02;
		volatile uint8_t status_;
	};

	template<
		board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO, 
		SonarType SONAR_TYPE = SonarType::BLOCKING>
	class HCSR04: public AbstractSonar<TIMER>
	{
	private:
		using PARENT = AbstractSonar<TIMER>;
		using TIMER_TYPE = timer::Timer<TIMER>;
		using TIMER_TRAIT = board_traits::Timer_trait<TIMER>;
		using ECHO_PIN_TRAIT = board_traits::DigitalPin_trait<ECHO>;
		using ECHO_PORT_TRAIT = board_traits::Port_trait<ECHO_PIN_TRAIT::PORT>;
		static_assert(SONAR_TYPE != SonarType::ASYNC_ICP || TIMER_TRAIT::ICP_PIN == ECHO, 
			"SONAR_TYPE == ASYNC_ICP but ECHO is not an ICP pin");
		static_assert(SONAR_TYPE != SonarType::ASYNC_INT || ECHO_PIN_TRAIT::IS_INT, 
			"SONAR_TYPE == ASYNC_INT but ECHO is not an INT pin");
		static_assert(SONAR_TYPE != SonarType::ASYNC_PCINT || ECHO_PORT_TRAIT::PCINT != 0xFF, 
			"SONAR_TYPE == ASYNC_PCINT but ECHO is not an PCI pin");
				
	public:
		using TYPE = typename TIMER_TYPE::TIMER_TYPE;
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		HCSR04(timer::Timer<TIMER>& timer)
			:	PARENT{timer}, 
				trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT} {}

		inline void register_handler()
		{
			static_assert(SONAR_TYPE != SonarType::BLOCKING, 
				"register_handler() must not be called with SonarType::BLOCKING");
			interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		TYPE echo_ticks(TYPE timeout_ticks)
		{
			async_echo();
			return await_echo_ticks(timeout_ticks);
		}

		TYPE await_echo_ticks(TYPE timeout_ticks)
		{
			if (SONAR_TYPE != SonarType::BLOCKING)
				return this->async_echo_ticks(timeout_ticks);
			else
				return this->template blocking_echo_ticks<ECHO>(echo_, timeout_ticks);
		}
		
		// We want to avoid using await_echo_us() to handle state & timeout!
		void async_echo(bool trigger = true)
		{
			this->trigger_sent(SONAR_TYPE == SonarType::ASYNC_ICP, trigger);
			if (trigger) this->trigger();
		}

		bool on_pin_change(TYPE ticks)
		{
			static_assert(SONAR_TYPE == SonarType::ASYNC_INT || SONAR_TYPE == SonarType::ASYNC_PCINT, 
				"on_pin_change() must be called only with SonarType::ASYNC_INT or ASYNC_PCINT");
			return this->pulse_edge(echo_.value(), ticks);
		}

		bool on_capture(TYPE capture)
		{
			static_assert(SONAR_TYPE == SonarType::ASYNC_ICP, 
				"on_capture() must be called only with SonarType::ASYNC_ICP");
			return this->pulse_captured(capture);
		}

	private:
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
	};
}
}

#endif /* HCSR04_H */
