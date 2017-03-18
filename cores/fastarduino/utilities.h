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

#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

#define synchronized \
_Pragma ("GCC diagnostic ignored \"-Wreturn-type\"") \
ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

// Macro found on https://tty1.net/blog/2008/avr-gcc-optimisations_en.html
// This allows processing pointers to SRAM data be performed directly from Y, Z registers
// this may optimize code size on some circumstances
#define FIX_BASE_POINTER(_ptr) __asm__ __volatile__("" : "=b" (_ptr) : "0" (_ptr))

namespace utils
{
	template<typename T>
	constexpr T constrain(T value, T min, T max)
	{
		return value < min ? min : value > max ? max : value;
	}
	template<typename TI, typename TO>
	constexpr TO map(TI value, TI input_range, TO output_min, TO output_max)
	{
		return TO (value * (output_max - output_min) / input_range + output_min);
	}
	template<typename TI, typename TO>
	constexpr TO map(TI value, TI input_min, TI input_max, TO output_min, TO output_max)
	{
		return map(value, input_max - input_min, output_min, output_max);
	}

	constexpr uint16_t as_uint16_t(uint8_t high, uint8_t low)
	{
		return (high << 8) | low;
	}

	template<typename T>
	constexpr T is_zero(T value, T default_value)
	{
		return (value ? value : default_value);
	}

	template<typename T>
	void set_mask(volatile T& reg, T mask, T value)
	{
		reg = (reg & ~mask) | (value & mask);
	}
}

// Useful macros to pass arguments containing a comma
#define AS_ONE_ARG(...) __VA_ARGS__
#define SINGLE_ARG1_(...) __VA_ARGS__
#define SINGLE_ARG2_(...) __VA_ARGS__
#define SINGLE_ARG3_(...) __VA_ARGS__

// Utilities to handle ISR callbacks
#define HANDLER_HOLDER_(HANDLER) interrupt::HandlerHolder< HANDLER >

#define CALLBACK_HANDLER_HOLDER_(HANDLER, CALLBACK,...)	\
interrupt::HandlerHolder< HANDLER >::ArgsHodler< __VA_ARGS__ >::CallbackHolder< CALLBACK >

#define CALL_HANDLER_(HANDLER, CALLBACK,...)	\
CALLBACK_HANDLER_HOLDER_(SINGLE_ARG1_(HANDLER), SINGLE_ARG1_(CALLBACK), ##__VA_ARGS__)::handle

#define REGISTER_ISR_METHOD_(VECTOR, HANDLER, CALLBACK)	\
ISR(VECTOR)												\
{														\
	CALL_HANDLER_(SINGLE_ARG2_(HANDLER), SINGLE_ARG2_(CALLBACK))();				\
}

#define REGISTER_ISR_FUNCTION_(VECTOR, CALLBACK)	\
ISR(VECTOR)											\
{													\
	CALLBACK ();									\
}

namespace interrupt
{
	template<typename Handler> void register_handler(Handler&);
	template<typename Handler>
	class HandlerHolder
	{
	public:
		static Handler* handler()
		{
			return _handler;
		}

		using Holder = HandlerHolder<Handler>;

		template<typename... Args>
		struct ArgsHodler
		{
			template<void (Handler::*Callback)(Args...)>
			struct CallbackHolder
			{
				static void handle(Args... args)
				{
					Handler* handler_instance = Holder::handler();
					FIX_BASE_POINTER(handler_instance);
					(handler_instance->*Callback)(args...);
				}
			};
		};

	private:
		static Handler* _handler;
		friend void register_handler<Handler>(Handler&);
	};

	template<typename Handler>
	Handler* HandlerHolder<Handler>::_handler = 0;

	template<typename Handler>
	void register_handler(Handler& handler)
	{
		HandlerHolder<Handler>::_handler = &handler;
	}
}

// Useful macro to iterate
// NOTE: these macros have been inspired by several readings:
// http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// http://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms

#define EMPTY(...)
#define CAT(X, Y) X ## Y
#define CAT3(X, Y, Z) X ## Y ## Z
#define COMMA() ,
#define STRINGIFY(X, ...) #X
#define ID(X, ...) X

#define FE_0_(M, DATA, FIRST, SEP, LAST) 
#define FE_1_(M, DATA, FIRST, SEP, LAST, X1) FIRST() M(X1, DATA) LAST()
#define FE_2_(M, DATA, FIRST, SEP, LAST, X1, X2) FIRST() M(X1, DATA) SEP() M(X2, DATA) LAST()
#define FE_3_(M, DATA, FIRST, SEP, LAST, X1, X2, X3) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) LAST()
#define FE_4_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4) FIRST()  M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) LAST()
#define FE_5_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) LAST()
#define FE_6_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) LAST()
#define FE_7_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) LAST()
#define FE_8_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7, X8) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) SEP() M(X8, DATA) LAST()
#define FE_9_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7, X8, X9) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) SEP() M(X8, DATA) SEP() M(X9, DATA) LAST()

#define GET_MACRO_9_(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, NAME,...) NAME 

// FOR_EACH executes M macro for each argument passed to it, returns empty if passed an empty list.
// Number of arguments in variadic list must be between 0 and 9
#define FOR_EACH(M, DATA, ...) GET_MACRO_9_(unused, ##__VA_ARGS__, FE_9_, FE_8_, FE_7_, FE_6_, FE_5_, FE_4_, FE_3_, FE_2_, FE_1_, FE_0_)(M, DATA, EMPTY, EMPTY, EMPTY, ## __VA_ARGS__)
// FOR_EACH_SEP executes M macro for each argument passed to it, separates each transformed value with SEP(),
// prepends FIRST() and appends LAST() if result list is not empty; returns empty if passed an empty list
// Number of arguments in variadic list must be between 0 and 9
#define FOR_EACH_SEP(M, DATA, FIRST, SEP, LAST, ...) GET_MACRO_9_(unused, ##__VA_ARGS__, FE_9_, FE_8_, FE_7_, FE_6_, FE_5_, FE_4_, FE_3_, FE_2_, FE_1_, FE_0_)(M, DATA, FIRST, SEP, LAST, ##__VA_ARGS__)

#endif	/* UTILITIES_HH */
