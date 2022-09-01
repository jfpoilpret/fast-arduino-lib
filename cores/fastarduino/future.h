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
 * Utility API to handle the concept of futures.
 * For general discussion about this concept, please check 
 * https://en.wikipedia.org/wiki/Futures_and_promises
 */
#ifndef FUTURE_HH
#define FUTURE_HH

#include <string.h>
#include "flash.h"
#include "interrupts.h"
#include "iterator.h"
#include "errors.h"
#include "time.h"

/**
 * Register the necessary callbacks that will be notified when a `future::Future`
 * (or a `future::FakeFuture`) has its status changed.
 * 
 * @note only Futures constructed with `FutureNotification::STATUS` or 
 * `FutureNotification::BOTH` notifications parameter will produce such notifications.
 * 
 * Each handler registered here will be notified of status changes for ALL futures
 * producing such notifications.
 * 
 * @warning this macro must be called only once, with all interested handlers classes;
 * calling it more than once will lead to errors at link time.
 * 
 * @note you do not need to call this macro if you do not use futures in your program
 * (do note that I2C support does use futures, even if you don't care about futures).
 * 
 * @note When an I2C device needs you to register some of its classes, it is indicated
 * in its API documentation.
 * 
 * @param ABSTRACT_TYPE one of `AbstractFuture` or `AbstractFakeFuture`; any other
 * type will fail compilation.
 * @param HANDLER1 a class which registered instance will be notified, through its
 * `void on_status_change(const ABSTRACT_TYPE&, FutureStatus)` method when 
 * a notifying future has its status changed.
 * @param ... other classes similar to HANDLER1.
 * 
 * @sa REGISTER_FUTURE_STATUS_NO_LISTENERS()
 * @sa interrupt::register_handler
 */
#define REGISTER_FUTURE_STATUS_LISTENERS(ABSTRACT_TYPE, HANDLER1, ...)								\
	namespace future																				\
	{																								\
		void future_on_status_change_dispatch(const AbstractFuture& future, FutureStatus status)	\
		{																							\
			dispatch_handler<ABSTRACT_TYPE, AbstractFuture>::future_on_status_change<				\
				HANDLER1, ##__VA_ARGS__>(future, status);											\
		}																							\
		void future_on_status_change_dispatch(const AbstractFakeFuture& future, FutureStatus status)\
		{																							\
			dispatch_handler<ABSTRACT_TYPE, AbstractFakeFuture>::future_on_status_change<			\
				HANDLER1, ##__VA_ARGS__>(future, status);											\
		}																							\
	}																								\

/**
 * Register no callback at all to Future status changes notification.
 * You normally do not need this macro, except if you:
 * - use Futures (directly or indirectly, i.e. through I2C support API)
 * - do not use `future::AbstractFuturesGroup` (directlty or indirectlty, some I2C devices use it)
 * - you do not need to be called back when one of your Futures change state
 * 
 * @note When an I2C device needs you to register some of its classes, it is indicated
 * in its API documentation.
 * 
 * @sa REGISTER_FUTURE_STATUS_LISTENERS()
 * @sa REGISTER_FUTURE_NO_LISTENERS()
 */
#define REGISTER_FUTURE_STATUS_NO_LISTENERS()													\
	void future::future_on_status_change_dispatch(const AbstractFuture&, FutureStatus) {}		\
	void future::future_on_status_change_dispatch(const AbstractFakeFuture&, FutureStatus) {}

/**
 * Register the necessary callbacks that will be notified when a `future::Future`
 * (or a `future::FakeFuture`) has its output content filled in, even partly.
 * 
 * @note only Futures constructed with `FutureNotification::OUTPUT` or 
 * `FutureNotification::BOTH` notifications parameter will produce such notifications.
 * 
 * Each handler registered here will be notified of output changes for ALL futures
 * producing such notifications.
 * 
 * @warning this macro must be called only once, with all interested handlers classes;
 * calling it more than once will lead to errors at link time.
 * 
 * @note you do not need to call this macro if you do not use futures in your program
 * (do note that I2C support does use futures, even if you don't care about futures).
 * 
 * @note When an I2C device needs you to register some of its classes, it is indicated
 * in its API documentation.
 * 
 * @param ABSTRACT_TYPE one of `AbstractFuture` or `AbstractFakeFuture`; any other
 * type will fail compilation.
 * @param HANDLER1 a class which registered instance will be notified, through its
 * `void on_output_change(const ABSTRACT_TYPE&)` method when a notifying future 
 * has its output content filled in.
 * @param ... other classes similar to HANDLER1.
 * 
 * @sa REGISTER_FUTURE_OUTPUT_NO_LISTENERS()
 * @sa interrupt::register_handler
 */
#define REGISTER_FUTURE_OUTPUT_LISTENERS(ABSTRACT_TYPE, HANDLER1, ...)						\
	namespace future																		\
	{																						\
		void future_on_output_change_dispatch(const AbstractFuture& future)					\
		{																					\
			dispatch_handler<ABSTRACT_TYPE, AbstractFuture>::future_on_output_change<		\
				HANDLER1, ##__VA_ARGS__>(future);											\
		}																					\
		void future_on_output_change_dispatch(const AbstractFakeFuture& future)				\
		{																					\
			dispatch_handler<ABSTRACT_TYPE, AbstractFakeFuture>::future_on_output_change<	\
				HANDLER1, ##__VA_ARGS__>(future);											\
		}																					\
	}																						\

/**
 * Register no callback at all to Future output buffer changes notification.
 * You normally do not need this macro, except if you:
 * - use Futures (directly or indirectly, i.e. through I2C support API)
 * - you do not need to be called back when one of your Futures change outut buffer
 * 
 * @note When an I2C device needs you to register some of its classes, it is indicated
 * in its API documentation.
 * 
 * @sa REGISTER_FUTURE_OUTPUT_LISTENERS()
 * @sa REGISTER_FUTURE_NO_LISTENERS()
 */
