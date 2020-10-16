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
 * Common I2C Manager API. This will automatically include the proper header,
 * based on target architecture, ATmega or ATtiny.
 */
#ifndef I2C_HANDLER_HH
#define I2C_HANDLER_HH

#ifdef TWCR
#include "new_i2c_handler_atmega.h"
#else
#include "new_i2c_handler_attiny.h"
#endif

/**
 * This namespace defines everything related to I2C.
 * In FastArduino, I2C communication is centralized by an I2CManager; 
 * there are several flavors of I2CManager defined in FastArduino, with distinct
 * characteristics such as:
 * - synchronous (all MCU) or asynchronous (ATmega only)
 * - I2C mode supported (fast 400kHz or standard 100kHz)
 * - policy to follow in case of failure during an I2C transaction
 * - ...
 * 
 * I2C devices to connect with must be managed by a dedicated subclass of 
 * i2c::I2CDevice, which provides a specific API for the interfaced device, 
 * and handles all communication with an I2CManager.
 * 
 * For any I2C device subclass, the provided  API comes in 2 flavours at a time 
 * (whatever I2CManager is used):
 * - *asynchronous*: the API enqueues a chain of I2C commands for the underlying I2C
 *   transaction and lets the I2CManager handle these commands asynchronously if 
 *   possible (the I2CManager must support asynchronous operations); when really
 *   handled asynchronously, the API returns immediately, before the actual I2C 
 *   transaction is performed. Actual results will be returned through a 
 *   future::Future instance, passed as input argument of the API.
 * - *synchronous*: the API blocks until the complete underlying I2C transaction is
 *   complete. This API is implemented based on the asynchronous API above, but
 *   simply awaits for the Future result of the I2C transaction.
 * 
 * FastArduino defines many specific I2CManager classes among the following:
 * - I2CAsyncManager: bare bones asynchronous I2CManager 
 * - I2CAsyncLCManager: asynchronous I2CManager with lifecycle support
 * - I2CAsyncDebugManager: asynchronous I2CManager with a debug callback hook
 * - I2CAsyncStatusManager: asynchronous I2CManager with an I2C status callback hook
 * - I2CAsyncStatusDebugManager: asynchronous I2CManager with both an I2C status 
 *   callback hook and a debug callback hook
 * - I2CAsyncLCDebugManager: asynchronous I2CManager with lifecycle support and 
 *   a debug callback hook
 * - I2CAsyncLCStatusManager: asynchronous I2CManager with lifecycle support and 
 *   an I2C status callback hook
 * - I2CAsyncLCStatusDebugManager: asynchronous I2CManager with lifecycle support
 *   and both an I2C status callback hook and a debug callback hook
 * - I2CSyncManager: bare bones synchronous I2CManager
 * - I2CSyncLCManager: synchronous I2CManager with lifecycle support
 * - I2CSyncDebugManager: synchronous I2CManager with a debug callback hook
 * - I2CSyncStatusManager: synchronous I2CManager with an I2C status callback hook
 * - I2CSyncStatusDebugManager: synchronous I2CManager with both an I2C status 
 *   callback hook and a debug callback hook
 * - I2CSyncLCDebugManager: synchronous I2CManager with lifecycle support and 
 *   a debug callback hook
 * - I2CSyncLCStatusManager: synchronous I2CManager with lifecycle support and 
 *   an I2C status callback hook
 * - I2CSyncLCStatusDebugManager: synchronous I2CManager with lifecycle support
 *   and both an I2C status callback hook and a debug callback hook
 * 
 * All these classes are template classes with various arguments (the actual list
 * of arguments depends on each specific class):
 * - MODE: I2C mode (bus frequency) supported (fast 400kHz or standard 100kHz)
 * - POLICY: I2CPolicy (behavior in case of an error during a transaction)
 *   for asynchronous I2CManagers only
 * - DEBUG_HOOK: the type of callback hook for debug, can be a simple function 
 *   pointer (type I2C_DEBUG_HOOK) or a more complex functor class
 * - STATUS_HOOK: the type of callback hook for I2C status, can be a simple function 
 *   pointer (type I2C_STATUS_HOOK) or a more complex functor class
 * 
 * All these different flavors of I2CManager share the same API (except for their 
 * constructor that may need different arguments).
 * 
 * Lifecycle support enables programs to move futures around without losing track
 * of the right, thanks to the use of lifecycle:LightProxy. Although not often 
 * needed, it can prove useful in some situations.
 * 
 * All I2CManager asynchronous flavors operate based on a queue of I2C commands.
 * It is up to the end program to create the properly sized buffer for that 
 * command queue, before instantiating the relevant asynchronous I2CManager; the 
 * buffer must be passed to the asynchronous I2CManager constructor.
 * Asynchronous I2CManager classes will work fine only if the proper ISR function
 * is registered, through one of the 3 provided registration macros.
 * Some of these registration macros also allow registration of a callback hook
 * that will be called for every single I2C step (as defined in ATmega datasheet). 
 * 
 * The following snippet shows the minimal code to operate the I2C RTC device
 * DS1307 in synchronous mode:
 * @code
 * TODO I2C snippet
 * @endcode
 * 
 * The next snippet demonstrates how to do the same but in an asynchronous way:
 * @code
 * TODO I2C snippet
 * @endcode
 * 
 * @sa REGISTER_I2C_ISR()
 * @sa REGISTER_I2C_ISR_FUNCTION()
 * @sa REGISTER_I2C_ISR_METHOD()
 */
namespace i2c
{
}

#endif /* I2C_HANDLER_HH */

/// @endcond
