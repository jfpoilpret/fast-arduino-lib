//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * Useful defines GCC specific attributes.
 */
#ifndef DEFINES_HH
#define DEFINES_HH

/**
 * Specific GCC attribute to declare an argument or variable unused, so that the 
 * compiler does not emit any warning.
 * 
 * @code
 * static void set_mode(UNUSED PinMode mode, UNUSED bool value = false) {}
 * 
 * FastPin(PinMode mode UNUSED, bool value UNUSED = false) INLINE {}
 * 
 * static void constraints(T* p)
 * {
 *     UNUSED B* pb = p;
 * }
 * @endcode
 */
#define UNUSED __attribute__((unused))

/**
 * Specific GCC attribute to force the compiler to always inline code of a given
 * function.
 * 
 * @code
 * void set_PORT(uint8_t port) INLINE {...}
 * 
 * explicit inline I2CHandler(I2C_STATUS_HOOK hook) INLINE;
 * 
 * LinkedListImpl() INLINE = default;
 * @endcode
 */
#define INLINE __attribute__((always_inline))

/**
 * Specific GCC attribute for AVR target, declaring a function as a signal handler
 * (aka ISR, or Interrupt Service Routine).
 * 
 * You will never need to use it in your programs as all ISR for a target are declared
 * by FastArduino in header files in `fastarduino/board` directory and get automatically
 * included as soon as you use FastArduino in your project.
 * 
 * @code
 * extern "C" {
 *     void INT0_vect(void) SIGNAL;
 * }
 * @endcode
 */
#define SIGNAL __attribute__ ((signal))

/**
 * Specific GCC attribute for AVR target, declaring a signal handler (aka ISR, 
 * or Interrupt Service Routine) as an empty function. This is used when enabling
 * an interrupt only to awaken the MCU but with no further processing of the interrupt;
 * this ensures more compact code generation.
 * 
 * You will normally never need to use it in your programs as FastArduino provides
 * regsitration macros for empty ISR. You just have to call the proper macro(s)
 * once in your program.
 * 
 * @sa REGISTER_INT_ISR_EMPTY(INT_NUM, PIN)
 * @sa REGISTER_PCI_ISR_EMPTY(PCI_NUM, PIN, ...)
 * @sa REGISTER_TIMER_COMPARE_ISR_EMPTY(TIMER_NUM)
 * @sa REGISTER_TIMER_OVERFLOW_ISR_EMPTY(TIMER_NUM)
 * @sa REGISTER_TIMER_CAPTURE_ISR_EMPTY(TIMER_NUM)
 */
#define NAKED_SIGNAL __attribute__((signal, naked, __INTR_ATTRS))

/**
 * Specific GCC attribute to declare a function as weakly linked, which makes it
 * a default implementation that can be overwritten by simply redefining it without
 * that attribute.
 * 
 * @code
 * // In FastArduino main.cpp
 * int main() WEAK;
 * int main()
 * {
 *     return 0;
 * }
 * 
 * // In another source file: this code will replace the default code defined in main.cpp
 * int main()
 * {
 *     // Actual stuff...
 * }
 * @endcode
 */
#define WEAK __attribute__((weak))

#endif /* DEFINES_HH */
/// @endcond
