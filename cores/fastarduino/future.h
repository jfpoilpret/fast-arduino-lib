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
 * Utility API to handle the concept of futures.
 * For general discussion about this concept, please check 
 * https://en.wikipedia.org/wiki/Futures_and_promises
 */
#ifndef FUTURE_HH
#define FUTURE_HH

#include <string.h>
#include "errors.h"
#include "move.h"
#include "time.h"
#include "streams.h"

/**
 * Contains the API around Future implementation.
 * A Future allows you to pass and get values across different units of executions
 * (threads, or more likely on AVR MCU, the main program and an ISR).
 * 
 * Concepts applied in this API:
 * - A Future holds a buffer for a future "output" value (any type, even `void`, 
 * i.e. no value)
 * - A Future may also hold a storage "input" value (constant, any type) with 
 * same lifetime as the Future
 * - Each Future is identified by a unique ID
 * - A Future is either:
 *      - Invalid: it is not linked to anything and is unusable; this happens
 *        in several circumstances: default construction, instance move, value (or 
 *        error) set and already read once
 *      - Not ready: its value has not been obtained yet
 *      - Ready: its value has been fully set and not yet read by anyone
 *      - Error: an error occurred in the provider, hence no value will ever be 
 *        held by this Future, the actual error has not yet been read by anyone
 * - Once invalid, a Future becomes useless, unless re-assigned with a newly 
 * constructed Future
 * - A `FutureManager` centralizes lifetime of all Futures
 * - There can be only one FutureManager in the system (singleton)
 * - The FutureManager holds up-to-date pointers to each valid Future
 * - Maximum number of Futures is statically defined at build time
 * - Futures notify their lifetime to FutureManager (moved, deleted, inactive)
 * - Futures ID are used as an index into an internal FutureManager table, 0 means 
 * "not registered yet" or "invalid" after moving to another Future instance
 * - Output value providers must know the ID in order to fill up values 
 * (or errors) of a Future, through the FutureManager (only the FutureManager 
 * knows exactly where each Future stands)
 * - Storage input value consumers must know the ID in order to get storage value
 * of a Future, through the FutureManager (only the FutureManager knows exactly
 * where each Future stands)
 * - It is possible to subclass a Future to add last minute transformation on 
 * `Future.get()` method
 * - The FutureManager tries to limit potential conflicts when assigning an ID
 * during Future registration, by searching for an available ID AFTER the last ID
 * removed; this may not be sufficient: it is possible (although a well-written 
 * program should never do that) that a NOT_READY Future gets destructed and its 
 * output value provider tries to fill its value, since the provider only gets the 
 * ID, if the same ID has been assigned to a new Future, a conflict may occur and 
 * possibly lead to a crash.
 * 
 * Here is a simple usage example of this API, using PCINT0 interrupt to take a 
 * "snapshot" of 6 input buttons connected to all PORTB pins, after one of them has
 * changed level:
 * @code
 * // global variable holding the id of the future to fill in
 * static uint8_t portB_snapshot_id = 0;
 * // PCINT0 ISR
 * void take_snapshot() {
 *     gpio::FastPort<board::Port::PORT_B> port;
 *     future::AbstractFutureManager::instance().set_future_value(portB_snapshot_id, port.get_PIN());
 * }
 * REGISTER_PCI_ISR_FUNCTION(0, take_snapshot, board::InterruptPin::D8_PB0_PCI0)
 * ...
 * 
 * // Within main()
 * // First create a FutureManager singleton with max 16 futures
 * static constexpr uint8_t MAX_FUTURES = 16;
 * future::FutureManager<MAX_FUTURES> manager;
 * // Initialize PORTB and PCI
 * gpio::FastPort<board::Port::PORT_B> port{0xFF, 0xFF};
 * interrupt::PCI_PORT_SIGNAL<board::Port::PORT_B> signal;
 * signal.set_enable_pins(0xFF);
 * signal.enable();
 * ...
 * while (true) {
 *     // Create a Future and register it
 *     future::Future<uint8_t> portB_snapshot;
 *     manager.register_future(portB_snapshot);
 *     portB_snapshot_id = portB_snapshot.id();
 *     ...
 *     // Wait for the future to be filled in by the PCINT0 ISR
 *     uint8_t snapshot = 0;
 *     if (portB_snapshot.get(snapshot)) {
 *         // Do something with the obtained snapshot
 *     }
 * }
 * ...
 * @endcode
 */
namespace future
{
	/// @cond notdocumented
	// Forward declarations
	class AbstractFuture;
	template<typename OUT, typename IN> class Future;
	/// @endcond

	/**
	 * This is the parent of `FutureManager` and it provides all `FutureManager` API.
	 * You should normally never need to subclass it.
	 * This is a singleton, in the sense there can be only one `AbstractFutureManager`
	 * active at a time.
	 * An `AbstractFutureManager` holds a limited number of registered `Future`s;
	 * the limit is fixed at construction time of `FutureManager`.
	 * 
	 * @sa FutureManager
	 * @sa Future
	 */
	class AbstractFutureManager
	{
	public:
		/**
		 * Return the single instance of `AbstractFutureManager`, or `null` if no 
		 * `FutureManager` has been instantiated yet.
		 */
		static AbstractFutureManager& instance()
		{
			return *instance_;
		}

		/**
		 * Register a newly instantiated `AbstractFuture` with this `AbstractFutureManager`.
		 * @warning DO NOT USE THIS METHOD! This method was made `public` because
		 * no better way could be found to make it visible to other parts FastArduino API.
		 * But you should never use it in your own programs.
		 * You should use `AbstractFutureManager.register_future(Future<OUT, IN>&)` instead.
		 * 
		 * @sa AbstractFutureManager.register_future(Future<OUT, IN>&)
		 */
		bool register_future(AbstractFuture& future)
		{
			synchronized return register_future_(future);
		}

