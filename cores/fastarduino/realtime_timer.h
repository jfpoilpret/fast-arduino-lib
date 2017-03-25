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

#ifndef RTT_HH
#define RTT_HH

#include <avr/interrupt.h>
#include "interrupts.h"
#include "timer.h"
#include "time.h"
#include "events.h"

#define REGISTER_RTT_ISR(TIMER_NUM)	\
REGISTER_TIMER_ISR_METHOD(TIMER_NUM, CAT(timer::RTT<board::Timer::TIMER, TIMER_NUM) >, CAT(&timer::RTT<board::Timer::TIMER, TIMER_NUM) >::on_timer)

// Utilities to handle ISR callbacks
#define REGISTER_RTT_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)							\
ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))												\
{																						\
	using RTT_HANDLER = CAT(timer::RTT<board::Timer::TIMER, TIMER_NUM) >;				\
	using RTT_HOLDER = HANDLER_HOLDER_(RTT_HANDLER);									\
	using RTT_HANDLE = CALLBACK_HANDLER_HOLDER_(RTT_HANDLER, &RTT_HANDLER::on_timer);	\
	RTT_HANDLE::handle();																\
	CALL_HANDLER_(HANDLER, CALLBACK, uint32_t)(RTT_HOLDER::handler()->millis());		\
}

#define REGISTER_RTT_ISR_FUNCTION(TIMER_NUM, CALLBACK)									\
ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))												\
{																						\
	using RTT_HANDLER = CAT(timer::RTT<board::Timer::TIMER, TIMER_NUM) >;				\
	using RTT_HOLDER = HANDLER_HOLDER_(RTT_HANDLER);									\
	using RTT_HANDLE = CALLBACK_HANDLER_HOLDER_(RTT_HANDLER, &RTT_HANDLER::on_timer);	\
	RTT_HANDLE::handle();																\
	CALLBACK (RTT_HOLDER::handler()->millis());											\
}

namespace timer
{
	template<board::Timer TIMER>
	class RTT: private Timer<TIMER>
	{
	private:
		using TRAIT = typename Timer<TIMER>::TRAIT;
		using TIMER_TYPE = typename Timer<TIMER>::TIMER_TYPE;
		using TIMER_PRESCALER = typename Timer<TIMER>::TIMER_PRESCALER;

	public:
		RTT():Timer<TIMER>{TimerMode::CTC} {}
		
		void register_rtt_handler()
		{
			interrupt::register_handler(*this);
		}

		inline uint32_t millis() const
		{
			synchronized return _millis;
		}
		void delay(uint32_t ms) const
		{
			uint32_t end = millis() + ms + 1;
			while (millis() < end)
				time::yield();
		}
		inline uint16_t micros() const
		{
			synchronized return compute_micros();
		}

		time::RTTTime time() const
		{
			synchronized return time::RTTTime(_millis, compute_micros());
		}

		inline void millis(uint32_t ms)
		{
			synchronized
			{
				_millis = ms;
				// Reset timer counter
				//TODO do we absolutely need the cast here?
				(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
			}
		}

		inline void begin()
		{
			synchronized _begin();
		}
		inline void _begin()
		{
			_millis = 0;
			Timer<TIMER>::_begin(MILLI_PRESCALER, MILLI_COUNTER);
		}
		inline void end()
		{
			Timer<TIMER>::end();
		}
		inline void _end()
		{
			Timer<TIMER>::_end();
		}

		void on_timer()
		{
			++_millis;
		}

	protected:
		volatile uint32_t _millis;

	private:
		using CALC = Calculator<TIMER>;
		static constexpr const uint32_t ONE_MILLI = 1000UL;
		static constexpr const TIMER_PRESCALER MILLI_PRESCALER = CALC::CTC_prescaler(ONE_MILLI);
		static constexpr const TIMER_TYPE MILLI_COUNTER = CALC::CTC_counter(MILLI_PRESCALER, ONE_MILLI);

		inline uint16_t compute_micros() const
		{
			return uint16_t(ONE_MILLI * ((volatile TIMER_TYPE&) TRAIT::TCNT) / (1 + (volatile TIMER_TYPE&) TRAIT::OCRA));
		}
	};

	template<uint32_t PERIOD_MS = 1024>
	class RTTEventCallback
	{
		static_assert((PERIOD_MS & (PERIOD_MS - 1)) == 0, "PERIOD_MS must be a power of 2");
	public:
		RTTEventCallback(containers::Queue<events::Event>& event_queue)
			:_event_queue(event_queue) {}

		void on_rtt_change(uint32_t millis)
		{
			if ((millis & (PERIOD_MS - 1)) == 0)
				_event_queue._push(events::Event{events::Type::RTT_TIMER});
		}

		containers::Queue<events::Event>& _event_queue;
	};
}

#endif /* RTT_HH */
