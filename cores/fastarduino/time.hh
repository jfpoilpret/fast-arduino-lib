#ifndef TIME_HH
#define	TIME_HH

#include <stdint.h>
#include "utilities.hh"

namespace Time
{
	struct RTTTime
	{
		RTTTime(uint32_t millis = 0, uint16_t micros = 0):millis(millis), micros(micros) {}
		const uint32_t millis;
		const uint16_t micros;
	};

	void yield();
	RTTTime delta(const RTTTime& time1, const RTTTime& time2);
	uint32_t since(uint32_t start_ms);

	using DELAY_PTR = void (*)(uint32_t ms);
	extern DELAY_PTR delay;
	using MILLIS_PTR = uint32_t (*)();
	extern MILLIS_PTR millis;
	
	void delay_ms(uint16_t ms);
	void delay_us(uint16_t us);
	void default_delay(uint32_t ms);
	
	template<typename CLOCK>
	class ClockDelegate
	{
	public:
		static void set_clock(const CLOCK* clock)
		{
			_clock = clock;
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
		static CLOCK* _clock;
	};
	
	template<typename CLOCK>
	CLOCK* ClockDelegate<CLOCK>::_clock = 0;
	
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