		/**
		 * Register a newly instantiated `AbstractFuture` with this `AbstractFutureManager`.
		 * @warning DO NOT USE THIS METHOD! This method was made `public` because
		 * no better way could be found to make it visible to other parts FastArduino API.
		 * But you should never use it in your own programs.
		 * You should use `AbstractFutureManager.register_future_(Future<OUT, IN>&)` instead.
		 * 
		 * @sa AbstractFutureManager.register_future_(Future<OUT, IN>&)
		 */
		bool register_future_(AbstractFuture& future);

		/**
		 * Register a newly instantiated Future with this `AbstractFutureManager`.
		 * A Future is useless until it has been registered.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.register_future_()` instead.
		 * 
		 * @tparam OUT the output (result value) type that @p future holds
		 * @tparam IN the input (storage value) type that @p future holds
		 * @param future a reference to a newly constructed Future that we want to register
		 * in order to use it as expected
		 * @retval true if @p future has been successfully registered by this `AbstractFutureManager`
		 * @retval false if @p future could not be registered by this `AbstractFutureManager`,
		 * either because the limit of registrable `Future`s has been reached already, or
		 * because @p future is already registered.
		 * 
		 * @sa Future
		 * @sa AbstractFutureManager.register_future_()
		 */
		template<typename OUT, typename IN> bool register_future(Future<OUT, IN>& future)
		{
			return register_future((AbstractFuture&) future);
		}

		/**
		 * Register a newly instantiated Future with this `AbstractFutureManager`.
		 * A Future is useless until it has been registered.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.register_future()` instead.
		 * 
		 * @tparam OUT the output (result value) type that @p future holds
		 * @tparam IN the input (storage value) type that @p future holds
		 * @param future a reference to a newly constructed Future that we want to register
		 * in order to use it as expected
		 * @retval true if @p future has been successfully registered by this `AbstractFutureManager`
		 * @retval false if @p future could not be registered by this `AbstractFutureManager`,
		 * either because the limit of registrable `Future`s has been reached already, or
		 * because @p future is already registered.
		 * 
		 * @sa Future
		 * @sa AbstractFutureManager.register_future()
		 */
		template<typename OUT, typename IN> bool register_future_(Future<OUT, IN>& future)
		{
			return register_future_((AbstractFuture&) future);
		}

		/**
		 * Return the number of available `Future`s in this `AbstractFutureManager`.
		 * This means the maximum number of `Future`s that can be registered if no
		 * Future already registered get destroyed.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.available_futures_()` instead.
		 * 
		 * @sa register_future()
		 * @sa Future
		 * @sa available_futures_()
		 */
		uint8_t available_futures() const
		{
			synchronized return available_futures_();
		}

		/**
		 * Return the number of available `Future`s in this `AbstractFutureManager`.
		 * This means the maximum number of `Future`s that can be registered if no
		 * Future already registered get destroyed.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.available_futures()` instead.
		 * 
		 * @sa register_future_()
		 * @sa Future
		 * @sa available_futures()
		 */
		uint8_t available_futures_() const
		{
			uint8_t free = 0;
			for (uint8_t i = 0; i < size_; ++i)
				if (futures_[i] == nullptr)
					++free;
			return free;
		}

		/**
		 * Check the number of bytes remaining to write to the output value of
		 * a Future identified by @p id.
		 * This method is called by a Future output value producer to know how many
		 * bytes remain to write until the end of the output value.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.get_future_value_size_()` instead.
		 * 
		 * @param id the unique id of the Future to query
		 * @return the number of bytes to be written to the output value stored
		 * in Future identified by id @p id
		 * 
		 * @sa get_future_value_size_()
		 */
		uint8_t get_future_value_size(uint8_t id) const
		{
			synchronized return get_future_value_size_(id);
		}

		/**
		 * Mark the Future identified by @p id as `FutureStatus::READY`.
		 * This method is called by a Future ouput value provider to indicate
		 * that a Future is ready for use.
		 * This method is useful only for `Future<void>` i.e. `Future`s that have 
		 * no output, but exist as way to indicate the end of an asynchronous 
		 * process. For other `Future<T>`s, with a non `void` type `T`, you should
		 * use one of `set_future_value` methods instead.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.set_future_finish_()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked ready
		 * @retval false if there is no Future with id @p id, or if the related 
		 * Future cannot be updated properly (because it is not in
		 * `FutureStatus::NOT_READY` or if it is still expecting data)
		 * @retval true if the matching Future has been properly updated
		 * 
		 * @sa Future.status()
		 * @sa set_future_value()
		 * @sa set_future_error()
		 * @sa set_future_finish_()
		 */
		bool set_future_finish(uint8_t id) const
		{
			synchronized return set_future_finish_(id);
		}

		/**
		 * Add one byte to the output value content of the Future identified 
		 * by @p id.
		 * This method is called by a Future ouput value provider to fill up,
		 * byte after byte, the output value of a Future.
		 * Calling this method may change the status of the Future to `FutureStatus::READY`
		 * if this is the last output value byte to be filled for this Future.
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * You should not use it for a `Future<void>` instance.
		 * 
		 * It is also possible to fill the output value by larger chunks, with
		 * other overloaded versions of this method.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.set_future_value_(uint8_t, uint8_t) const` instead.
		 * 
		 * @param id the unique id of the Future which output value shall be filled up
		 * @param chunk the byte to append to this Future output value
		 * @retval true if @p chunk could be added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa Future.status()
		 * @sa set_future_value(uint8_t, const uint8_t*, uint8_t) const
		 * @sa set_future_value(uint8_t, const T&) const
		 * @sa set_future_value_(uint8_t, uint8_t) const
		 */
		bool set_future_value(uint8_t id, uint8_t chunk) const
		{
			synchronized return set_future_value_(id, chunk);
		}