#define REGISTER_FUTURE_OUTPUT_NO_LISTENERS()									\
	void future::future_on_output_change_dispatch(const AbstractFuture&) {}		\
	void future::future_on_output_change_dispatch(const AbstractFakeFuture&) {}

/**
 * Register no callback at all to any Future notification.
 * You normally do not need this macro, except if you:
 * - use Futures (directly or indirectly, i.e. through I2C support API)
 * - you do not need to be called back when one of your Futures change outut buffer
 * 
 * @note When an I2C device needs you to register some of its classes, it is indicated
 * in its API documentation.
 * 
 * @sa REGISTER_FUTURE_STATUS_NO_LISTENERS()
 * @sa REGISTER_FUTURE_OUTPUT_NO_LISTENERS()
 */
#define REGISTER_FUTURE_NO_LISTENERS()		\
	REGISTER_FUTURE_STATUS_NO_LISTENERS()	\
	REGISTER_FUTURE_OUTPUT_NO_LISTENERS()

/**
 * This macro shall be used in a class containing a private callback method
 * `void on_status_change(const ABSTRACT_TYPE&, FutureStatus)`, registered by 
 * `REGISTER_FUTURE_STATUS_LISTENERS`, or `void on_output_change(const ABSTRACT_TYPE&)`,
 * registered by `REGISTER_FUTURE_OUTPUT_LISTENERS`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_FUTURE_LISTENERS_FRIEND         \
	template<typename> friend struct future::dispatch_handler_impl;

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
 * - A Future is either:
 *      - Not ready: its value has not been obtained yet (initial status)
 *      - Ready: its value has been fully set and not yet read by anyone
 *      - Error: an error occurred in the provider, hence no value will ever be 
 *        held by this Future, the actual error has not yet been read by anyone
 * - Output value providers must hold a reference (or pointer) to the Future they
 * need to fill
 * - Storage input value consumers must hold a reference (or pointer) to the Future
 * in order to get its storage value
 * - It is possible to subclass a Future to add last minute transformation on 
 * `Future.get()` method
 * 
 * There are two hierarchies of futures:
 * - "real futures": all derive from `AbstractFuture`
 * - "fake futures": all derive from `AbstractFakeFuture`
 * 
 * The main difference is that fake futures must be filled in immediately (not piece
 * after piece, e.g. through an ISR).
 * 
 * In general you won't need fake futures in your program, but these exist to fit some
 * FastArduino API (I2C) that can work in either asynchronous or synchronous mode.
 * In synchronous mode, the cost of "real futures" (code size and execution speed)
 * is not needed, but in order to fulfill internal requirements to work with a future 
 * API, we have implemented fake futures too, with the same API as real ones.
 * 
 * Futures can be listened to through a special, virtual-free, mechanism. Futures
 * can send notfications to dedicated listeners.
 * There are 2 kinds of notifications that a future may dispatch:
 * - `FutureNotification::STATUS`: a notification is dispatched whenever the future
 * status changes
 * - `FutureNotification::OUTPUT`: a notification is dispatched whenever the future
 * outpur buffer changes
 * A combination of both is possible.
 * 
 * Listeners to future notification are instances of classes that must:
 * 1. Call `interrupt::register_handler()` at construction time and
 * `interrupt::unregister_handler()` at destruction time
 * 2. Must be registered statically with `REGISTER_FUTURE_STATUS_LISTENERS()`,
 * `REGISTER_FUTURE_OUTPUT_LISTENERS()` or both (if they want to be notified of
 * both kinds of notifications)
 * 3. Have a `on_status_change(const F&, FutureStatus)` or `on_output_change(const F&)`
 * methods (F is either `AbstractFuture` or `AbstractFakeFuture`, based on which 
 * future ind you want to use)
 * 4. These methods must either be `public` in the class or, if not, the class must
 * use `DECL_FUTURE_LISTENERS_FRIEND` in its definition, so that FastArduino notification
 * dispatch mechanism can call those methods
 * 
 * @warning Only one instance can exist for each listener class at a given time;
 * behavior is undefined if several instances exist at the same time.
 * 
 * @warning All listeners receive all notifications coming from all futures having 
 * notification enabled. Hence any listener callback method shall start by checking
 * the passed future reference is of interest or not, then act accordingly.
 * 
 * @warning All listener classes must be registered statically at the same time,
 * with a single call to `REGISTER_FUTURE_STATUS_LISTENERS()` (resp. 
 * `REGISTER_FUTURE_OUTPUT_LISTENERS`). Doing otherwise will result in link errors.
 * 
 * @warning If you use futures in your program but do not need notifications (of one 
 * or both kinds), you still need to use `REGISTER_FUTURE_STATUS_NO_LISTENERS()`,
 * `REGISTER_FUTURE_OUTPUT_NO_LISTENERS()` or `REGISTER_FUTURE_NO_LISTENERS()`.
 * Doing otherwise will result in a link failure.
 * 
 * @note FastArduino I2C API is based on futures, thus the previous warnings apply
 * when you use I2C in your program.
 * 
 * Here is a simple usage example of this API, using PCINT0 interrupt to take a 
 * "snapshot" of 6 input buttons connected to all PORTB pins, after one of them has
 * changed level:
 * @code
 * // PCINT0 ISR
 * class ButtonsSnapshot {
 * public:
 *     ButtonsSnapshot() : port_{0x3F, 0x3F} {
 *         interrupt::register_handler(*this);
 *         signal_.set_enable_pins(0x3F);
 *         signal_.enable();
 *     }
 * 
 *     bool get_snapshot(uint8_t& snapshot) {
 *         // Wait for future result
 *         bool result = future_.get(snapshot);
 *         // Reset future
 *         future_.reset_();
 *         return result;
 *     }
 * 
 * private:
 *     void take_snapshot() {
 *         future_.set_future_value_(port_.get_PIN());
 *     }
 * 
 *     Future<uint8_t> future_;
 *     gpio::FastPort<board::Port::PORT_B> port_;
 *     interrupt::PCI_PORT_SIGNAL<board::Port::PORT_B> signal_;
 * 
 *     DECL_PCINT_ISR_FRIENDS
 * };
 * 
 * REGISTER_PCI_ISR_METHOD(PCINT, ButtonsSnapshot, &ButtonsSnapshot::take_snapshot, board::InterruptPin::D8_PB0_PCI0)
 * 
 * int main() {
 *     board::init();
 * 
 *     // Enable interrupts at startup time
 *     sei();
 * 
 *     // Initialize PORT and PCI through ButtonsSnapshot
 *     ButtonsSnapshot snapshot_taker;
 * 
 *     while (true) {
 *         // Wait for the future to be filled in by the PCINT0 ISR
 *         uint8_t snapshot = 0;
 *         if (snapshot_taker.get_snapshot(snapshot)) {
 *             // Do something with the obtained snapshot
 *         }
 *     }
 * }
 * @endcode
 */
