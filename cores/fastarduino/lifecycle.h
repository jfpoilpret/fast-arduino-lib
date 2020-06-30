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
 * Utility API to handle lifecycle of objects so that:
 * - objects can be registered and identified with a repository
 * - objects can be removed from the repository
 * - objects can be moved around and still be properyl referenced by their repository
 */
#ifndef LIFECYCLE_HH
#define LIFECYCLE_HH

#include <string.h>
#include "move.h"
#include "defines.h"
#include "utilities.h"

/**
 * Contains the API around LifeCycle management implementation.
 * This concept is here to be extended with concrete needs (e.g. Futures).
 * The API is based on the follwowing:
 * - LifeCycleManager is a repository of some kinf of objects
 * - A LifeCycleManager has a limited number of managed objects
 * - You can define different LifeCycleManager subclasses and create as many 
 * instances as you need
 * - To have its lifecycle managed, an object must have a type that subclasses 
 * LifeCycle class.
 */
namespace lifecycle
{
	/// @cond notdocumented
	// Forward declarations
	class LifeCycle;
	/// @endcond

	class AbstractLifecycleManager
	{
	protected:
		/// @cond notdocumented
		AbstractLifecycleManager(LifeCycle** slots, uint8_t size)
			: size_{size}, slots_{slots}
		{
			for (uint8_t i = 0; i < size_; ++i)
				slots_[i] = nullptr;
		}

		AbstractLifecycleManager(const AbstractLifecycleManager&) = delete;
		AbstractLifecycleManager& operator=(const AbstractLifecycleManager&) = delete;
		~AbstractLifecycleManager() = default;
		/// @endcond

		bool register_object_(LifeCycle& object);

		bool remove_object_(uint8_t id)
		{
			// Check id is registered
			LifeCycle** slot = find_slot_(id);
			if (slot == nullptr) return false;
			*slot = nullptr;
			last_removed_id_ = id;
			return true;
		}

		LifeCycle* find_object_(uint8_t id) const
		{
			LifeCycle** slot = find_slot_(id);
			if (slot != nullptr)
				return *slot;
			else
				return nullptr;
		}

		uint8_t available_slots_() const
		{
			uint8_t free = 0;
			for (uint8_t i = 0; i < size_; ++i)
				if (slots_[i] == nullptr)
					++free;
			return free;
		}

	private:
		LifeCycle** find_slot_(uint8_t id) const
		{
			if ((id == 0) || (id > size_))
				return nullptr;
			return &slots_[id - 1];
		}

		bool register_at_index_(LifeCycle& future, uint8_t index);

		// Called by LifeCycle themselves (on move construction and assignment)
		bool move_object_(uint8_t id, LifeCycle* new_address)
		{
			// Check id is registered
			LifeCycle** slot = find_slot_(id);
			if (slot == nullptr) return false;
			*slot = new_address;
			return true;
		}

		const uint8_t size_;
		LifeCycle** slots_;
		uint8_t last_removed_id_ = 0;

		friend class LifeCycle;
	};

	// We may also define copyable and moveable lifecycle instances as subclasses
	// The minimum required is to have id_ and manager_ members
	class LifeCycle
	{
	public:
		LifeCycle() = default;
		LifeCycle(const LifeCycle&) = delete;

	//TODO or protected?
	private:
		uint8_t id_ = 0;
		AbstractLifecycleManager* manager_ = nullptr;

		friend class AbstractLifecycleManager;
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

#endif /* LIFECYCLE_HH */
/// @endcond