		/**
		 * Add several bytes to the output value content of the Future identified 
		 * by @p id.
		 * This method is called by a Future ouput value provider to fill up,
		 * with a chunk of bytes, the output value of a Future.
		 * Calling this method may change the status of the Future to `FutureStatus::READY`
		 * if this is the last output value chunk to be filled for this Future.
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * You should not use it for a `Future<void>` instance.
		 * 
		 * It is also possible to fill the output value byte per byte, with
		 * other overloaded versions of this method.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.set_future_value_(uint8_t, const uint8_t*, uint8_t) const`
		 * instead.
		 * 
		 * @param id the unique id of the Future which output value shall be filled up
		 * @param chunk pointer to the first byte to be added to this Future output
		 * value
		 * @param size the number of bytes to be appended to this Future output value
		 * @retval true if @p chunk could be completely added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`, or when @p size
		 * additional bytes would make the output value bigger than expected
		 * 
		 * @sa Future.status()
		 * @sa set_future_value(uint8_t, uint8_t) const
		 * @sa set_future_value(uint8_t, const T&) const
		 * @sa set_future_value_(uint8_t, const uint8_t*, uint8_t) const
		 */
		bool set_future_value(uint8_t id, const uint8_t* chunk, uint8_t size) const
		{
			synchronized return set_future_value_(id, chunk, size);
		}

		/**
		 * Set the output value content of the Future identified by @p id.
		 * This method is called by a Future ouput value provider to fully fill
		 * up, with the proper value, the output value of a Future.
		 * Calling this method will change the status of the Future to 
		 * `FutureStatus::READY`
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * 
		 * It is also possible to fill the output value byte per byte, with
		 * other overloaded versions of this method.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.set_future_value_(uint8_t, const T&) const` instead.
		 * 
		 * @tparam T the type of output value of the Future indetified by @p id
		 * 
		 * @param id the unique id of the Future which output value shall be set
		 * @param value a constant reference to the value to set in the target Future
		 * @retval true if @p value could be properly set into the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa Future.status()
		 * @sa set_future_value(uint8_t, uint8_t) const
		 * @sa set_future_value(uint8_t, const uint8_t*, uint8_t) const
		 * @sa set_future_value_(uint8_t, const T&) const
		 */
		template<typename T> bool set_future_value(uint8_t id, const T& value) const
		{
			synchronized return set_future_value_(id, value);
		}

		/**
		 * Mark the Future identified by @p id as `FutureStatus::ERROR`.
		 * This method is called by a Future ouput value provider to indicate
		 * that it cannot compute an output value for a given Future.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.set_future_error_()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked in error
		 * @param error the error code to set for the Future
		 * @retval true if the matching Future has been properly updated
		 * @retval false if there is no Future with id @p id, or if the related 
		 * Future cannot be updated properly (because it is not in
		 * `FutureStatus::NOT_READY`)
		 * 
		 * @sa Future.status()
		 * @sa set_future_value()
		 * @sa set_future_finish()
		 * @sa set_future_error_()
		 */
		bool set_future_error(uint8_t id, int error) const
		{
			synchronized return set_future_error_(id, error);
		}

		/**
		 * Check the number of bytes remaining to read from a Future identified
		 * by @p id.
		 * This method is called by a Future input value consumer to know how many
		 * bytes remain to get read until the end of the input value.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.get_storage_value_size_()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked ready
		 * @return the number of bytes to be read from the input value stored
		 * by Future identified by id @p id
		 * 
		 * @sa get_storage_value_size_()
		 */
		uint8_t get_storage_value_size(uint8_t id) const
		{
			synchronized return get_storage_value_size_(id);
		}

		/**
		 * Get one byte from the input storage value of the Future identified 
		 * by @p id.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next byte of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.get_storage_value_()` instead.
		 * 
		 * @param id the unique id of the Future which input storage value to get
		 * @param chunk the byte reference that will receive the next byte of this
		 * Future input value
		 * @retval true if the next byte of this Future could be read successfully
		 * @retval false if there is no Future with id @p id, or if all bytes of 
		 * its input storage value have been read already
		 * 
		 * @sa Future::Future(const IN&)
		 * @sa Future.status()
		 * @sa get_storage_value(uint8_t, uint8_t*, uint8_t) const
		 * @sa get_storage_value_(uint8_t, uint8_t&) const
		 */
		bool get_storage_value(uint8_t id, uint8_t& chunk) const
		{
			synchronized return get_storage_value_(id, chunk);
		}

		/**
		 * Get @p size bytes from the input storage value of the Future identified 
		 * by @p id.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next chunk of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractFutureManager.get_storage_value_()` instead.
		 * 
		 * @param id the unique id of the Future which input storage value to get
		 * @param chunk a pointer to an array of at least @p size bytes, which will
		 * be filled with the next chunk of bytes of this Future input value
		 * @param size the number of bytes to get from the input storage value
		 * @retval true if the right amount bytes of this Future could be read 
		 * successfully
		 * @retval false if there is no Future with id @p id, or if @p size is
		 * larger than the remaining number of bytes to be read from the input 
		 * storage value
		 * 
		 * @sa Future::Future(const IN&)
		 * @sa Future.status()
		 * @sa get_storage_value(uint8_t, uint8_t&) const
		 * @sa get_storage_value_(uint8_t, uint8_t*, uint8_t) const
		 */
		bool get_storage_value(uint8_t id, uint8_t* chunk, uint8_t size) const
		{
			synchronized return get_storage_value_(id, chunk, size);
		}

		/**
		 * Check the number of bytes remaining to write to the output value of
		 * a Future identified by @p id.
		 * This method is called by a Future output value producer to know how many
		 * bytes remain to write until the end of the output value.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.get_future_value_size()` instead.
		 * 
		 * @param id the unique id of the Future to query
		 * @return the number of bytes to be written to the output value stored
		 * in Future identified by id @p id
		 * 
		 * @sa get_future_value_size()
		 */
		uint8_t get_future_value_size_(uint8_t id) const;

