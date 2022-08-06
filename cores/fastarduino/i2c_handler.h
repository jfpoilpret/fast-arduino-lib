//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Common I2C Manager API. This will automatically include the proper header,
 * based on target architecture, ATmega or ATtiny.
 */
#ifndef I2C_HANDLER_HH
#define I2C_HANDLER_HH

// Thsi ehader is included only to get all registers (TWCR in particular) defined for the current target
#include <fastarduino/boards/board.h>

#ifdef TWCR
#include "i2c_handler_atmega.h"
#else
#include "i2c_handler_attiny.h"
#endif

/**
 * This namespace defines everything related to I2C.
 * In FastArduino, I2C communication is centralized by an I2C Manager; 
 * there are several flavors of I2C Manager defined in FastArduino, with distinct
 * characteristics such as:
 * - synchronous (all MCU) or asynchronous (ATmega only)
 * - I2C mode supported (fast 400kHz or standard 100kHz)
 * - policy to follow in case of failure during an I2C transaction
 * - ...
 * 
 * I2C devices to connect with must be managed by a dedicated subclass of 
 * i2c::I2CDevice, which provides a specific API for the interfaced device, 
 * and handles all communication with an I2C Manager.
 * 
 * For any I2C device subclass, the provided  API comes in 2 flavours at a time 
 * (whatever I2C Manager is used):
 * - *asynchronous*: the API enqueues a chain of I2C commands for the underlying I2C
 *   transaction and lets the I2C Manager handle these commands asynchronously if 
 *   possible (the I2C Manager must support asynchronous operations); when really
 *   handled asynchronously, the API returns immediately, before the actual I2C 
 *   transaction is performed. Actual results will be returned through a 
 *   future::Future instance, passed as input argument of the API.
 * - *synchronous*: the API blocks until the complete underlying I2C transaction is
 *   complete. This API is implemented based on the asynchronous API above, but
 *   simply awaits for the Future result of the I2C transaction.
 * 
 * FastArduino defines many specific I2C Manager classes among the following:
 * - I2CAsyncManager: bare bones asynchronous I2C Manager 
 * - I2CAsyncLCManager: asynchronous I2C Manager with lifecycle support
 * - I2CAsyncDebugManager: asynchronous I2C Manager with a debug callback hook
 * - I2CAsyncStatusManager: asynchronous I2C Manager with an I2C status callback hook
 * - I2CAsyncStatusDebugManager: asynchronous I2C Manager with both an I2C status 
 *   callback hook and a debug callback hook
 * - I2CAsyncLCDebugManager: asynchronous I2C Manager with lifecycle support and 
 *   a debug callback hook
 * - I2CAsyncLCStatusManager: asynchronous I2C Manager with lifecycle support and 
 *   an I2C status callback hook
 * - I2CAsyncLCStatusDebugManager: asynchronous I2C Manager with lifecycle support
 *   and both an I2C status callback hook and a debug callback hook
 * - I2CSyncManager: bare bones synchronous I2C Manager
 * - I2CSyncLCManager: synchronous I2C Manager with lifecycle support
 * - I2CSyncDebugManager: synchronous I2C Manager with a debug callback hook
 * - I2CSyncStatusManager: synchronous I2C Manager with an I2C status callback hook
 * - I2CSyncStatusDebugManager: synchronous I2C Manager with both an I2C status 
 *   callback hook and a debug callback hook
 * - I2CSyncLCDebugManager: synchronous I2C Manager with lifecycle support and 
 *   a debug callback hook
 * - I2CSyncLCStatusManager: synchronous I2C Manager with lifecycle support and 
 *   an I2C status callback hook
 * - I2CSyncLCStatusDebugManager: synchronous I2C Manager with lifecycle support
 *   and both an I2C status callback hook and a debug callback hook
 * 
 * All these classes are template classes with various arguments (the actual list
 * of arguments depends on each specific class):
 * - MODE: i2c::I2CMode (bus frequency) supported (fast 400kHz or standard 100kHz)
 * - POLICY: i2c::I2CErrorPolicy (behavior in case of an error during a transaction)
 *   for asynchronous I2C Managers only
 * - DEBUG_HOOK: the type of callback hook for debug, can be a simple function 
 *   pointer (type i2c::I2C_DEBUG_HOOK) or a more complex functor class
 * - STATUS_HOOK: the type of callback hook for I2C status, can be a simple function 
 *   pointer (type i2c::I2C_STATUS_HOOK) or a more complex functor class
 * 
 * All these different flavors of I2C Manager share the same API (except for their 
 * constructor that may need different arguments).
 * 
 * Lifecycle support enables programs to move futures around without losing track
 * of the right location, thanks to the use of lifecycle::LightProxy. Although not often 
 * needed, it can prove useful in some situations.
 * 
 * All I2C Manager asynchronous flavors operate based on a queue of I2C commands.
 * It is up to the end program to create the properly sized buffer for that 
 * command queue, before instantiating the relevant asynchronous I2C Manager; the 
 * buffer must be passed to the asynchronous I2C Manager constructor.
 * Asynchronous I2C Manager classes will work fine only if the proper ISR function
 * is registered, through one of the 3 provided registration macros.
 * Some of these registration macros also allow registration of a callback hook
 * that will be called for every single I2C step (as defined in ATmega datasheet). 
 * 
 * The following snippet shows the minimal code to operate the I2C RTC device
 * DS1307 in synchronous mode:
 * @code
 * // Define type alias for I2C Manager; here we use the simplest possible synchronous manager
 * using MANAGER = i2c::I2CSyncManager<i2c::I2CMode::STANDARD>;
 * using RTC = DS1307<MANAGER>;
 * ...
 * // Instantiate and start I2C Manager
 * MANAGER manager;
 * manager.begin();
 * // Instantiate the DS1307 RTC device
 * RTC rtc{manager};
 * 
 * // Call specific DS1307 API to get current date
 * tm now;
 * rtc.get_datetime(now);
 * @endcode
 * 
 * The next snippet demonstrates how to do the same but in an asynchronous way:
 * @code
 * // Define type alias for I2C Manager; here we use the simplest possible asynchronous manager
 * using MANAGER = i2c::I2CAsyncManager<i2c::I2CMode::STANDARD>;
 * using RTC = DS1307<MANAGER>;
 * 
 * // Define a buffer for the I2C Manager commands queue
 * static constexpr uint8_t I2C_BUFFER_SIZE = 32;
 * static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
 * 
 * // Register I2C ISR to allow asynchronous operation with the I2C Manager
 * REGISTER_I2C_ISR(MANAGER)
 * ...
 * // Instantiate and start I2C Manager
 * MANAGER manager{i2c_buffer};
 * manager.begin();
 * // Instantiate the DS1307 RTC device
 * RTC rtc{manager};
 * 
 * // Prepare Future to receive current date
 * RTC::GetDatetimeFuture future;
 * // Call specific DS1307 API to get current date
 * int error = rtc.get_datetime(future);
 * // Check error here (should be 0)...
 * 
 * // When needed, get the Future result (await if needed)
 * tm now;
 * bool ok = get_date_future.get(now);
 * @endcode
 * 
 * @sa REGISTER_I2C_ISR()
 * @sa REGISTER_I2C_ISR_FUNCTION()
 * @sa REGISTER_I2C_ISR_METHOD()
 */
