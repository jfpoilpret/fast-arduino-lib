#ifndef TIME_HH
#define	TIME_HH

#include <stdint.h>

namespace Time
{
	void yield();
	extern void (*delay)(uint32_t ms);
	
	void default_delay(uint32_t ms);
};

#endif	/* TIME_HH */