		/**
		 * Mark the Future identified by @p id as `FutureStatus::READY`.
		 * This method is called by a Future ouput value provider to indicate
		 * that a Future is ready for use.
		 * This method is useful only for `Future<void>` i.e. `Future`s that have 
		 * no output, but exist as way to indicate the end of an asynchronous 
		 * process. For other `Future<T>`s, with a non `void` type `T`, you should
		 * use one of `set_future_value` methods instead.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.set_future_finish()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked ready
		 * @retval false if there is no Future with id @p id, or if the related 
		 * Future cannot be updated properly (because it is not in
		 * `FutureStatus::NOT_READY` or if it is still expecting data)
		 * @retval true if the matching Future has been properly updated
		 * 
		 * @sa Future.status()
		 * @sa set_future_value_()
		 * @sa set_future_error_()
		 * @sa set_future_finish()
		 */
		bool set_future_finish_(uint8_t id) const;

		/**
		 * Add one byte to the output value content of the Future identified 
		 * by @p id.
		 * This method is called by a Future ouput value provider to fill up,
		 * byte after byte, the output value of a Future.
		 * Calling this method may change the status of the Future to `FutureStatus::READY`
		 * if this is the last output value byte to be filled for this Future.
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * You should not use it for a `Future<void>` instance.
		 * 
		 * It is also possible to fill the output value by larger chunks, with
		 * other overloaded versions of this method.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.set_future_value(uint8_t, uint8_t) const` instead.
		 * 
		 * @param id the unique id of the Future which output value shall be filled up
		 * @param chunk the byte to append to this Future output value
		 * @retval true if @p chunk could be added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa Future.status()
		 * @sa set_future_value_(uint8_t, const uint8_t*, uint8_t) const
		 * @sa set_future_value_(uint8_t, const T&) const
		 * @sa set_future_value(uint8_t, uint8_t) const
		 */
		bool set_future_value_(uint8_t id, uint8_t chunk) const;

		/**
		 * Add several bytes to the output value content of the Future identified 
		 * by @p id.
		 * This method is called by a Future ouput value provider to fill up,
		 * with a chunk of bytes, the output value of a Future.
		 * Calling this method may change the status of the Future to `FutureStatus::READY`
		 * if this is the last output value chunk to be filled for this Future.
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * You should not use it for a `Future<void>` instance.
		 * 
		 * It is also possible to fill the output value byte per byte, with
		 * other overloaded versions of this method.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.set_future_value(uint8_t, const uint8_t*, uint8_t) const` 
		 * instead.
		 * 
		 * @param id the unique id of the Future which output value shall be filled up
		 * @param chunk pointer to the first byte to be added to this Future output
		 * value
		 * @param size the number of bytes to be appended to this Future output value
		 * @retval true if @p chunk could be completely added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`, or when @p size
		 * additional bytes would make the output value bigger than expected
		 * 
		 * @sa Future.status()
		 * @sa set_future_value_(uint8_t, uint8_t) const
		 * @sa set_future_value_(uint8_t, const T&) const
		 * @sa set_future_value(uint8_t, const uint8_t*, uint8_t) const
		 */
		bool set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const;

		/**
		 * Set the output value content of the Future identified by @p id.
		 * This method is called by a Future ouput value provider to fully fill
		 * up, with the proper value, the output value of a Future.
		 * Calling this method will change the status of the Future to 
		 * `FutureStatus::READY`
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * 
		 * It is also possible to fill the output value byte per byte, with
		 * other overloaded versions of this method.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.set_future_value(uint8_t, const T&) const` instead.
		 * 
		 * @tparam T the type of output value of the Future indetified by @p id
		 * 
		 * @param id the unique id of the Future which output value shall be set
		 * @param value a constant reference to the value to set in the target Future
		 * @retval true if @p value could be properly set into the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa Future.status()
		 * @sa set_future_value_(uint8_t, uint8_t) const
		 * @sa set_future_value_(uint8_t, const uint8_t*, uint8_t) const
		 * @sa set_future_value(uint8_t, const T&) const
		 */
		template<typename T> bool set_future_value_(uint8_t id, const T& value) const;

		/**
		 * Mark the Future identified by @p id as `FutureStatus::ERROR`.
		 * This method is called by a Future ouput value provider to indicate
		 * that it cannot compute an output value for a given Future.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.set_future_error()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked in error
		 * @param error the error code to set for the Future
		 * @retval true if the matching Future has been properly updated
		 * @retval false if there is no Future with id @p id, or if the related 
		 * Future cannot be updated properly (because it is not in
		 * `FutureStatus::NOT_READY`)
		 * 
		 * @sa Future.status()
		 * @sa set_future_value_()
		 * @sa set_future_finish_()
		 * @sa set_future_error()
		 */
		bool set_future_error_(uint8_t id, int error) const;

		/**
		 * Check the number of bytes remaining to read from a Future identified
		 * by @p id.
		 * This method is called by a Future input value consumer to know how many
		 * bytes remain to get read until the end of the input value.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.get_storage_value_size()` instead.
		 * 
		 * @param id the unique id of the Future that shall be marked ready
		 * @return the number of bytes to be read from the input value stored
		 * by Future identified by id @p id
		 * 
		 * @sa get_storage_value_size()
		 */
		uint8_t get_storage_value_size_(uint8_t id) const;