namespace i2c
{
	/// @cond notdocumented
	// Specific traits for I2C Async Managers (both ATmega and ATtiny)
	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncManager<MODE_>>
		:	I2CManager_trait_impl<false, false, false, false, MODE_> {};

	template<I2CMode MODE_>
	struct I2CManager_trait<I2CSyncLCManager<MODE_>>
		:	I2CManager_trait_impl<false, true, false, false, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_>
	struct I2CManager_trait<I2CSyncStatusManager<MODE_, STATUS_HOOK_>>
		:	I2CManager_trait_impl<false, false, true, false, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, false, false, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncStatusDebugManager<MODE_, STATUS_HOOK_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, false, true, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_>
	struct I2CManager_trait<I2CSyncLCStatusManager<MODE_, STATUS_HOOK_>>
		:	I2CManager_trait_impl<false, true, true, false, MODE_> {};

	template<I2CMode MODE_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncLCDebugManager<MODE_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, true, false, true, MODE_> {};

	template<I2CMode MODE_, typename STATUS_HOOK_, typename DEBUG_HOOK_>
	struct I2CManager_trait<I2CSyncLCStatusDebugManager<MODE_, STATUS_HOOK_, DEBUG_HOOK_>>
		:	I2CManager_trait_impl<false, true, true, true, MODE_> {};
	/// @endcond
}

#endif /* I2C_HANDLER_HH */

/// @endcond
