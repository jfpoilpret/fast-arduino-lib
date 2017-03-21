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

#ifndef TIME_HH
#define	TIME_HH

#include <stdint.h>
#include <util/delay_basic.h>

#include "utilities.h"

namespace time
{
	struct RTTTime
	{
		RTTTime(uint32_t millis = 0, uint16_t micros = 0):millis(millis), micros(micros) {}
		RTTTime(const RTTTime& that):millis{that.millis}, micros{that.micros} {}
		RTTTime& operator= (const RTTTime& that)
		{
			millis = that.millis;
			micros = that.micros;
			return *this;
		}
		uint32_t millis;
		uint16_t micros;
	};

	void yield();
	RTTTime delta(const RTTTime& time1, const RTTTime& time2);
	uint32_t since(uint32_t start_ms);

	using DELAY_PTR = void (*)(uint32_t ms);
	extern DELAY_PTR delay;
	using MILLIS_PTR = uint32_t (*)();
	extern MILLIS_PTR millis;
	
	inline void delay_us(uint16_t us) INLINE;
	inline void delay_us(uint16_t us)
	{
		_delay_loop_2((us * F_CPU) / 4000000L);
	}

	inline void delay_ms(uint16_t ms) INLINE;
	inline void delay_ms(uint16_t ms)
	{
		while (ms--) delay_us(1000);
	}

	void default_delay(uint32_t ms);
	
	template<typename CLOCK>
	class ClockDelegate
	{
		using TYPE = ClockDelegate<CLOCK>;
		
	public:
		static void set_clock(const CLOCK& clock, bool set_defaults = true)
		{
			_clock = &clock;
			if (set_defaults)
			{
				time::delay = TYPE::delay;
				time::millis = TYPE::millis;
			}
		}
		
		static void delay(uint32_t ms)
		{
			_clock->delay(ms);
		}
		
		static uint32_t millis()
		{
			return _clock->millis();
		}
		
	private:
		static const CLOCK* _clock;
	};
	
	template<typename CLOCK>
	const CLOCK* ClockDelegate<CLOCK>::_clock = 0;

	template<typename CLOCK>
	void set_clock(const CLOCK& clock, bool set_defaults = true)
	{
		time::ClockDelegate<CLOCK>::set_clock(clock, set_defaults);
	}
	
	class auto_delay
	{
	public:
		auto_delay(DELAY_PTR new_delay) INLINE:_old_delay{delay}
		{
			delay = new_delay;
		}
		~auto_delay() INLINE
		{
			delay = _old_delay;
		}
	private:
		const DELAY_PTR _old_delay;
	};

	class auto_millis
	{
	public:
		auto_millis(MILLIS_PTR new_millis) INLINE:_old_millis{millis}
		{
			millis = new_millis;
		}
		~auto_millis() INLINE
		{
			millis = _old_millis;
		}
	private:
		const MILLIS_PTR _old_millis;
	};
};

#endif	/* TIME_HH */
