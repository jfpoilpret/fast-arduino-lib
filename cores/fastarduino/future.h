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
#include "array.h"
#include "errors.h"
#include "move.h"
#include "streams.h"
#include "time.h"

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
 *         future_ = std::move(Future<uint8_t>{});
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
	streams::ostream& operator<<(streams::ostream& out, future::FutureStatus s);
	/// @endcond

	/**
	 * Listener is the interface allowing to listen to changes of a Future status.
	 * This may be added to any Future at construction time.
	 * 
	 * @tparam F the type of Future to listen to; this may be `AbstractFuture`,
	 * `AbstractFakeFuture`, or any subclass of these.
	 * 
	 * @sa Future
	 * @sa FutureStatus
	 * @sa FutureOutputListener
	 */
	template<typename F> class FutureStatusListener
	{
	protected:
		/**
		 * Called whenever a listened-to Future changes its FutureStatus.
		 * @param future a reference to the Future which status has changed
		 * @param new_status the new FutureStatus of @p future
		 */
		virtual void on_status_change(const F& future, FutureStatus new_status) = 0;
		friend F;
	};

	/**
	 * Listener is the interface allowing to listen to changes of a Future output
	 * buffer (while being fed by a producer).
	 * This may be added to any Future at construction time.
	 * 
	 * @tparam F the type of Future to listen to; this may be `AbstractFuture`,
	 * `AbstractFakeFuture`, or any subclass of these.
	 * 
	 * @sa Future
	 * @sa FutureStatus
	 * @sa FutureStatusListener
	 */
	template<typename F> class FutureOutputListener
	{
	protected:
		/**
		 * Called whenever a listened-to Future has its output buffer changed.
		 * This is called for every byte added to the output buffer.
		 * @param future a reference to the Future which output has been updated
		 * @param output_data pointer to the beginning of @p future output buffer;
		 * will be `null` if @p future is an `AbstractFakeFuture` or subclass.
		 * @param output_current pointer to the next byte to be written to 
		 * @p future output buffer; `*(9)output_current - 1)` points to the byte
		 * newly added.
		 */
		virtual void on_output_change(const F& future, uint8_t* output_data, uint8_t* output_current) = 0;
		friend F;
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

		/**
		 * The type to use for status listeners for this type of Future.
		 * @sa FutureStatusListener
		 */
		using STATUS_LISTENER = FutureStatusListener<AbstractFuture>;

		/**
		 * The type to use for output listeners for this type of Future.
		 * @sa FutureOutputListener
		 */
		using OUTPUT_LISTENER = FutureOutputListener<AbstractFuture>;

	protected:
		/// @cond notdocumented
		// "default" constructor
		AbstractFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size,
			STATUS_LISTENER* status_listener = nullptr,	OUTPUT_LISTENER* output_listener = nullptr)
			:	output_data_{output_data}, output_current_{output_data}, output_size_{output_size},
				input_data_{input_data}, input_current_{input_data}, input_size_{input_size},
				status_listener_{status_listener}, output_listener_{output_listener} {}

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

		// This method is called by subclass in their move constructor and assignment operator
		void move_(AbstractFuture&& that, uint8_t full_output_size, uint8_t full_input_size)
		{
			// Now copy all attributes from rhs (output_data_ was already initialized when this was constructed)
			status_listener_ = that.status_listener_;
			output_listener_ = that.output_listener_;

			const FutureStatus status = that.status_;
			status_ = status;
			// Make rhs Future invalid
			if (status == FutureStatus::NOT_READY)
				that.status_ = FutureStatus::INVALID;

			error_ = that.error_;
			const uint8_t output_size = that.output_size_;
			// Calculate data pointer attribute for next set value calls
			output_current_ = output_data_ + full_output_size - output_size;
			output_size_ = output_size;

			const uint8_t input_size = that.input_size_;
			// Calculate data pointer attribute for next set value calls
			input_current_ = input_data_ + full_input_size - input_size;
			input_size_ = input_size;
		}
		/// @endcond

	private:
		void callback_status()
		{
			if (status_listener_ != nullptr)
				status_listener_->on_status_change(*this, status_);
		}
		
		void callback_output()
		{
			if (output_listener_ != nullptr)
				output_listener_->on_output_change(*this, output_data_, output_current_);
		}
		
		volatile FutureStatus status_ = FutureStatus::NOT_READY;
		int error_ = 0;

		uint8_t* output_data_ = nullptr;
		uint8_t* output_current_ = nullptr;
		uint8_t output_size_ = 0;
		
		uint8_t* input_data_ = nullptr;
		uint8_t* input_current_ = nullptr;
		uint8_t input_size_ = 0;

		STATUS_LISTENER* status_listener_ = nullptr;
		OUTPUT_LISTENER* output_listener_ = nullptr;

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
		 * @param status_listener an optional listener to status changes on this 
		 * future
		 * @param output_listener an optional listener to output buffer changes on 
		 * this future
		 * 
		 * @sa AbstractFuture
		 * @sa status()
		 * @sa FutureStatus
		 */
		explicit Future(const IN& input = IN{}, STATUS_LISTENER* status_listener = nullptr,
			OUTPUT_LISTENER* output_listener = nullptr)
			:	AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN), 
					status_listener, output_listener}, input_{input} {}

		/// @cond notdocumented
		~Future() = default;
		Future(Future&& that) : AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}
		{
			move(std::move(that));
		}
		Future& operator=(Future&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		Future(const Future&) = delete;
		Future& operator=(const Future&) = delete;
		/// @endcond

		//TODO DOC
		// This method completely resets this future (for reuse from scratch)
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
		void move(Future&& that)
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

		static constexpr uint8_t OUT_SIZE = sizeof(OUT);
		static constexpr uint8_t IN_SIZE = 0;

		Future(STATUS_LISTENER* status_listener = nullptr, OUTPUT_LISTENER* output_listener = nullptr)
		: AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0, status_listener, output_listener} {}
		~Future() = default;

		Future(Future&& that) : AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0}
		{
			move(std::move(that));
		}
		Future& operator=(Future&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		Future(const Future&) = delete;
		Future& operator=(const Future&) = delete;

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
		void move(Future&& that)
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

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = sizeof(IN);

		explicit Future(const IN& input = IN{}, STATUS_LISTENER* status_listener = nullptr)
			: AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN), status_listener}, input_{input} {}
		~Future() = default;

		Future(Future&& that) : AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN)}
		{
			move(std::move(that));
		}
		Future& operator=(Future&& that)
		{
			if (this == &that) return *this;
			move(std::move(that));
			return *this;
		}

		Future(const Future&) = delete;
		Future& operator=(const Future&) = delete;

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
		void move(Future&& that)
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

		static constexpr uint8_t OUT_SIZE = 0;
		static constexpr uint8_t IN_SIZE = 0;

		Future(STATUS_LISTENER* status_listener = nullptr)
		: AbstractFuture{nullptr, 0,nullptr, 0, status_listener} {}
		~Future() = default;

		Future(Future&& that) : AbstractFuture{nullptr, 0, nullptr, 0}
		{
			synchronized move_(std::move(that), 0, 0);
		}
		Future& operator=(Future&& that)
		{
			if (this == &that) return *this;
			synchronized move_(std::move(that), 0, 0);
			return *this;
		}

		Future(const Future&) = delete;
		Future& operator=(const Future&) = delete;

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
			//TODO Check if this affects current usage!
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

		using STATUS_LISTENER = FutureStatusListener<AbstractFakeFuture>;
		using OUTPUT_LISTENER = FutureOutputListener<AbstractFakeFuture>;
		/// @endcond
		
	protected:
		/// @cond notdocumented
		// "default" constructor
		AbstractFakeFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size,
			STATUS_LISTENER* status_listener = nullptr, OUTPUT_LISTENER* output_listener = nullptr)
			:	output_current_{output_data}, output_size_{output_size},
				input_current_{input_data}, input_size_{input_size},
				status_listener_{status_listener}, output_listener_{output_listener} {}

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
			if (status_listener_ != nullptr)
				status_listener_->on_status_change(*this, status());
		}
		
		void callback_output()
		{
			if (output_listener_ != nullptr)
				output_listener_->on_output_change(*this, nullptr, output_current_);
		}

		int error_ = 0;

		uint8_t* output_current_ = nullptr;
		uint8_t output_size_ = 0;
		
		uint8_t* input_current_ = nullptr;
		uint8_t input_size_ = 0;

		STATUS_LISTENER* status_listener_ = nullptr;
		OUTPUT_LISTENER* output_listener_ = nullptr;

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
			STATUS_LISTENER* status_listener = nullptr, OUTPUT_LISTENER* output_listener = nullptr)
			:	AbstractFakeFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN),
				status_listener, output_listener}, input_{input} {}

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

		explicit FakeFuture(STATUS_LISTENER* status_listener = nullptr, OUTPUT_LISTENER* output_listener = nullptr)
			:	AbstractFakeFuture{output_buffer_, sizeof(OUT), nullptr, 0,
				status_listener, output_listener} {}

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

		explicit FakeFuture(const IN& input = IN{}, STATUS_LISTENER* status_listener = nullptr)
			: AbstractFakeFuture{nullptr, 0, input_buffer_, sizeof(IN), status_listener}, input_{input} {}

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

		explicit FakeFuture(STATUS_LISTENER* status_listener = nullptr)
			: AbstractFakeFuture{nullptr, 0, nullptr, 0, status_listener} {}

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
	 * The following snippet shows how this must be sued to create an actual group 
	 * of futures:
	 * @code
	 * class MyGroup : public AbstractFuturesGroup<AbstractFuture>
	 * {
	 * public:
	 *     MyGroup() : AbstractFuturesGroup<AbstractFuture>{{&f1_, &f2_, &f3_}}, f1_{}, f2_{}, f3_{} {}
	 * 
	 *     F1& get_f1() { return f1_; }
	 *     F2& get_f2() { return f2_; }
	 *     F3& get_f3() { return f3_; }
	 * 
	 * private:
	 *     MyFuture1 f1_;
	 *     MyFuture2 f2_;
	 *     MyFuture3 f3_;
	 * };
	 * @endcode
	 * In that snippet, `MyGroup` embeds 3 different futures, each of a different type;
	 * the 3 futures are constructed at `MyGroup` construction time, and their pointers
	 * passed to the parent `FuturesGroup`.
	 * Three getter mehtods allow teh application to access individual futures.
	 * 
	 * @tparam F the type of Future to aggregate int this group; this shall be
	 * either `AbstractFuture` or `AbstractFakeFuture`.
	 * @tparam SIZE the number of futures in this group
	 */
	template<typename F> class AbstractFuturesGroup : public F, public FutureStatusListener<F>
	{
		static_assert(Future_trait<F>::IS_FUTURE, "F must be a Future");
		static_assert(Future_trait<F>::IS_ABSTRACT, "F must be an abstract Future");

	public:
		/**
		 * The type to use for status listeners for this type of Future.
		 * @sa FutureStatusListener
		 */
		using STATUS_LISTENER = FutureStatusListener<F>;

	protected:
		//TODO DOCS
		explicit AbstractFuturesGroup(STATUS_LISTENER* status_listener = nullptr)
			:	F{nullptr, 0, nullptr, 0, status_listener} {}

		// This must be called from subclass constructor
		//TODO DOCS
		void init(std::initializer_list<F*> futures)
		{
			num_ready_ = futures.size();
			for (F* future: futures)
				future->status_listener_ = this;
		}
		/// @cond notdocumented
		AbstractFuturesGroup(AbstractFuturesGroup&&) = delete;
		AbstractFuturesGroup& operator=(AbstractFuturesGroup&&) = delete;
		AbstractFuturesGroup(const AbstractFuturesGroup&) = delete;
		AbstractFuturesGroup& operator=(const AbstractFuturesGroup&) = delete;
		/// @endcond

	private:
		void on_status_change(const F& future, FutureStatus status) final
		{
			switch (status)
			{
				case FutureStatus::ERROR:
				this->set_future_error_(future.error());
				break;

				case FutureStatus::INVALID:
				this->set_future_error_(errors::EILSEQ);
				break;

				case FutureStatus::READY:
				if (--num_ready_ == 0)
					this->set_future_finish_();
				break;

				default:
				break;
			}
		}

		uint8_t num_ready_ = 0;
	};
}

#endif /* FUTURE_HH */
/// @endcond
