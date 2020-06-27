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

#include "move.h"
#include "future_commons.h"

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
 * void take_snapshot(future::AbstractFutureManager& manager) {
 *     gpio::FastPort<board::Port::PORT_B> port;
 *     manager.set_future_value(portB_snapshot_id, port.get_PIN());
 * }
 * REGISTER_PCI_ISR_FUNCTION(0, take_snapshot, board::InterruptPin::D8_PB0_PCI0)
 * ...
 * 
 * // Within main()
 * // First create a FutureManager with max 16 futures
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
 * 
 * @note There exist different implementation of Futures and FutureManagers with
 * distinct characteristics:
 * - FutureManager (default) handles futures with potential long lifetime, that can
 * be "moved around" without losing their reference: wherever they are moved, they
 * are guaranteed to be reached and updated when needed
 * - StaticFutureManager handles futures that are not moveable (code is smaller than
 * standard Futuremanager where futures are moveable)
 * - SingleFutureManager handles only one -non moveable- future with very short
 * lifetime (i.e. a kind of "immediate" future); its code is much smaller than other
 * implementations and can be used in situations where a future is expected but you
 * want to await its result immediately.
 * 
 * @sa StaticFutureManager
 * @sa SingleFutureManager
 */
namespace future
{
	/// @cond notdocumented
	// Forward declaration needed by FUTURE
	template<typename OUT, typename IN> class Future;
	/// @endcond

	/**
	 * This is the parent of `FutureManager` and it provides all `FutureManager` API.
	 * You should normally never need to subclass it.
	 * An `AbstractFutureManager` holds a limited number of registered `Future`s;
	 * the limit is fixed at construction time of `FutureManager`.
	 * 
	 * @sa FutureManager
	 * @sa Future
	 */
	class AbstractFutureManager : public AbstractMultiFutureManager
	{
	public:
		//TODO DOC
		template<typename OUT, typename IN> using FUTURE = Future<OUT, IN>;

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
		 * @sa set_future_value_(uint8_t, const uint8_t*, uint8_t) const
		 */
		bool set_future_value(uint8_t id, const uint8_t* chunk, uint8_t size) const
		{
			synchronized return set_future_value_(id, chunk, size);
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
		uint8_t get_future_value_size_(uint8_t id) const
		{
			return AbstractMultiFutureManager::get_future_value_size_(find_future(id));
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
		bool set_future_finish_(uint8_t id) const
		{
			return AbstractMultiFutureManager::set_future_finish_(find_future(id));
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
		 * @sa set_future_value(uint8_t, uint8_t) const
		 */
		bool set_future_value_(uint8_t id, uint8_t chunk) const
		{
			return AbstractMultiFutureManager::set_future_value_(find_future(id), chunk);
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
		 * @sa set_future_value(uint8_t, const uint8_t*, uint8_t) const
		 */
		bool set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const
		{
			return AbstractMultiFutureManager::set_future_value_(find_future(id), chunk, size);
		}

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
		bool set_future_error_(uint8_t id, int error) const
		{
			return AbstractMultiFutureManager::set_future_error_(find_future(id), error);
		}

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
		uint8_t get_storage_value_size_(uint8_t id) const
		{
			return AbstractMultiFutureManager::get_storage_value_size_(find_future(id));
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
		bool get_storage_value_(uint8_t id, uint8_t& chunk) const
		{
			return AbstractMultiFutureManager::get_storage_value_(find_future(id), chunk);
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
		bool get_storage_value_(uint8_t id, uint8_t* chunk, uint8_t size) const
		{
			return AbstractMultiFutureManager::get_storage_value_(find_future(id), chunk, size);
		}

	protected:
		/// @cond notdocumented
		AbstractFutureManager(AbstractManagedFuture** futures, uint8_t size)
			: AbstractMultiFutureManager{futures, size} {}

		AbstractFutureManager(const AbstractFutureManager&) = delete;
		AbstractFutureManager& operator=(const AbstractFutureManager&) = delete;
		AbstractFutureManager(AbstractFutureManager&&) = delete;
		AbstractFutureManager& operator=(AbstractFutureManager&&) = delete;
		~AbstractFutureManager() = default;
		/// @endcond
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
		 * Construct a FutureManager.
		 * @sa AbstractFutureManager
		 */
		FutureManager() : AbstractFutureManager{buffer_, SIZE}, buffer_{} {}

	private:
		AbstractManagedFuture* buffer_[SIZE];
	};

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
	 * @sa AbstractManagedFuture
	 * @sa FutureStatus
	 */
	template<typename OUT_ = void, typename IN_ = void>
	class Future : public AbstractManagedFuture
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
		 * @sa AbstractManagedFuture
		 * @sa status()
		 * @sa FutureStatus
		 * @sa FutureManager
		 * @sa Future(Future&&)
		 * @sa operator=(Future&&)
		 */
		explicit Future(const IN& input = IN{})
			: AbstractManagedFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}, input_{input} {}

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
		Future(Future<OUT, IN>&& that) : AbstractManagedFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}
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
	class Future<OUT_, void> : public AbstractManagedFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");

	public:
		using OUT = OUT_;
		using IN = void;

		Future() : AbstractManagedFuture{output_buffer_, sizeof(OUT), nullptr, 0} {}
		~Future() = default;

		Future(Future<OUT, void>&& that) : AbstractManagedFuture{output_buffer_, sizeof(OUT), nullptr, 0}
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
	class Future<void, IN_> : public AbstractManagedFuture
	{
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		using OUT = void;
		using IN = IN_;

		explicit Future(const IN& input = IN{})
			: AbstractManagedFuture{nullptr, 0, input_buffer_, sizeof(IN)}, input_{input} {}
		~Future() = default;

		Future(Future<void, IN>&& that) : AbstractManagedFuture{nullptr, 0, input_buffer_, sizeof(IN)}
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
	class Future<void, void> : public AbstractManagedFuture
	{
	public:
		using OUT = void;
		using IN = void;

		Future() : AbstractManagedFuture{nullptr, 0,nullptr, 0} {}
		~Future() = default;

		Future(Future<void, void>&& that) : AbstractManagedFuture{nullptr, 0, nullptr, 0}
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

	/// @cond notdocumented
	template<uint8_t SIZE> struct FutureManager_trait<FutureManager<SIZE>>
	{
		static constexpr const bool IS_FUTURE_MANAGER = true;
	};
	/// @endcond
}

#endif /* FUTURE_HH */
/// @endcond
