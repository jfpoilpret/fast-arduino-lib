//   Copyright 2016-2018 Jean-Francois Poilpret
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

#ifndef SONAR_H
#define SONAR_H

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

//TODO document!
// Utilities to handle ISR callbacks
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)					\
	ISR(CAT3(INT, INT_NUM, _vect))												\
	{																			\
		devices::sonar::isr_handler_sonar_int<INT_NUM, TIMER, TRIGGER, ECHO>();	\
	}

#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)								\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                							\
	{                                                               							\
		devices::sonar::isr_handler_sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHO, ##__VA_ARGS__>();	\
	}

#define REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)								\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                 										\
	{                                                                										\
		devices::sonar::isr_handler_sonar_distinct_pci<PCI_NUM, TIMER, TRIGGER, ECHO,  ##__VA_ARGS__>();	\
	}

#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)            		\
	ISR(CAT3(INT, INT_NUM, _vect))                                                                  		\
	{                                                                                               		\
		devices::sonar::isr_handler_sonar_int_method<INT_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>();	\
	}

#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)                	\
	ISR(CAT3(INT, INT_NUM, _vect))                                                               	\
	{                                                                                            	\
		devices::sonar::isr_handler_sonar_int_function<INT_NUM, TIMER, TRIGGER, ECHO, CALLBACK>();	\
	}

#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)            		\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                		\
	{                                                                                               		\
		devices::sonar::isr_handler_sonar_pci_method<PCI_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>();	\
	}

#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)                  	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                               	\
	{                                                                                              	\
		devices::sonar::isr_handler_sonar_pci_function<PCI_NUM, TIMER, TRIGGER, ECHO, CALLBACK>();	\
	}

#define REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK)	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                    		\
	{                                                                                                   		\
		devices::sonar::isr_handler_multi_sonar_pci_method<PCI_NUM, TIMER, TRIGGER, 							\
			ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK>();															\
	}

#define REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(PCI_NUM, TIMER, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK)	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                              						\
	{                                                                             						\
		devices::sonar::isr_handler_multi_sonar_pci_function<PCI_NUM, TIMER, TRIGGER, 					\
			ECHO_PORT, ECHO_MASK, CALLBACK>();															\
	}

