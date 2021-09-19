//   Copyright 2016-2021 Jean-Francois Poilpret
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
#define INTERRUPTS_HH

#include "boards/io.h"
#include <avr/interrupt.h>

/// @cond notdocumented
// Macro found on https://tty1.net/blog/2008/avr-gcc-optimisations_en.html
// This allows processing pointers to SRAM data be performed directly from Y, Z registers
// this may optimize code size on some circumstances
#define FIX_BASE_POINTER(_ptr) __asm__ __volatile__("" : "=b"(_ptr) : "0"(_ptr))
/// @endcond

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
	template<typename Handler> class HandlerHolder
	{
	public:
		static Handler* handler()
		{
			return handler_;
		}

		using Holder = HandlerHolder<Handler>;

		template<typename Ret, typename... Args> struct ArgsHolder
		{
			template<Ret (Handler::*Callback)(Args...)> struct CallbackHolder
			{
				static Ret handle(Args... args)
				{
					Handler* handler_instance = Holder::handler();
					FIX_BASE_POINTER(handler_instance);
					return (handler_instance->*Callback)(args...);
				}
			};
		};

	private:
		static Handler* handler_;
		friend void register_handler<Handler>(Handler&);
	};

	template<typename Handler> Handler* HandlerHolder<Handler>::handler_ = nullptr;

	// Used by ISR to perform a callback to a PTMF
	// Found great inspiration for this pattern there:
	// https://stackoverflow.com/questions/9779105/generic-member-function-pointer-as-a-template-parameter
	template<typename T, T> struct CallbackHandler;
	template<typename HANDLER, typename RET, typename... ARGS, RET (HANDLER::*CALLBACK)(ARGS...)>
	struct CallbackHandler<RET (HANDLER::*)(ARGS...), CALLBACK>
	{
		static RET call(ARGS... args)
		{
			// NOTE the following line does not compile, it must be broken down for the compiler to understand
			// return HandlerHolder<HANDLER>::ArgsHolder<RET, ARGS...>::CallbackHolder<CALLBACK>::handle(args...);
			using HOLDER = HandlerHolder<HANDLER>;
			using ARGS_HOLDER = typename HOLDER::template ArgsHolder<RET, ARGS...>;
			using CALLBACK_HOLDER = typename ARGS_HOLDER::template CallbackHolder<CALLBACK>;
			return CALLBACK_HOLDER::handle(args...);
		}
	};
	template<typename RET, typename... ARGS, RET (*CALLBACK)(ARGS...)>
	struct CallbackHandler<RET (*)(ARGS...), CALLBACK>
	{
		static RET call(ARGS... args)
		{
			return CALLBACK(args...);
		}
	};
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
	template<typename Handler> void register_handler(Handler& handler)
	{
		HandlerHolder<Handler>::handler_ = &handler;
	}
}

/// @cond notdocumented

// Macro often used to build vector names from parameters
#define CAT3(X, Y, Z) X##Y##Z

/// @endcond

#endif /* INTERRUPTS_HH */
/// @endcond