		/**
		 * Get one byte from the input storage value of the Future identified 
		 * by @p id.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next byte of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.get_storage_value(uint8_t, uint8_t&) const` instead.
		 * 
		 * @param id the unique id of the Future which input storage value to get
		 * @param chunk the byte reference that will receive the next byte of this
		 * Future input value
		 * @retval true if the next byte of this Future could be read successfully
		 * @retval false if there is no Future with id @p id, or if all bytes of 
		 * its input storage value have been read already
		 * 
		 * @sa Future::Future(const IN&)
		 * @sa Future.status()
		 * @sa get_storage_value_(uint8_t, uint8_t*, uint8_t) const
		 * @sa get_storage_value(uint8_t, uint8_t&) const
		 */
		bool get_storage_value_(uint8_t id, uint8_t& chunk) const;

		/**
		 * Get @p size bytes from the input storage value of the Future identified 
		 * by @p id.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next chunk of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractFutureManager.get_storage_value(uint8_t, uint8_t*, uint8_t) const` 
		 * instead.
		 * 
		 * @param id the unique id of the Future which input storage value to get
		 * @param chunk a pointer to an array of at least @p size bytes, which will
		 * be filled with the next chunk of bytes of this Future input value
		 * @param size the number of bytes to get from the input storage value
		 * @retval true if the right amount bytes of this Future could be read 
		 * successfully
		 * @retval false if there is no Future with id @p id, or if @p size is
		 * larger than the remaining number of bytes to be read from the input 
		 * storage value
		 * 
		 * @sa Future::Future(const IN&)
		 * @sa Future.status()
		 * @sa get_storage_value_(uint8_t, uint8_t&) const
		 * @sa get_storage_value(uint8_t, uint8_t*, uint8_t) const
		 */
		bool get_storage_value_(uint8_t id, uint8_t* chunk, uint8_t size) const;

	protected:
		/// @cond notdocumented
		AbstractFutureManager(AbstractFuture** futures, uint8_t size)
			: size_{size}, futures_{futures}
		{
			for (uint8_t i = 0; i < size_; ++i)
				futures_[i] = nullptr;
			synchronized AbstractFutureManager::instance_ = this;
		}

		AbstractFutureManager(const AbstractFutureManager&) = delete;
		AbstractFutureManager& operator=(const AbstractFutureManager&) = delete;
		~AbstractFutureManager()
		{
			synchronized AbstractFutureManager::instance_ = nullptr;
		}
		/// @endcond

	private:
		static AbstractFutureManager* instance_;

		bool register_at_index_(AbstractFuture& future, uint8_t index);

		AbstractFuture* find_future(uint8_t id) const
		{
			if ((id == 0) || (id > size_))
				return nullptr;
			return futures_[id - 1];
		}
		bool update_future(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
		{
			synchronized return update_future_(id, old_address, new_address);
		}
		// Called by Future themselves (on construction, destruction, assignment)
		bool update_future_(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
		{
			// Check id is plausible and address matches
			if (find_future(id) != old_address)
				return false;
			futures_[id - 1] = new_address;
			if (new_address == nullptr)
				last_removed_id_ = id;
			return true;
		}

		const uint8_t size_;
		AbstractFuture** futures_;
		uint8_t last_removed_id_ = 0;

		friend class AbstractFuture;
	};

	/**
	 * The actual FutureManager implementation, based on `AbstractFutureManager`,
	 * adding static storage for it.
	 * You must define a `FutureManager` instance if you want to use FastArduino
	 * futures comcept.
	 * 
	 * @tparam SIZE the maximum number of `Future`s this FutureManager can register,
	 * i.e. the maximum number of `Future`s that can exist simultaneously in the
	 * system
	 */
	template<uint8_t SIZE>
	class FutureManager : public AbstractFutureManager
	{
	public:
		/** 
		 * Construct a FutureManager, that will become THE FutureManager singleton.
		 * @sa AbstractFutureManager
		 * @sa AbstractFutureManager.instance()
		 */
		FutureManager() : AbstractFutureManager{buffer_, SIZE}, buffer_{} {}

	private:
		AbstractFuture* buffer_[SIZE];
	};

	/**
	 * Status of a Future.
	 * A Future follows a strict lifecycle by passing through the various statuses
	 * defined here.
	 * 
	 * @sa Future
	 */
	enum class FutureStatus : uint8_t
	{
		/** 
		 * The initial status of a Future, once constructed.
		 * This is also the final status of a Future, once it has been "read"
		 * by a consumer.
		 * This can finally be the status of a Future that has been "moved" to 
		 * another Future (though C++ move constructor or move assignment operator).
		 * 
		 * @sa Future.get()
		 * @sa Future.error()
		 */
		INVALID = 0,

		/**
		 * The status of a Future immediately after it has been registered with
		 * the `FutureManager`.
		 * The Future will keep this status until:
		 * - either its output value provider has fully filled its value (then its
		 * status will become `READY`)
		 * - or its output value provider has reported an error to it (then its 
		 * status will become `ERROR`)
		 * - or it is moved to another Future (then its status will become `INVALID`)
		 * 
		 * @sa AbstractFutureManager.register_future()
		 * @sa AbstractFutureManager.set_future_value()
		 * @sa AbstractFutureManager.set_future_finish()
		 * @sa AbstractFutureManager.set_future_error()
		 */
		NOT_READY,

		/**
		 * The status of a Future once its output value has been fully set by a 
		 * provider.
		 * The Future will keep this value until:
		 * - either its value is read from some consumer code (then its 
		 * status will become `INVALID`)
		 * - or it is moved to another Future (then its status will become 
		 * `INVALID`)
		 * 
		 * @sa AbstractFutureManager.set_future_value()
		 * @sa AbstractFutureManager.set_future_finish()
		 * @sa Future.get()
		 */
		READY,

		/**
		 * The status of a Future once a value provider has reported an error 
		 * to it.
		 * The Future will keep this value until:
		 * - either its error is read from some consumer code (then its 
		 * status will become `INVALID`)
		 * - or it is moved to another Future (then its status will become 
		 * `INVALID`)
		 * 
		 * @sa AbstractFutureManager.set_future_error()
		 * @sa Future.error()
		 */
		ERROR
	};

	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, future::FutureStatus s);
	/// @endcond