namespace devices::sonar
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

	enum class SonarType : uint8_t
	{
		BLOCKING,
		ASYNC_INT,
		ASYNC_PCINT
	};

	//FIXME missing timeout
	template<board::Timer NTIMER_> class AbstractSonar
	{
	public:
		using RTT = timer::RTT<NTIMER_>;

		inline bool ready() const
		{
			return status_ == READY;
		}

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

	protected:
		AbstractSonar(const RTT& rtt) : rtt_{rtt}, status_{UNKNOWN}, echo_start_{}, echo_end_{}
		{
		}

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
						echo_start_ = echo_end_ = time::RTTTime{};
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
				echo_start_ = rtt_.time();
			}
			// Wait for echo signal end
			while (echo.value())
				if (rtt_.millis() >= now) return 0;
			synchronized
			{
				status_ = READY;
				echo_end_ = rtt_.time();
				return echo_time_();
			}
		}

		inline void trigger_sent()
		{
			status_ = TRIGGERED;
		}

		inline bool pulse_edge(bool rising)
		{
			if (rising && status_ == TRIGGERED)
			{
				status_ = ECHO_STARTED;
				echo_start_ = rtt_.time_();
			}
			else if ((!rising) && status_ == ECHO_STARTED)
			{
				status_ = READY;
				echo_end_ = rtt_.time_();
				return true;
			}
			return false;
		}

	private:
		uint16_t echo_time_() const
		{
			return uint16_t((echo_end_ - echo_start_).total_micros());
		}

		const RTT& rtt_;

		static constexpr const uint8_t UNKNOWN = 0x00;
		static constexpr const uint8_t TRIGGERED = 0x10;
		static constexpr const uint8_t ECHO_STARTED = 0x11;
		static constexpr const uint8_t READY = 0x20;

		volatile uint8_t status_;
		time::RTTTime echo_start_;
		time::RTTTime echo_end_;
	};

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
			 SonarType SONAR_TYPE_ = SonarType::BLOCKING>
	class HCSR04 : public AbstractSonar<NTIMER_>
	{
	public:
		using RTT = timer::RTT<NTIMER_>;
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		static constexpr const board::DigitalPin ECHO = ECHO_;
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
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		HCSR04(const RTT& rtt) : PARENT{rtt}, trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}
		{
		}

		inline void register_handler()
		{
			static_assert(SONAR_TYPE != SonarType::BLOCKING,
						  "register_handler() must not be called with SonarType::BLOCKING");
			interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		uint16_t echo_us(uint16_t timeout_ms)
		{
			async_echo();
			return await_echo_us(timeout_ms);
		}

		uint16_t await_echo_us(uint16_t timeout_ms)
		{
			if (SONAR_TYPE != SonarType::BLOCKING)
				return this->async_echo_us(timeout_ms);
			else
				return this->template blocking_echo_us<ECHO>(echo_, timeout_ms);
		}

		// We want to avoid using await_echo_us() to handle state & timeout!
		void async_echo(bool trigger = true)
		{
			this->trigger_sent();
			if (trigger) this->trigger();
		}

	private:
		bool on_pin_change()
		{
			static_assert(SONAR_TYPE == SonarType::ASYNC_INT || SONAR_TYPE == SonarType::ASYNC_PCINT,
						  "on_pin_change() must be called only with SonarType::ASYNC_INT or ASYNC_PCINT");
			return this->pulse_edge(echo_.value());
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
		template<int, board::Timer, board::DigitalPin, board::DigitalPin>
		friend bool isr_handler_sonar_int();
		template<int, board::Timer, board::DigitalPin, board::DigitalPin, board::DigitalPin...>
		friend bool isr_handler_sonar_pci();
		template<int, board::Timer, board::DigitalPin, board::DigitalPin, board::DigitalPin...>
		friend bool isr_handler_sonar_distinct_pci();
	};

	struct SonarEvent
	{
	public:
		SonarEvent() : started_{}, ready_{}, time_{}
		{
		}
		SonarEvent(uint8_t started, uint8_t ready, time::RTTTime time) : started_{started}, ready_{ready}, time_{time}
		{
		}

		uint8_t started() const
		{
			return started_;
		}
		uint8_t ready() const
		{
			return ready_;
		}
		time::RTTTime time() const
		{
			return time_;
		}

	private:
		uint8_t started_;
		uint8_t ready_;
		time::RTTTime time_;
	};

	//FIXME missing timeout
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

		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		MultiHCSR04(RTT& rtt)
			: rtt_{rtt}, started_{}, ready_{}, active_{false}, trigger_{gpio::PinMode::OUTPUT}, echo_{0}
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

		void trigger()
		{
			started_ = 0;
			ready_ = 0;
			active_ = true;
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

	private:
		SonarEvent on_pin_change()
		{
			if (!active_) return SonarEvent{};
			// Compute the newly started echoes
			uint8_t pins = echo_.get_PIN();
			uint8_t started = pins & ~started_;
			// Compute the newly finished echoes
			uint8_t ready = ~pins & started_ & ~ready_;
			// Update status of all echo pins
			started_ |= started;
			ready_ |= ready;
			if (ready_ == ECHO_MASK) active_ = false;
			return SonarEvent{started, ready, rtt_.time_()};
		}

		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		RTT& rtt_;
		volatile uint8_t started_;
		volatile uint8_t ready_;
		volatile bool active_;
		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		gpio::FastMaskedPort<ECHO_PORT, ECHO_MASK> echo_;

		// Make friends with all ISR handlers
		template<int, board::Timer, board::DigitalPin, board::Port, uint8_t,
			typename HANDLER, void (HANDLER::*CALLBACK)(const SonarEvent&)>
		friend void isr_handler_multi_sonar_pci_method();
		template<int, board::Timer, board::DigitalPin, board::Port, uint8_t, void (*CALLBACK)()>
		friend void isr_handler_multi_sonar_pci_function();
	};

	/// @cond notdocumented
	// All sonar-related methods called by pre-defined ISR are defined here
	template<int INT_NUM, board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO>
	bool isr_handler_sonar_int()
	{
		static_assert(board_traits::DigitalPin_trait<ECHO>::IS_INT, "ECHO must be an INT pin.");
		static_assert(board_traits::ExternalInterruptPin_trait<ECHO>::INT == INT_NUM,
					"ECHO INT number must match INT_NUM");
		using SONAR = HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_INT>;
		return interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
	}

	template<int INT_NUM, board::Timer TIMER,
		board::DigitalPin TRIGGER, board::DigitalPin ECHO, typename HANDLER, void (HANDLER::*CALLBACK)()>
	void isr_handler_sonar_int_method()
	{
		if (isr_handler_sonar_int<INT_NUM, TIMER, TRIGGER, ECHO>())
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();
	}

	template<int INT_NUM, board::Timer TIMER,
		board::DigitalPin TRIGGER, board::DigitalPin ECHO, void (*CALLBACK)()>
	void isr_handler_sonar_int_function()
	{
		if (isr_handler_sonar_int<INT_NUM, TIMER, TRIGGER, ECHO>())
			CALLBACK();
	}

	template<int PCI_NUM, board::Timer TIMER, board::DigitalPin TRIGGER>
	bool isr_handler_sonar_pci()
	{
		return false;
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::DigitalPin ECHO1, board::DigitalPin... ECHOS>
	bool isr_handler_sonar_pci()
	{
		// handle first echo pin
		static_assert(board_traits::PCI_trait<PCI_NUM>::PORT != board::Port::NONE, "PORT must support PCI");
		static_assert(board_traits::DigitalPin_trait<ECHO1>::PORT == board_traits::PCI_trait<PCI_NUM>::PORT,
					"ECHO port must match PCI_NUM port");
		static_assert(_BV(board_traits::DigitalPin_trait<ECHO1>::BIT) & board_traits::PCI_trait<PCI_NUM>::PCI_MASK,
					"ECHO must be a PCINT pin");
		using SONAR = HCSR04<TIMER, TRIGGER, ECHO1, SonarType::ASYNC_PCINT>;
		bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		// handle other echo pins
		return result || isr_handler_sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHOS...>();
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::DigitalPin ECHO, typename HANDLER, void (HANDLER::*CALLBACK)()>
	void isr_handler_sonar_pci_method()
	{
		if (isr_handler_sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHO>())
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::DigitalPin ECHO, void (*CALLBACK)()>
	void isr_handler_sonar_pci_function()
	{
		if (isr_handler_sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHO>())
			CALLBACK();
	}

	template<int PCI_NUM, board::Timer TIMER>
	bool isr_handler_sonar_distinct_pci()
	{
		return false;
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::DigitalPin ECHO, board::DigitalPin... TRIGGER_ECHOS>
	bool isr_handler_sonar_distinct_pci()
	{
		// handle first echo pin
		static_assert(board_traits::PCI_trait<PCI_NUM>::PORT != board::Port::NONE, "PORT must support PCI");
		static_assert(board_traits::DigitalPin_trait<ECHO>::PORT == board_traits::PCI_trait<PCI_NUM>::PORT,
					"ECHO port must match PCI_NUM port");
		static_assert(_BV(board_traits::DigitalPin_trait<ECHO>::BIT) & board_traits::PCI_trait<PCI_NUM>::PCI_MASK,
					"ECHO must be a PCINT pin");
		using SONAR = HCSR04<TIMER, TRIGGER, ECHO, SonarType::ASYNC_PCINT>;
		bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		// handle other echo pins
		return result || isr_handler_sonar_pci<PCI_NUM, TIMER, TRIGGER_ECHOS...>();
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::Port ECHO_PORT, uint8_t ECHO_MASK,
		typename HANDLER, void (HANDLER::*CALLBACK)(const SonarEvent&)>
	void isr_handler_multi_sonar_pci_method()
	{
		static_assert(board_traits::PCI_trait<PCI_NUM>::PORT == ECHO_PORT, "ECHO_PORT must match PCI_NUM");
		using PTRAIT = board_traits::Port_trait<ECHO_PORT>;
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK) == ECHO_MASK, "ECHO_MASK must contain available PORT pins");
		using SONAR = MultiHCSR04<TIMER, TRIGGER, ECHO_PORT, ECHO_MASK>;
		SonarEvent event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		if (event.ready() || event.started())
			interrupt::CallbackHandler<void (HANDLER::*)(const SonarEvent&), CALLBACK>::call(event);
	}

	template<int PCI_NUM, board::Timer TIMER, 
		board::DigitalPin TRIGGER, board::Port ECHO_PORT, uint8_t ECHO_MASK, void (*CALLBACK)(const SonarEvent&)>
	void isr_handler_multi_sonar_pci_function()
	{
		static_assert(board_traits::PCI_trait<PCI_NUM>::PORT != ECHO_PORT, "ECHO_PORT must match PCI_NUM");
		using PTRAIT = board_traits::Port_trait<ECHO_PORT>;
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK) == ECHO_MASK, "ECHO_MASK must contain available PORT pins");
		using SONAR = MultiHCSR04<TIMER, TRIGGER, ECHO_PORT, ECHO_MASK>;
		SonarEvent event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		if (event.ready() || event.started())
			CALLBACK(event);
	}
	/// @endcond
}

#endif /* SONAR_H */
