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

/// @cond api

/**
 * @file 
 * General API for handling AVR interrupt vectors.
 * In FastArduino, ISR management is performed in 1 or 2 steps:
 * 
 * 1. Declare the ISR itself, in your program, by using one of `REGISTER_XXX` macros
 * provided by FastArduino API; these macros always come in 2 flavours, described later.
 * These macros *define* the ISR function and perform a callback to your code.
 * 2. If you used a `REGISTER_XXX_ISR_METHOD` macro flavour, then you need to register
 * the instance of the class that contains the callback method; this is done with
 * `interrupt::register_handler()`. This method may sometimes be wrapped inside FastArduino
 * API, hence you do not always need to call it yourself.
 * 
 * There are 2 flavours of each registration macro:
 * 
 * 1. `REGISTER_XXX_ISR_FUNCTION`: this flavour defines an ISR for the *"XXX"* signal
 * with callback to a global (or static) function defined in your own program. This may 
 * be useful in simple situations where your callback function does not need any
 * context to perform its task; if your function needs to access some context, then
 * this can only be achieved with global volatile variables, which is not a recommended
 * practice.
 * 2. `REGISTER_XXX_ISR_METHOD`: this flavour defines an ISR for the *"XXX"* signal
 * with callback to a member function of a class. The instance of this class must later
 * be registered with `interrupt::register_handler()` so that the defined ISR can find
 * it when it gets executed.
 * 
 * Note that some FastArduino API define a 3rd flavour of registration macro where
 * you don't need to specify a callback function or method, because this callback 
 * is implicitly defined by the API, e.g. `REGISTER_WATCHDOG_CLOCK_ISR()` defines
 * the necessary ISR for `watchdog::Watchdog` to be notified of watchdog timeouts
 * so that it can update its internal clock counter and generate events.
 * 
 * The rationale behind this approach to register ISR and clallbacks is based on the 
 * following principles:
 * - you decide what ISR you want to use, FastArduino will not impose it to you
 * - you may override FastArduino default ISR if you need to
 * - ISR callbacks may be any function or any method of any class: FastArduino will 
 * not force you to subclass one of its own classes in order to override some 
 * `virtual` method. Calling `virtual` methods from an ISR has an impact on the
 * size and speed of this ISR (all AVR registers must be pushed to the stack even
 * though only a few of them may be used by the overridden method).
 * 
 * Most macros defined in this file are normally not used directly by your programs,
 * they are low-level and used by other higher-level macros defined in each FastArduino
 * specific APIs.
 * These macros are documented for the sake of completeness and in the rare cases where
 * you might need them.
 */
#ifndef INTERRUPTS_HH
#define	INTERRUPTS_HH

#include <avr/interrupt.h>

/// @cond notdocumented
// Macro found on https://tty1.net/blog/2008/avr-gcc-optimisations_en.html
// This allows processing pointers to SRAM data be performed directly from Y, Z registers
// this may optimize code size on some circumstances
#define FIX_BASE_POINTER(_ptr) __asm__ __volatile__("" : "=b" (_ptr) : "0" (_ptr))

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
/// @endcond

/**
 * Define an ISR for @p VECTOR; this ISR will simply call the @p CALLBACK method
 * of @p HANDLER class.
 * Note that a proper instance needs to be first registered with
 * `interrupt::register_handler()` before the first call to this ISR.
 * This macro is normally used only by FastArduino to define higher-level macros 
 * for registration of specific ISR; you normally won't need to use this macro in 
 * your own programs.
 * @param VECTOR the name of the interrupt vector for which to generate the ISR;
 * must exist for the current AVR target.
 * @param HANDLER the class which registered instance will be used to call 
 * @p CALLBACK method when the ISR is called; this class must have been defined
 * **before** the macro is used.
 * @param CALLBACK the @p HANDLER method that will be called back by the ISR;
 * must be a proper Pointer to Member Function of @p HANDLER. This method takes 
 * no argument.
 * @sa interrupt::register_handler()
 */
#define REGISTER_ISR_METHOD_(VECTOR, HANDLER, CALLBACK)				\
ISR(VECTOR)															\
{																	\
	CALL_HANDLER_(SINGLE_ARG2_(HANDLER), SINGLE_ARG2_(CALLBACK))();	\
}

/**
 * Define an ISR for @p VECTOR; this ISR will simply call the @p CALLBACK function.
 * This macro is normally used only by FastArduino to define higher-level macros 
 * for registration of specific ISR; you normally won't need to use this macro in 
 * your own programs.
 * @param VECTOR the name of the interrupt vector for which to generate the ISR;
 * must exist for the current AVR target.
 * @param CALLBACK the global or static function that will be called back by the ISR;
 * this function takes no argument. This function must have been defined (or at 
 * least declared) **before** the macro is used.
 */
#define REGISTER_ISR_FUNCTION_(VECTOR, CALLBACK)	\
ISR(VECTOR)											\
{													\
	CALLBACK ();									\
}

/**
 * Defines API to handle AVR interruptions.
 * In particular, the following API are provided:
 * - generically handle interrupts callbacks
 * - handle external interrupt pins
 * - handle pin change interrupts
 */
namespace interrupt
{
	/// @cond notdocumented
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

		//FIXME rename Hodler -> Holder
		//TODO allow for callbacks that return something
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
	/// @endcond

	/**
	 * Register a class instance containing methods that shall be called back by an ISR.
	 * The class and member function shall be passed to one of `REGISTER_XXXX_ISR_METHOD()`
	 * macros proposed by various FastArduino API, e.g. `REGISTER_WATCHDOG_ISR_METHOD()`.
	 * Note that you can register different classes, but only one instance of a given class.
	 * Also, one class may have different methods to handle different ISR callbacks.
	 * 
	 * @tparam Handler the class containing callback methods
	 * @param handler the @p Handler instance which methods will be called back by 
	 * registered ISR.
	 */
	template<typename Handler>
	void register_handler(Handler& handler)
	{
		HandlerHolder<Handler>::_handler = &handler;
	}
}

/// @cond notdocumented

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
/// @endcond

#endif	/* INTERRUPTS_HH */
/// @endcond