	/**
	 * Base class for all `Future`s.
	 * This defines most API and implementation of a Future.
	 * 
	 * @sa Future
	 */
	class AbstractFuture
	{
	public:
		/**
		 * The unique ID of this Future, as provded by the `FutureManager` 
		 * upon registration.
		 * This is `0` when:
		 * - the Future has just been constructed (not registered yet)
		 * - the Future has just been moved to another Future.
		 * 
		 * @sa AbstractFutureManager.register_future()
		 */
		uint8_t id() const
		{
			return id_;
		}

		/**
		 * The current status of this Future.
		 * 
		 * @sa FutureStatus
		 */
		FutureStatus status() const
		{
			return status_;
		}

		/**
		 * Wait until this Future becomes "ready", that is when it holds either an
		 * output value or an error.
		 * The method returns immediately if this Future is "INVALID".
		 * 
		 * @return the latest status of the Future
		 * 
		 * @sa Future.get()
		 * @sa error()
		 */
		FutureStatus await() const
		{
			while (status_ == FutureStatus::NOT_READY)
				time::yield();
			return status_;
		}

		/**
		 * Wait until this Future becomes "ready", that is when it holds either an
		 * output value or an error, then return the error reported.
		 * Calling this method when the Future holds an error will change its
		 * status to `FutureStatus::INVALID`.
		 * 
		 * @retval 0 if the Future is READY, i.e. holds a valid output value
		 * @retval EINVAL if the Future is currently `INVALID`
		 * @return the actual error reported by a provider on this Future
		 * 
		 * @sa await()
		 * @sa AbstractFutureManager.set_future_error()
		 * @sa errors::EINVAL
		 */
		int error()
		{
			switch (await())
			{
				case FutureStatus::ERROR:
				status_ = FutureStatus::INVALID;
				return error_;

				case FutureStatus::READY:
				return 0;

				case FutureStatus::INVALID:
				default:
				return errors::EINVAL;
			}
		}

	protected:
		/// @cond notdocumented
		// Constructor used by FutureManager
		AbstractFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
			:	output_data_{output_data}, output_current_{output_data}, output_size_{output_size},
				input_data_{input_data}, input_current_{input_data}, input_size_{input_size} {}
		~AbstractFuture()
		{
			// Notify FutureManager about destruction
			AbstractFutureManager::instance().update_future(id_, this, nullptr);
		}

		AbstractFuture(const AbstractFuture&) = delete;
		AbstractFuture& operator=(const AbstractFuture&) = delete;
		AbstractFuture(AbstractFuture&&) = delete;
		AbstractFuture& operator=(AbstractFuture&&) = delete;

		void invalidate()
		{
			status_ = FutureStatus::INVALID;
		}

		// This method is called by subclass to check if input is replaceable,
		// i.e. it has not been read yet
		bool can_replace_input() const
		{
			synchronized return can_replace_input_();
		}
		bool can_replace_input_() const
		{
			return (input_current_ == input_data_);
		}

		// This method is called by subclass in their move constructor and assignment operator
		void move_(AbstractFuture&& that, uint8_t full_output_size, uint8_t full_input_size)
		{
			// In case this Future is valid, it must be first invalidated with FutureManager
			AbstractFutureManager::instance().update_future_(id_, this, nullptr);

			// Now copy all attributes from rhs (output_data_ was already initialized when this was constructed)
			id_ = that.id_;
			status_ = that.status_;
			error_ = that.error_;
			output_size_ = that.output_size_;
			input_size_ = that.input_size_;
			// Calculate data pointer attribute for next set value calls
			output_current_ = output_data_ + full_output_size - output_size_;
			input_current_ = input_data_ + full_input_size - input_size_;

			// Notify FutureManager about Future move
			if (!AbstractFutureManager::instance().update_future_(id_, &that, this))
				status_ = FutureStatus::INVALID;

			// Make rhs Future invalid
			that.id_ = 0;
			that.status_ = FutureStatus::INVALID;
		}
		/// @endcond

	private:
		// The following methods are called by FutureManager to fill the Future value (or error)
		uint8_t get_output_size_() const
		{
			return output_size_;
		}

		bool set_finish_()
		{
			// Check this future is waiting for data
			if (status_ != FutureStatus::NOT_READY)
				return false;
			if (output_size_ == 0)
				status_ = FutureStatus::READY;
			return true;
		}
		bool set_chunk_(uint8_t chunk)
		{
			// Check this future is waiting for data
			if (status_ != FutureStatus::NOT_READY)
				return false;
			// Update Future value chunk
			*output_current_++ = chunk;
			// Is that the last chunk?
			if (--output_size_ == 0)
				status_ = FutureStatus::READY;
			return true;
		}
		bool set_chunk_(const uint8_t* chunk, uint8_t size)
		{
			// Check this future is waiting for data
			if (status_ != FutureStatus::NOT_READY)
				return false;
			// Check size does not go beyond expected size
			if (size > output_size_)
			{
				// Store error
				set_error_(errors::EMSGSIZE);
				return false;
			}
			memcpy(output_current_, chunk, size);
			output_current_ += size;
			// Is that the last chunk?
			output_size_ -= size;
			if (output_size_ == 0)
				status_ = FutureStatus::READY;
			return true;
		}
		bool set_error_(int error)
		{
			// Check this future is waiting for data
			if (error == 0 || status_ != FutureStatus::NOT_READY)
				return false;
			error_ = error;
			status_ = FutureStatus::ERROR;
			return true;
		}

		// The following methods are called by FutureManager to get the read-only value held by this Future
		uint8_t get_input_size_() const
		{
			return input_size_;
		}