namespace future
{
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
		 * The status of a Future immediately after it has been constructed.
		 * The Future will keep this status until:
		 * - either its output value provider has fully filled its value (then its
		 * status will become `READY`)
		 * - or its output value provider has reported an error to it (then its 
		 * status will become `ERROR`)
		 * 
		 * @sa Future.set_future_value_()
		 * @sa Future.set_future_finish()
		 * @sa Future.set_future_error()
		 */
		NOT_READY = 0,

		/**
		 * The status of a Future once its output value has been fully set by a 
		 * provider.
		 * 
		 * @sa Future.set_future_value()
		 * @sa Future.set_future_finish()
		 * @sa Future.get()
		 */
		READY,

		/**
		 * The status of a Future once a value provider has reported an error 
		 * to it.
		 * 
		 * @sa Future.set_future_error()
		 * @sa Future.error()
		 */
		ERROR,

		/**
		 * The status of a Future that has been moved, if it was `NOT_READY` before
		 * moving.
		 */
		INVALID
	};

	/// @cond notdocumented
	template<typename OSTREAM> OSTREAM& operator<<(OSTREAM& out, FutureStatus s)
	{
		// Conversion lambda for local usage
		auto convert = [](FutureStatus s)
		{
			switch (s)
			{
				case FutureStatus::NOT_READY:
				return F("NOT_READY");

				case FutureStatus::READY:
				return F("READY");

				case FutureStatus::ERROR:
				return F("ERROR");

				case FutureStatus::INVALID:
				return F("INVALID");

				default:
				return F("");
			}
		};
		return out << convert(s);
	}
	/// @endcond

	/// @cond notdocumented
	class AbstractFuture;
	extern void future_on_status_change_dispatch(const AbstractFuture&, FutureStatus);
	extern void future_on_output_change_dispatch(const AbstractFuture&);
	class AbstractFakeFuture;
	extern void future_on_status_change_dispatch(const AbstractFakeFuture&, FutureStatus);
	extern void future_on_output_change_dispatch(const AbstractFakeFuture&);
	/// @endcond

	/**
	 * Notification(s) dispatched by a Future.
	 * This is passed to Future constructor and detrmines whether the instantiated
	 * Future will dispatch of its own notification changes.
	 */
	enum class FutureNotification : uint8_t
	{
		/** No notification is dispatched by the Future. */
		NONE = 0,
		/** Notification is dispatched whenever the Future status changes. */
		STATUS = 1,
		/** Notification is dispatched whenever the Future output buffer gets filled, even partly. */
		OUTPUT = 2,
		/** 
		 * Notification is dispatched whenever the Future status changes or
		 * the Future output buffer gets filled, even partly.
		 */
		BOTH = 3
	};

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
		 * 
		 * @retval 0 if the Future is READY, i.e. holds a valid output value
		 * @return the actual error reported by a provider on this Future
		 * 
		 * @sa await()
		 * @sa set_future_error_()
		 * @sa errors::EINVAL
		 */
		int error() const
		{
			switch (await())
			{
				case FutureStatus::ERROR:
				return error_;

				case FutureStatus::READY:
				return 0;

				default:
				// This should never happen
				return errors::EINVAL;
			}
		}

		// Methods called by a Future consumer
		//=====================================

		/**
		 * Check the number of bytes remaining to read from this Future.
		 * This method is called by a Future input value consumer to know how many
		 * bytes remain to get read until the end of the input value.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @return the number of bytes to be read from the input value stored
		 * by this Future
		 * 
		 * @sa get_storage_value_()
		 */
		uint8_t get_storage_value_size_() const
		{
			return input_size_;
		}

		/**
		 * Get one byte from the input storage value of this Future.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next byte of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param chunk the byte reference that will receive the next byte of this
		 * Future input value
		 * @retval true if the next byte of this Future could be read successfully
		 * @retval false if all bytes of this Future input storage value have been
		 * read already
		 * 
		 * @sa get_storage_value_size_()
		 * @sa get_storage_value(uint8_t*, uint8_t)
		 * @sa Future::Future(const IN&)
		 */
		bool get_storage_value_(uint8_t& chunk)
		{
			// Check all bytes have not been transferred yet
			if (input_size_ == 0)
				return false;
			chunk = *input_current_++;
			--input_size_;
			return true;
		}

		/**
		 * Get @p size bytes from the input storage value of this Future.
		 * This method is called by a Future input value consumer to consume
		 * the input value held by a Future.
		 * Every call to this method will advance the Future internal pointer 
		 * to input data, so that next call will return the next chunk of data.
		 * Calling this method never changes the status of the Future, hence
		 * it is not possible to read the input value more than once.
		 * This method is useful only for `Future<?, T>` where `T` type is not 
		 * `void`.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param chunk a pointer to an array of at least @p size bytes, which will
		 * be filled with the next chunk of bytes of this Future input value
		 * @param size the number of bytes to get from the input storage value
		 * @retval true if the right amount bytes of this Future could be read 
		 * successfully
		 * @retval false if @p size is larger than the remaining number of bytes 
		 * to be read from the input storage value
		 * 
		 * @sa get_storage_value_size_()
		 * @sa Future::Future(const IN&)
		 * @sa get_storage_value_(uint8_t&)
		 */
		bool get_storage_value_(uint8_t* chunk, uint8_t size)
		{
			// Check size does not go beyond transferrable size
			if (size > input_size_)
				return false;
			memcpy(chunk, input_current_, size);
			input_current_ += size;
			input_size_ -= size;
			return true;
		}

		// Methods called by a Future supplier
		//=====================================

		/**
		 * Check the number of bytes remaining to write to the output value of
		 * this Future.
		 * This method is called by a Future output value producer to know how many
		 * bytes remain to write until the end of the output value.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @return the number of bytes to be written to the output value stored
		 * in this Future
		 * 
		 * @sa set_future_finish_()
		 * @sa set_future_value_()
		 */
		uint8_t get_future_value_size_() const
		{
			return output_size_;
		}

		/**
		 * Mark this Future as `FutureStatus::READY`.
		 * This method is called by a Future ouput value provider to indicate
		 * that a Future is ready for use.
		 * This method is useful only for `Future<void>` i.e. `Future`s that have 
		 * no output, but exist as a way to indicate the end of an asynchronous 
		 * process. For other `Future<T>`s, with a non `void` type `T`, you should
		 * use one of `set_future_value_` methods instead.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @retval false if this Future cannot be updated properly (because it is
		 * not in `FutureStatus::NOT_READY` or if it is still expecting data)
		 * @retval true if this Future has been properly updated
		 * 
		 * @sa status()
		 * @sa set_future_value_()
		 * @sa set_future_error_()
		 */
		bool set_future_finish_()
		{
			// Check this future is waiting for data
			if (status_ != FutureStatus::NOT_READY)
				return false;
			if (output_size_ == 0)
			{
				status_ = FutureStatus::READY;
				callback_status();
			}
			return true;
		}

		/**
		 * Add one byte to the output value content of this Future.
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
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param chunk the byte to append to this Future output value
		 * @retval true if @p chunk could be added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of this Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa status()
		 * @sa set_future_value_(const uint8_t*, uint8_t)
		 * @sa set_future_value_(const T&)
		 */
		bool set_future_value_(uint8_t chunk)
		{
			// Check this future is waiting for data
			if (status_ != FutureStatus::NOT_READY)
				return false;
			// Update Future value chunk
			*output_current_++ = chunk;
			// Is that the last chunk?
			--output_size_;
			callback_output();
			if (output_size_ == 0)
			{
				status_ = FutureStatus::READY;
				callback_status();
			}
			return true;
		}

		/**
		 * Add several bytes to the output value content of this Future.
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
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param chunk pointer to the first byte to be added to this Future output
		 * value
		 * @param size the number of bytes to be appended to this Future output value
		 * @retval true if @p chunk could be completely added to the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`, or when @p size
		 * additional bytes would make the output value bigger than expected
		 * 
		 * @sa status()
		 * @sa set_future_value_(uint8_t)
		 * @sa set_future_value_(const T&)
		 */
		bool set_future_value_(const uint8_t* chunk, uint8_t size)
		{
			// Check size does not go beyond expected size
			if (size > output_size_)
			{
				// Store error
				set_future_error_(errors::EMSGSIZE);
				return false;
			}
			while (size--)
			{
				if (!set_future_value_(*chunk++)) return false;
			}
			return true;
		}

		/**
		 * Set the output value content of this Future.
		 * This method is called by a Future ouput value provider to fully fill
		 * up, with the proper value, the output value of a Future.
		 * Calling this method will change the status of the Future to 
		 * `FutureStatus::READY`
		 * This method is useful only for `Future<T>` where `T` type is not `void`.
		 * 
		 * It is also possible to fill the output value byte per byte, with
		 * other overloaded versions of this method.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @tparam T the type of output value of this Future
		 * 
		 * @param value a constant reference to the value to set in this Future
		 * @retval true if @p value could be properly set into the future
		 * @retval false if this method failed; typically, when the current status
		 * of the target Future is not `FutureStatus::NOT_READY`
		 * 
		 * @sa status()
		 * @sa set_future_value_(uint8_t)
		 * @sa set_future_value_(const uint8_t*, uint8_t)
		 */
		template<typename T> bool set_future_value_(const T& value)
		{
			return set_future_value_(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
		}

		/**
		 * Mark this Future as `FutureStatus::ERROR`.
		 * This method is called by a Future ouput value provider to indicate
		 * that it cannot compute an output value for a given Future.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param error the error code to set for the Future
		 * @retval true if this Future has been properly updated
		 * @retval false if this Future cannot be updated properly (because it is
		 * not in `FutureStatus::NOT_READY`)
		 * 
		 * @sa status()
		 * @sa set_future_value_()
		 * @sa set_future_finish_()
		 */
		bool set_future_error_(int error)
		{
			// Check this future is waiting for data
			if ((error == 0) || (status_ != FutureStatus::NOT_READY))
				return false;
			error_ = error;
			status_ = FutureStatus::ERROR;
			callback_status();
			return true;
		}

	protected:
		/// @cond notdocumented
		// "default" constructor
		AbstractFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size,
			FutureNotification notifications = FutureNotification::NONE)
			:	output_data_{output_data}, output_current_{output_data}, output_size_{output_size},
				input_data_{input_data}, input_current_{input_data}, input_size_{input_size},
				notifications_{notifications} {}

		// these constructors are forbidden (subclass ctors shall call above move/copy ctor instead)
		AbstractFuture(const AbstractFuture&) = delete;
		AbstractFuture& operator=(const AbstractFuture&) = delete;
		AbstractFuture(AbstractFuture&&) = delete;
		AbstractFuture& operator=(AbstractFuture&&) = delete;

		~AbstractFuture() = default;

		// This method is called by subclasses to completely reset this future (for reuse from scratch)
		void reset_(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
		{
			status_ = FutureStatus::NOT_READY;
			error_ = 0;
			output_data_ = output_current_ = output_data;
			output_size_ = output_size;
			input_data_ = input_current_ = input_data;
			input_size_ = input_size;
		}

		// This method is called by subclass to check if input is replaceable,
		// i.e. it has not been read yet
		bool can_replace_input_() const
		{
			return (input_current_ == input_data_);
		}
		/// @endcond

	private:
		void callback_status()
		{
			if (uint8_t(notifications_) & uint8_t(FutureNotification::STATUS))
				future_on_status_change_dispatch(*this, status_);
		}
		
		void callback_output()
		{
			if (uint8_t(notifications_) & uint8_t(FutureNotification::OUTPUT))
				future_on_output_change_dispatch(*this);
		}
		
		volatile FutureStatus status_ = FutureStatus::NOT_READY;
		int error_ = 0;

		uint8_t* output_data_ = nullptr;
		uint8_t* output_current_ = nullptr;
		uint8_t output_size_ = 0;
		
		uint8_t* input_data_ = nullptr;
		uint8_t* input_current_ = nullptr;
		uint8_t input_size_ = 0;

		FutureNotification notifications_;

		template<typename F> friend class AbstractFuturesGroup;
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
	 * A Future can also hold an error code instead of a valid output value; that code
	 * is provided, when needed, by the asynchronous function computing the output
	 * value, if it encounters an unrecoverable error.
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

		/** Size of the output value of this Future. */
		static constexpr uint8_t OUT_SIZE = sizeof(OUT);
		/** Size of the input value of this Future. */
		static constexpr uint8_t IN_SIZE = sizeof(IN);

		/** 
		 * Construct a new Future.
		 * The created Future is in `FutureStatus::NOT_READY` status.
		 * The Future holds buffers to store both the input storage value and the 
		 * output value.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 * @param notifications determines if and which notifications should be
		 * dispatched by this Future; default is none.
		 * 
		 * @sa AbstractFuture
		 * @sa FutureNotification
		 * @sa status()
		 * @sa FutureStatus
		 */
		explicit Future(const IN& input = IN{}, FutureNotification notifications = FutureNotification::NONE)
			:	AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN), 
					notifications}, input_{input} {}

		/// @cond notdocumented
		~Future() = default;
		/// @endcond

		/**
		 * This method completely resets this future (for reuse from scratch), 
		 * as if it was just constructed. This allows reuse of a single future
		 * several times.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 */
		void reset_(const IN& input = IN{})
		{
			AbstractFuture::reset_(output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN));
			input_ = input;
		}

		/**
		 * Reset the input storage value held by this Future with a new value.
		 * This is possible only if no consumer has started reading the current 
		 * input storage value yet.
		 * 
		 * @warning This method is not synchronized, it shall be called exclusively
		 * from an ISR, or possibly from inside a `synchronized` block.
		 * 
		 * @param input a value to be copied to this Future input storage value;
		 * this argument does not exist when @p IN is `void`.
		 * @retval true if the input strage value has been successfully replaced
		 * @retval false if the input strage value could not be replaced because 
		 * a consumer already started reading the previous input storage value
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
		 * This method should never be called from an ISR as it is blocking for
		 * an undetermined time!
		 * 
		 * @param result a reference to a variable that will be assigned the output 
		 * value, once available; if an error occurred, result is unchanged. This
		 * argument does not exist when @p OUT is `void`
		 * @retval true if an output value has been set to this Future and it was
		 * assigned to @p result
		 * @retval false if an error was reported to this Future; the error can be
		 * obtained through `error()`.
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
			return true;
		}

	protected:
		/**
		 * Return the input storage value as it was initially set (or reset 
		 * through `reset_input_()`), whatever the current state of this Future.
		 * @sa reset_input_()
		 */
		const IN& get_input() const
		{
			return input_;
		}

		/**
		 * Return the input storage value as it was initially set (or reset 
		 * through `reset_input_()`), whatever the current state of this Future.
		 * @sa reset_input_()
		 */
		IN& get_input()
		{
			return input_;
		}

	private:
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

		static constexpr uint8_t OUT_SIZE = sizeof(OUT);
		static constexpr uint8_t IN_SIZE = 0;

		explicit Future(FutureNotification notifications = FutureNotification::NONE)
		: AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0, notifications} {}
		~Future() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_()
		{
			AbstractFuture::reset_(output_buffer_, sizeof(OUT), nullptr, 0);
		}

		// The following method is blocking until this Future is ready
		bool get(OUT& result)
		{
			if (await() != FutureStatus::READY)
				return false;
			result = output_;
			return true;
		}

	private:
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

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = sizeof(IN);

		explicit Future(const IN& input = IN{}, FutureNotification notifications = FutureNotification::NONE)
			: AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN), notifications}, input_{input} {}
		~Future() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_(const IN& input = IN{})
		{
			AbstractFuture::reset_(nullptr, 0, input_buffer_, sizeof(IN));
			input_ = input;
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
			return (await() == FutureStatus::READY);
		}

	protected:
		const IN& get_input() const
		{
			return input_;
		}
		IN& get_input()
		{
			return input_;
		}

	private:
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

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = 0;

		explicit Future(FutureNotification notifications = FutureNotification::NONE)
		: AbstractFuture{nullptr, 0,nullptr, 0, notifications} {}
		~Future() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_()
		{
			AbstractFuture::reset_(nullptr, 0, nullptr, 0);
		}

		// The following method is blocking until this Future is ready
		bool get()
		{
			return (await() == FutureStatus::READY);
		}
	};
	/// @endcond

	/**
	 * Base class for all `FakeFuture`s.
	 * A FakeFuture is an implementation for a specific Future that shall be used
	 * and completed within a single function.
	 * It is used only as an optimization surrogate for I2C devices working in
	 * synchronous mode, and should generally not be needed in applications.
	 * 
	 * @sa AbstractFuture
	 * @sa FakeFuture
	 */
	class AbstractFakeFuture
	{
	public:
		/// @cond notdocumented
		FutureStatus status() const
		{
			return (error_ == 0 ? FutureStatus::READY : FutureStatus::ERROR);
		}

		FutureStatus await() const
		{
			return status();
		}

		int error() const
		{
			return error_;
		}

		// Methods called by a Future consumer
		//=====================================

		uint8_t get_storage_value_size_() const
		{
			return input_size_;
		}

		bool get_storage_value_(uint8_t& chunk)
		{
			// Check all bytes have not been transferred yet
			chunk = *input_current_++;
			--input_size_;
			return true;
		}

		bool get_storage_value_(uint8_t* chunk, uint8_t size)
		{
			memcpy(chunk, input_current_, size);
			input_current_ += size;
			input_size_ -= size;
			return true;
		}

		// Methods called by a Future supplier
		//=====================================

		uint8_t get_future_value_size_() const
		{
			return output_size_;
		}

		bool set_future_finish_()
		{
			callback_status();
			return true;
		}

		bool set_future_value_(uint8_t chunk)
		{
			// Update Future value chunk
			*output_current_++ = chunk;
			--output_size_;
			callback_output();
			return true;
		}

		bool set_future_value_(const uint8_t* chunk, uint8_t size)
		{
			while (size--)
			{
				set_future_value_(*chunk++);
			}
			return true;
		}

		template<typename T> bool set_future_value_(const T& value)
		{
			return set_future_value_(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
		}

		bool set_future_error_(int error)
		{
			error_ = error;
			callback_status();
			return true;
		}

		/// @endcond
		
	protected:
		/// @cond notdocumented
		// "default" constructor
		AbstractFakeFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size,
			FutureNotification notifications = FutureNotification::NONE)
			:	output_current_{output_data}, output_size_{output_size},
				input_current_{input_data}, input_size_{input_size},
				notifications_{notifications} {}

		// these constructors are forbidden (subclass ctors shall call above move/copy ctor instead)
		AbstractFakeFuture(const AbstractFakeFuture&) = delete;
		AbstractFakeFuture& operator=(const AbstractFakeFuture&) = delete;
		AbstractFakeFuture(AbstractFakeFuture&&) = delete;
		AbstractFakeFuture& operator=(AbstractFakeFuture&&) = delete;

		~AbstractFakeFuture() = default;

		// This method is called by subclasses to completely reset this future (for reuse from scratch)
		void reset_(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
		{
			error_ = 0;
			output_current_ = output_data;
			output_size_ = output_size;
			input_current_ = input_data;
			input_size_ = input_size;
		}

		// This method is called by subclass to check if input is replaceable,
		// i.e. it has not been read yet
		bool can_replace_input_() const
		{
			return true;
		}
		/// @endcond

	private:
		void callback_status()
		{
			if (uint8_t(notifications_) & uint8_t(FutureNotification::STATUS))
				future_on_status_change_dispatch(*this, status());
		}
		
		void callback_output()
		{
			if (uint8_t(notifications_) & uint8_t(FutureNotification::OUTPUT))
				future_on_output_change_dispatch(*this);
		}

		int error_ = 0;

		uint8_t* output_current_ = nullptr;
		uint8_t output_size_ = 0;
		
		uint8_t* input_current_ = nullptr;
		uint8_t input_size_ = 0;

		FutureNotification notifications_;

		template<typename F> friend class AbstractFuturesGroup;
	};

	/**
	 * Actual FakeFuture, it has the exact same API as Future and can be used in 
	 * lieu of Future.
	 * 
	 * It is used only as an optimization surrogate for I2C devices working in
	 * synchronous mode, and should generally not be needed in applications.
	 */
	template<typename OUT_ = void, typename IN_ = void>
	class FakeFuture : public AbstractFakeFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		/// @cond notdocumented
		using OUT = OUT_;
		using IN = IN_;

		static constexpr uint8_t OUT_SIZE = sizeof(OUT);
		static constexpr uint8_t IN_SIZE = sizeof(IN);

		explicit FakeFuture(const IN& input = IN{}, 
			FutureNotification notifications = FutureNotification::NONE)
			:	AbstractFakeFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN),
				notifications}, input_{input} {}

		~FakeFuture() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_(const IN& input = IN{})
		{
			AbstractFakeFuture::reset_(output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN));
			input_ = input;
		}

		bool reset_input_(const IN& input)
		{
			input_ = input;
			return true;
		}

		bool get(OUT& result)
		{
			result = output_;
			return true;
		}

	protected:
		const IN& get_input() const
		{
			return input_;
		}
		IN& get_input()
		{
			return input_;
		}
		/// @endcond

	private:
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

	/// @cond notdocumented	
	template<typename OUT_>
	class FakeFuture<OUT_, void> : public AbstractFakeFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");

	public:
		using OUT = OUT_;
		using IN = void;

		static constexpr uint8_t OUT_SIZE = sizeof(OUT);
		static constexpr uint8_t IN_SIZE = 0;

		explicit FakeFuture(FutureNotification notifications = FutureNotification::NONE)
			:	AbstractFakeFuture{output_buffer_, sizeof(OUT), nullptr, 0,
				notifications} {}

		~FakeFuture() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_()
		{
			AbstractFakeFuture::reset_(output_buffer_, sizeof(OUT), nullptr, 0);
		}

		bool get(OUT& result)
		{
			result = output_;
			return true;
		}

	private:
		union
		{
			OUT output_;
			uint8_t output_buffer_[sizeof(OUT)];
		};
	};
	/// @endcond

	/// @cond notdocumented	
	template<typename IN_>
	class FakeFuture<void, IN_> : public AbstractFakeFuture
	{
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		using OUT = void;
		using IN = IN_;

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = sizeof(IN);

		explicit FakeFuture(const IN& input = IN{}, FutureNotification notifications = FutureNotification::NONE)
			: AbstractFakeFuture{nullptr, 0, input_buffer_, sizeof(IN), notifications}, input_{input} {}

		~FakeFuture() = default;

		// This method completely resets this future (for reuse from scratch)
		void reset_(const IN& input = IN{})
		{
			AbstractFakeFuture::reset_(nullptr, 0, input_buffer_, sizeof(IN));
			input_ = input;
		}

		bool reset_input_(const IN& input)
		{
			input_ = input;
			return true;
		}

		bool get()
		{
			return true;
		}

	protected:
		const IN& get_input() const
		{
			return input_;
		}
		IN& get_input()
		{
			return input_;
		}

	private:
		union
		{
			IN input_;
			uint8_t input_buffer_[sizeof(IN)];
		};
	};
	/// @endcond

	/// @cond notdocumented	
	template<>
	class FakeFuture<void, void> : public AbstractFakeFuture
	{
	public:
		using OUT = void;
		using IN = void;

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = 0;

		explicit FakeFuture(FutureNotification notifications = FutureNotification::NONE)
			: AbstractFakeFuture{nullptr, 0, nullptr, 0, notifications} {}

		~FakeFuture() = default;

		void reset_()
		{
			AbstractFakeFuture::reset_(nullptr, 0, nullptr, 0);
		}

		bool get()
		{
			return true;
		}
	};
	/// @endcond

	/// @cond notdocumented
	// Specific traits for futures
	template<typename F> struct Future_trait
	{
		static constexpr const bool IS_FUTURE = false;
		static constexpr const bool IS_ABSTRACT = false;
		static constexpr const bool IS_FAKE = false;
	};
	template<> struct Future_trait<AbstractFuture>
	{
		static constexpr const bool IS_FUTURE = true;
		static constexpr const bool IS_ABSTRACT = true;
		static constexpr const bool IS_FAKE = false;
	};
	template<typename OUT, typename IN> struct Future_trait<Future<OUT, IN>>
	{
		static constexpr const bool IS_FUTURE = true;
		static constexpr const bool IS_ABSTRACT = false;
		static constexpr const bool IS_FAKE = false;
	};
	template<> struct Future_trait<AbstractFakeFuture>
	{
		static constexpr const bool IS_FUTURE = true;
		static constexpr const bool IS_ABSTRACT = true;
		static constexpr const bool IS_FAKE = true;
	};
	template<typename OUT, typename IN> struct Future_trait<FakeFuture<OUT, IN>>
	{
		static constexpr const bool IS_FUTURE = true;
		static constexpr const bool IS_ABSTRACT = false;
		static constexpr const bool IS_FAKE = true;
	};
	/// @endcond

	/**
	 * Abstract class to allow aggregation of several futures.
	 * This allows to `await()` for all futures, or query the overall `status()`
	 * of the group.
	 * 
	 * The following snippet shows how this must be used to create an actual group 
	 * of futures:
	 * @code
	 * class MyGroup : public AbstractFuturesGroup<AbstractFuture>
	 * {
	 * public:
	 *     MyGroup() : AbstractFuturesGroup<AbstractFuture>{{&f1_, &f2_, &f3_}}, f1_{}, f2_{}, f3_{} {
	 *         interrupt::register_handler(*this);
	 *     }
	 *     ~MyGroup() {
	 *         interrupt::unregister_handler(*this);
	 *     }
	 * 
	 *     F1& get_f1() { return f1_; }
	 *     F2& get_f2() { return f2_; }
	 *     F3& get_f3() { return f3_; }
	 * 
	 * private:
	 *     void on_status_change(const AbstractFuture& future, FutureStatus status) {
	 *         // Check future is one of our own futures
	 *         if (&future == &f1_ || &future == &f2_ || &future == &f3_) {
	 * 				on_status_change_pre_step(future, status);
	 *              // Any further specific processing goes here...
	 *         }
	 *     }
	 * 
	 *     MyFuture1 f1_;
	 *     MyFuture2 f2_;
	 *     MyFuture3 f3_;
	 * };
	 * ...
	 * // Register all future notification listeners
	 * REGISTER_FUTURE_STATUS_LISTENERS(AbstractFuture, MyGroup)
	 * @endcode
	 * In that snippet, `MyGroup` embeds 3 different futures, each of a different type;
	 * the 3 futures are constructed at `MyGroup` construction time, and their pointers
	 * passed to the parent `FuturesGroup`.
	 * Three getter methods allow the application to access individual futures.
	 * 
	 * Note the calls to `interrupt::register_handler()` and `interrupt::unregister_handler()`
	 * in the constructor and the destructor.
	 * 
	 * Also note the `on_status_change()` method, that will be called whenever any
	 * individual future has its status modified.
	 * 
	 * Finally, for the notification mechanism to work `MyGroup` must be registered
	 * as a listener, which is done with `REGISTER_FUTURE_STATUS_LISTENERS()`.
	 * 
	 * @tparam F the type of Future to aggregate int this group; this shall be
	 * either `AbstractFuture` or `AbstractFakeFuture`.
	 */
	template<typename F> class AbstractFuturesGroup : public F
	{
		static_assert(Future_trait<F>::IS_FUTURE, "F must be a Future");
		static_assert(Future_trait<F>::IS_ABSTRACT, "F must be an abstract Future");

	protected:
		/**
		 * Specific size value indicating this group has an unlimited (and unknown 
		 * at construction time) number of futures to handle.
		 * @sa init()
		 */
		static constexpr const uint16_t NO_LIMIT = 0xFFFFU;
		
		/** 
		 * Construct a new AbstractFuturesGroup.
		 * The created Group is in `FutureStatus::NOT_READY` status.
		 * The Future holds buffers to store both the input storage value and the 
		 * output value.
		 * The subclass constructor **must** call `init()` with the list of futures
		 * in this group.
		 * 
		 * @param notifications determines if and which notifications should be
		 * dispatched by this Future; default is none.
		 * 
		 * @sa init()
		 * @sa FutureNotification
		 * @sa FutureStatus
		 */
		explicit AbstractFuturesGroup(FutureNotification notifications = FutureNotification::NONE)
			:	F{nullptr, 0, nullptr, 0, notifications} {}

		/**
		 * Called from constructors of subclasses, this method allows this group
		 * to listen for the status of all its futures.
		 * @param futures list of pointers to futures in this group
		 * @param actual_size real number of futures; if `0` (default), this will
		 * match the size of @p futures; it may be bigger than the actual number of
		 * futures if e.g. a future is reused several times in this group; if
		 * `NO_LIMIT`, then the subclass does not know in advance how many times
		 * its futures shall be used, in this case, the subclass must itself
		 * indicate when this group of future is `FutureStatus::READY`.
		 * 
		 * @sa FutureStatus
		 * @sa Future::set_future_finish()
		 */
		void init(utils::range<F*> futures, uint16_t actual_size = 0)
		{
			num_ready_ = (actual_size != 0 ? actual_size : futures.size());
			for (F* future: futures)
				future->notifications_ = FutureNotification::STATUS;
		}

		/**
		 * This must be called by subclasses `on_status_change()` method, after
		 * checking that @p future is one of the futures handled by this instance,
		 * and before performing any further processing.
		 * 
		 * @param future a reference to the future which status has changed
		 * @param status the new status of @p future
		 */
		void on_status_change_pre_step(const F& future, FutureStatus status)
		{
			switch (status)
			{
				case FutureStatus::ERROR:
				this->set_future_error_(future.error());
				break;

				case FutureStatus::INVALID:
				this->set_future_error_(errors::EINVAL);
				break;

				case FutureStatus::READY:
				if (--num_ready_ == 0)
					this->set_future_finish_();
				break;

				default:
				break;
			}
		}

	private:
		uint16_t num_ready_ = 0;
	};

	/// @cond notdocumented
	template<typename F>
	struct dispatch_handler_impl
	{
		template<bool DUMMY> static void future_on_status_change_helper(const F&, FutureStatus)
		{
			// Intentionally blank (last recursive call)
		}

		template<bool DUMMY, typename HANDLER1_, typename... HANDLERS_> 
		static void future_on_status_change_helper(const F& future, FutureStatus status)
		{
			HANDLER1_* handler = interrupt::HandlerHolder<HANDLER1_>::handler();
			if (handler != nullptr)
				handler->on_status_change(future, status);
			// handle other handlers
			future_on_status_change_helper<true, HANDLERS_...>(future, status);
		}

		template<typename... HANDLERS_>
		static void future_on_status_change(const F& future, FutureStatus status)
		{
			// Ask each registered listener to handle on_status_change() if concerned
			future_on_status_change_helper<true, HANDLERS_...>(future, status);
		}

		template<bool DUMMY> static void future_on_output_change_helper(const F&)
		{
			// Intentionally blank (last recursive call)
		}

		template<bool DUMMY, typename HANDLER1_, typename... HANDLERS_> 
		static void future_on_output_change_helper(const F& future)
		{
			HANDLER1_* handler = interrupt::HandlerHolder<HANDLER1_>::handler();
			if (handler != nullptr)
				handler->on_output_change(future);
			// handle other handlers
			future_on_output_change_helper<true, HANDLERS_...>(future);
		}

		template<typename... HANDLERS_>
		static void future_on_output_change(const F& future)
		{
			// Ask each registered listener to handle on_output_change() if concerned
			future_on_output_change_helper<true, HANDLERS_...>(future);
		}
	};

	template<typename F1, typename F2>
	struct dispatch_handler
	{
		template<typename... HANDLERS_>
		static void future_on_status_change(const F2&, FutureStatus)
		{
			// Intentionally blank (tasks performed only on template specialization for actual futures)
		}
		template<typename... HANDLERS_>
		static void future_on_output_change(const F2&)
		{
			// Intentionally blank (tasks performed only on template specialization for actual futures)
		}
	};

	template<>
	struct dispatch_handler<AbstractFuture, AbstractFuture>
	{
		template<typename... HANDLERS_>
		static void future_on_status_change(const AbstractFuture& future, FutureStatus status)
		{
			dispatch_handler_impl<AbstractFuture>::future_on_status_change<HANDLERS_...>(
				future, status);
		}
		template<typename... HANDLERS_>
		static void future_on_output_change(const AbstractFuture& future)
		{
			dispatch_handler_impl<AbstractFuture>::future_on_output_change<HANDLERS_...>(future);
		}
	};

	template<>
	struct dispatch_handler<AbstractFakeFuture, AbstractFakeFuture>
	{
		template<typename... HANDLERS_>
		static void future_on_status_change(const AbstractFuture& future, FutureStatus status)
		{
			dispatch_handler_impl<AbstractFakeFuture>::future_on_status_change<HANDLERS_...>(
				future, status);
		}
		template<typename... HANDLERS_>
		static void future_on_output_change(const AbstractFakeFuture& future)
		{
			dispatch_handler_impl<AbstractFakeFuture>::future_on_output_change<HANDLERS_...>(future);
		}
	};
	/// @endcond 
}

#endif /* FUTURE_HH */
/// @endcond
