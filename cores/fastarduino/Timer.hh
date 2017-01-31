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

#ifndef TIMER_HH
#define TIMER_HH

#include <avr/interrupt.h>
#include <stddef.h>

#include "utilities.hh"
#include "Board_traits.hh"

//TODO do we need to put everything here in a namespace?

#define REGISTER_TIMER_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), HANDLER, CALLBACK)

#define REGISTER_TIMER_ISR_FUNCTION(TIMER_NUM, CALLBACK)	\
REGISTER_ISR_FUNCTION_(CAT3(TIMER, TIMER_NUM, _COMPA_vect), CALLBACK)

#define REGISTER_TIMER_ISR_EMPTY(TIMER_NUM)	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect));

template<Board::Timer TIMER>
class Timer
{
protected:
	using TRAIT = Board::Timer_trait<TIMER>;
	using PRESCALERS_TRAIT = typename TRAIT::PRESCALERS_TRAIT;
	
public:
	using TIMER_TYPE = typename TRAIT::TYPE;
	using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;

	static constexpr bool is_adequate(TIMER_PRESCALER p, uint32_t us)
	{
		return prescaler_is_adequate(prescaler_quotient(p, us));
	}
	static constexpr TIMER_PRESCALER prescaler(uint32_t us)
	{
		return best_prescaler(PRESCALERS_TRAIT::ALL_PRESCALERS, us);
	}
	static constexpr TIMER_TYPE counter(TIMER_PRESCALER prescaler, uint32_t us)
	{
		return (TIMER_TYPE) prescaler_quotient(prescaler, us) - 1;
	}
	static constexpr TIMER_TYPE counter(uint32_t us)
	{
		return (TIMER_TYPE) prescaler_quotient(prescaler(us), us) - 1;
	}

	inline void begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
	{
		synchronized _begin(prescaler, max);
	}

	// We'll need additional methods in Timer_trait<>
	inline void _begin(TIMER_PRESCALER prescaler, TIMER_TYPE max)
	{
		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
		(volatile uint8_t&) TRAIT::TCCRA = TRAIT::CTC_TCCRA;
		// Don't force output compare (FOCA & FOCB), Clock Select according to prescaler
		(volatile uint8_t&) TRAIT::TCCRB = TRAIT::CTC_TCCRB | TRAIT::TCCRB_prescaler(prescaler);
		// Set timer counter compare match (when value reached, 1ms has elapsed)
		(volatile TIMER_TYPE&) TRAIT::OCRA = max;
		// Reset timer counter
		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
	}
	inline void suspend()
	{
		synchronized _suspend();
	}
	inline void _suspend()
	{
		// Clear timer interrupt mode
		(volatile uint8_t&) TRAIT::TIMSK = 0;
	}
	inline void resume()
	{
		synchronized _resume();
	}
	inline void _resume()
	{
		// Reset timer counter
		(volatile TIMER_TYPE&) TRAIT::TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = _BV(OCIE0A);
	}
	inline bool is_suspended()
	{
		return (volatile uint8_t&) TRAIT::TIMSK == 0;
	}
	inline void end()
	{
		synchronized _end();
	}
	inline void _end()
	{
		// Stop timer
		(volatile uint8_t&) TRAIT::TCCRB = 0;
		// Clear timer interrupt mode (set interrupt on OCRnA compare match)
		(volatile uint8_t&) TRAIT::TIMSK = 0;
	}
	
private:
	static constexpr uint32_t prescaler_quotient(TIMER_PRESCALER p, uint32_t us)
	{
		return (F_CPU / 1000000UL * us) / _BV(uint8_t(p));
	}

	static constexpr uint32_t prescaler_remainder(TIMER_PRESCALER p, uint32_t us)
	{
		return (F_CPU / 1000000UL * us) % _BV(uint8_t(p));
	}

	static constexpr bool prescaler_is_adequate(uint32_t quotient)
	{
		return quotient > 1 and quotient < TRAIT::MAX_COUNTER;
	}

	static constexpr TIMER_PRESCALER best_prescaler_in_2(TIMER_PRESCALER p1, TIMER_PRESCALER p2, uint32_t us)
	{
		return (!prescaler_is_adequate(prescaler_quotient(p1, us)) ? p2 :
				!prescaler_is_adequate(prescaler_quotient(p2, us)) ? p1 :
				prescaler_remainder(p1, us) < prescaler_remainder(p2, us) ? p1 :
				prescaler_remainder(p1, us) > prescaler_remainder(p2, us) ? p2 :
				prescaler_quotient(p1, us) > prescaler_quotient(p2, us) ? p1 : p2);
	}

	static constexpr TIMER_PRESCALER best_prescaler(const TIMER_PRESCALER* begin, const TIMER_PRESCALER* end, uint32_t us)
	{
		return (begin + 1 == end ? *begin : best_prescaler_in_2(*begin, best_prescaler(begin + 1 , end, us), us));
	}

	template<size_t N>
	static constexpr TIMER_PRESCALER best_prescaler(const TIMER_PRESCALER(&prescalers)[N], uint32_t us)
	{
		return best_prescaler(prescalers, prescalers + N, us);
	}
};

#endif /* TIMER_HH */