		bool get_chunk_(uint8_t& chunk)
		{
			// Check all bytes have not been transferred yet
			if (!input_size_)
				return false;
			chunk = *input_current_++;
			--input_size_;
			return true;
		}
		bool get_chunk_(uint8_t* chunk, uint8_t size)
		{
			// Check size does not go beyond transferrable size
			if (size > input_size_)
				return false;
			memcpy(chunk, input_current_, size);
			input_current_ += size;
			input_size_ -= size;
			return true;
		}

		uint8_t id_ = 0;
		volatile FutureStatus status_ = FutureStatus::INVALID;
		int error_ = 0;

		uint8_t* output_data_ = nullptr;
		uint8_t* output_current_ = nullptr;
		uint8_t output_size_ = 0;
		
		uint8_t* input_data_ = nullptr;
		uint8_t* input_current_ = nullptr;
		uint8_t input_size_ = 0;

		friend class AbstractFutureManager;
	};

	template<typename T> bool AbstractFutureManager::set_future_value_(uint8_t id, const T& value) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->set_chunk_(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
	}

	/**
	 * Represent a value to be obtained, in some asynchronous way, in the future.
	 * A Future can be thought of as a container (or buffer) that is here to receive
	 * some value that will be read later on. The value can be fed by some external 
	 * function, either as a whole, or possibly byte per byte or even as chunks of 
	 * bytes. This value is called an output value as it represents the output of
	 * some asynchronous function.
	 * A Future is also a holder for a constant value that is persistent as long 
	 * as the Future persists; this value is called "input storage value" as it can 
	 * serve as input to an asynchronous function.
	 * A Future can also an error code instead of a valid output value; that code
	 * is provided, when needed, by the asynchronous function computing the output
	 * value, if it encounters an unrecoverable error.
	 * To be usable, any Future must have been registered first with a `FutureManager`.
	 * 
	 * This is a template class where one can define the types of the input storage 
	 * value and of the output value.
	 * Template specializations exist for @p OUT or @p IN `void`.
	 * 
	 * The lifecycle of a Future is described in further details in `FutureStatus`.
	 * 
	 * @tparam OUT the type of the output value; `void` by default. 
	 * This type is limited to 255 bytes in length.
	 * @tparam IN the type of the input storage value; `void` by default.
	 * This type is limited to 255 bytes in length.
	 * 
	 * @sa AbstractFuture
	 * @sa FutureStatus
	 */
	template<typename OUT_ = void, typename IN_ = void>
	class Future : public AbstractFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		/** Type of the output value of this Future. */
		using OUT = OUT_;
		/** Type of the input value of this Future. */
		using IN = IN_;

		/** 
		 * Construct a new Future.
		 * The created Future is in `FutureStatus::INVALID` and has no `id()` yet.
		 * It must be registered with `FutureManager` to become usable.
		 * The Future holds buffers to store both the input storage value and the 
		 * output value.
		 * These buffers are moved between Futures during move construction or move 
		 * assignment.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 * 
		 * @sa AbstractFuture
		 * @sa status()
		 * @sa FutureStatus
		 * @sa FutureManager
		 * @sa Future(Future&&)
		 * @sa operator=(Future&&)
		 */
		explicit Future(const IN& input = IN{})
			: AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}, input_{input} {}

		/**
		 * Destroy an existing Future.
		 * This Future will be automatically deregistered from `FutureManager`.
		 */
		~Future() = default;

