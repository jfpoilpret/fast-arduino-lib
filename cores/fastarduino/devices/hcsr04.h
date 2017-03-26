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
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");	\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,			\
	"PIN INT number must match INT_NUM");													\
ISR(CAT3(INT, INT_NUM, _vect))																\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_echo)();								\
}

#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO)								\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																\
ISR(CAT3(PCINT, PCI_NUM, _vect))															\
{																							\
	using SERVO_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO >;					\
	CALL_HANDLER_(SERVO_HANDLER, &SERVO_HANDLER::on_echo)();								\
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

	//TODO Add API to support callback in case distance becomes out of a specified range
	//TODO improve API to allow several types instances sharing the same TRIGGER pin
	template<board::Timer TIMER, board::DigitalPin TRIGGER, board::DigitalPin ECHO>
	class HCSR04
	{
	public:
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000 / SPEED_OF_SOUND + 1;

		HCSR04(timer::RTT<TIMER>& rtt, bool async = true)
			:	rtt_{rtt}, 
				trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}, 
				start_{}, echo_pulse_{0}, status_{0}
		{
			// If async mode is gonna be used, then register this as interrupt handler (for INT or PCI)
			if (async)
				interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		uint16_t echo_us(uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			trigger();
			// Wait for echo signal start
			timeout_ms += rtt_.millis();
			while (!echo_.value())
				if (rtt_.millis() >= timeout_ms) return 0;
			// Read current time (need RTT)
			time::RTTTime start = rtt_.time();
			// Wait for echo signal end
			while (echo_.value())
				if (rtt_.millis() >= timeout_ms) return 0;
			// Read current time (need RTT)
			time::RTTTime end = rtt_.time();
			time::RTTTime delta = time::delta(start, end);
			return uint16_t(delta.millis * 1000UL + delta.micros);
		}
		
		void async_echo(bool trigger = true)
		{
			status_ = 0;
			if (trigger) this->trigger();
		}

		bool ready() const
		{
			return status_ & READY;
		}

		uint16_t await_echo_us(uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			// Wait for echo signal start
			timeout_ms += rtt_.millis();
			while (!(status_ & READY))
				if (rtt_.millis() >= timeout_ms)
				{
					status_ = READY;
					return 0;
				}
			return echo_pulse_;
		}

		void on_echo()
		{
			if (echo_.value())
			{
				// pulse started
				start_ = rtt_.time();
				status_ = STARTED;
			}
			else if (status_ & STARTED)
			{
				// pulse ended
				time::RTTTime end = rtt_.time();
				time::RTTTime delta = time::delta(start_, end);
				echo_pulse_ = uint16_t(delta.millis * 1000UL + delta.micros);
				status_ = READY;
			}
		}

	private:
		void trigger()
		{
			rtt_.millis(0);
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}
		
		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		timer::RTT<TIMER>& rtt_;
		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		typename gpio::FastPinType<ECHO>::TYPE echo_;
		time::RTTTime start_;
		volatile uint16_t echo_pulse_;

		static constexpr const uint8_t READY = 0x01;
		static constexpr const uint8_t STARTED = 0x02;
		volatile uint8_t status_;
	};
}
}

#endif /* HCSR04_H */