		/**
		 * Construct a new Future from @p that Future, through moving operation.
		 * After this Future has been constructed:
		 * - @p that Future becomes unsable (`FutureStatus::INVALID`, `id() = 0`)
		 * - @p that is deregistered from `FutureManager` if it was registered already
		 * - this Future has the same `id()`, `state()` and values as @p that Future
		 * before the move
		 * - this Future is registered with `FutureManager` if @p that Future was
		 * 
		 * @param that the Future to be moved to this Future
		 * 
		 * @code
		 * Future<uint16_t> future1;
		 * // Do something with future1
		 * ...
		 * // Move future1 to future2, a new Future
		 * Future<uint16_t> future2 = std::move(future1);
		 * // Now use future2 only, as future1 has become useless
		 * ...
		 * @endcode
		 */
		Future(Future<OUT, IN>&& that) : AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}
		{
			move(std::move(that));
		}

		/**
		 * Move @p that Future to this Future.
		 * After this operation is finished:
		 * - @p that Future becomes unsable (`FutureStatus::INVALID`, `id() = 0`)
		 * - @p that is deregistered from `FutureManager` if it was registered already
		 * - this Future has the same `id()`, `state()` and values as @p that Future
		 * before the move
		 * - this Future is registered with `FutureManager` if @p that Future was
		 * 
		 * @param that the Future to be moved to this Future
		 * 
		 * @code
		 * Future<uint16_t> future1, future2;
		 * // Do something with future1
		 * ...
		 * // Move future1 to future2, an existing Future
		 * future2 = std::move(future1);
		 * // Now use future2 only, as future1 has become useless
		 * // If future2 was already registered before, it has been first 
		 * // deregistered before the move.
		 * ...
		 * @endcode
		 */
		Future<OUT, IN>& operator=(Future<OUT, IN>&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		/// @cond notdocumented
		Future(const Future<OUT, IN>&) = delete;
		Future<OUT, IN>& operator=(const Future<OUT, IN>&) = delete;
		/// @endcond

		/**
		 * Reset the input storage value held by this Future with a new value.
		 * This is possible only if no consumer has started reading the current 
		 * input storage value yet.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `reset_input_()` instead.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 * @retval true if the input strage value has been successfully replaced
		 * @retval false if the input strage value could not be replaced because 
		 * a consumer already started reading the previous input storage value
		 * 
		 * @sa reset_input_()
		 */
		bool reset_input(const IN& input)
		{
			synchronized return reset_input_(input);
		}

		/**
		 * Reset the input storage value held by this Future with a new value.
		 * This is possible only if no consumer has started reading the current 
		 * input storage value yet.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `reset_input()` instead.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 * @retval true if the input strage value has been successfully replaced
		 * @retval false if the input strage value could not be replaced because 
		 * a consumer already started reading the previous input storage value
		 * 
		 * @sa reset_input()
		 */
		bool reset_input_(const IN& input)
		{
			if (!can_replace_input_()) return false;
			input_ = input;
			return true;
		}

		/**
		 * Wait until an output value has been completely filled in this Future
		 * and return that value to the caller.
		 * 
		 * Once its output value has been read once through this method, this
		 * Future becomes invalid and cannot be reused.
		 * 
		 * This method should never be called from an ISR as it is blocking for
		 * an undetermined time!
		 * 
		 * @param result a reference to a variable that will be assigned the output 
		 * value, once available; if an error occurred, result is unchanged. This
		 * argument does not exist when @p OUT is `void`
		 * @retval true if an output value has been set to this Future and it was
		 * assigned to @p result
		 * @retval false if this Future is not yet registered, if it is invalid,
		 * or if an error was reported to it; the error can be obtained through 
		 * `error()`.
		 * 
		 * @sa await()
		 * @sa error()
		 * @sa FutureStatus::READY
		 * @sa FutureStatus::ERROR
		 */
		bool get(OUT& result)
		{
			if (await() != FutureStatus::READY)
				return false;
			result = output_;
			invalidate();
			return true;
		}

	protected:
		/**
		 * Return the input storage value as it was initially set (or reset 
		 * through `reset_input()`), whatever the current state of this Future.
		 * @sa reset_input()
		 */
		const IN& get_input() const
		{
			return input_;
		}

	private:
		void move(Future<OUT, IN>&& that)
		{
			synchronized
			{
				memcpy(output_buffer_, that.output_buffer_, sizeof(OUT));
				memcpy(input_buffer_, that.input_buffer_, sizeof(IN));
				move_(std::move(that), sizeof(OUT), sizeof(IN));
			}
		}

		union
		{
			OUT output_;
			uint8_t output_buffer_[sizeof(OUT)];
		};
		union
		{
			IN input_;
			uint8_t input_buffer_[sizeof(IN)];
		};
	};

	// Future template specializations for void types
	//================================================
	/// @cond notdocumented	
	template<typename OUT_>
	class Future<OUT_, void> : public AbstractFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");

	public:
		using OUT = OUT_;
		using IN = void;

		Future() : AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0} {}
		~Future() = default;

		Future(Future<OUT, void>&& that) : AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0}
		{
			move(std::move(that));
		}
		Future<OUT, void>& operator=(Future<OUT, void>&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		Future(const Future<OUT, void>&) = delete;
		Future<OUT, void>& operator=(const Future<OUT, void>&) = delete;

		// The following method is blocking until this Future is ready
		bool get(OUT& result)
		{
			if (await() != FutureStatus::READY)
				return false;
			result = output_;
			invalidate();
			return true;
		}

	private:
		void move(Future<OUT, void>&& that)
		{
			synchronized
			{
				memcpy(output_buffer_, that.output_buffer_, sizeof(OUT));
				move_(std::move(that), sizeof(OUT), 0);
			}
		}

		union
		{
			OUT output_;
			uint8_t output_buffer_[sizeof(OUT)];
		};
	};
	/// @endcond

	/// @cond notdocumented	
	template<typename IN_>
	class Future<void, IN_> : public AbstractFuture
	{
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		using OUT = void;
		using IN = IN_;

		explicit Future(const IN& input = IN{})
			: AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN)}, input_{input} {}
		~Future() = default;

		Future(Future<void, IN>&& that) : AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN)}
		{
			move(std::move(that));
		}
		Future<void, IN>& operator=(Future<void, IN>&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		Future(const Future<void, IN>&) = delete;
		Future<void, IN>& operator=(const Future<void, IN>&) = delete;

		bool reset_input(const IN& input)
		{
			synchronized return reset_input_(input);
		}
		bool reset_input_(const IN& input)
		{
			if (!can_replace_input_()) return false;
			input_ = input;
			return true;
		}

		// The following method is blocking until this Future is ready
		bool get()
		{
			if (await() != FutureStatus::READY)
				return false;
			invalidate();
			return true;
		}

	protected:
		const IN& get_input() const
		{
			return input_;
		}

	private:
		void move(Future<void, IN>&& that)
		{
			synchronized
			{
				memcpy(input_buffer_, that.input_buffer_, sizeof(IN));
				move_(std::move(that), 0, sizeof(IN));
			}
		}

		union
		{
			IN input_;
			uint8_t input_buffer_[sizeof(IN)];
		};
	};
	/// @endcond

	/// @cond notdocumented	
	template<>
	class Future<void, void> : public AbstractFuture
	{
	public:
		using OUT = void;
		using IN = void;

		Future() : AbstractFuture{nullptr, 0,nullptr, 0} {}
		~Future() = default;

		Future(Future<void, void>&& that) : AbstractFuture{nullptr, 0, nullptr, 0}
		{
			synchronized move_(std::move(that), 0, 0);
		}
		Future<void, void>& operator=(Future<void, void>&& that)
		{
			if (this == &that) return *this;
			synchronized move_(std::move(that), 0, 0);
			return *this;
		}

		Future(const Future<void, void>&) = delete;
		Future<void, void>& operator=(const Future<void, void>&) = delete;

		// The following method is blocking until this Future is ready
		bool get()
		{
			if (await() != FutureStatus::READY)
				return false;
			invalidate();
			return true;
		}
	};
	/// @endcond
}

#endif /* FUTURE_HH */
/// @endcond
