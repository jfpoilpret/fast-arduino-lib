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
 * Common API used by implementations that handle the concept of futures.
 * For general discussion about this concept, please check 
 * https://en.wikipedia.org/wiki/Futures_and_promises
 */
#ifndef FUTURE_COMMONS_HH
#define FUTURE_COMMONS_HH

#include <string.h>
#include "errors.h"
#include "streams.h"
#include "time.h"

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
	 * Abstract Base class for all diverse implementations of `Future`s.
	 * This defines most API of a Future.
	 * 
	 * @sa Future
	 */
	class AbstractBaseFuture
	{
	public:
		/**
		 * The unique ID of this Future, as provided by the `FutureManager` 
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
		AbstractBaseFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
			:	output_data_{output_data}, output_current_{output_data}, output_size_{output_size},
				input_data_{input_data}, input_current_{input_data}, input_size_{input_size} {}
		~AbstractBaseFuture() = default;

		AbstractBaseFuture(const AbstractBaseFuture&) = delete;
		AbstractBaseFuture& operator=(const AbstractBaseFuture&) = delete;
		AbstractBaseFuture(AbstractBaseFuture&&) = delete;
		AbstractBaseFuture& operator=(AbstractBaseFuture&&) = delete;

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
		/// @endcond

		friend class AbstractBaseFutureManager;
		friend class AbstractMultiFutureManager;
	};

	/**
	 * This is the parent of all FutureManager implementations, it proiveds many 
	 * protected utilities for actual implementations.
	 * You should normally never need to subclass it.
	 * 
	 * @sa AbstractFutureManager
	 */
	class AbstractBaseFutureManager
	{
	protected:
		static uint8_t get_future_value_size_(const AbstractBaseFuture* future)
		{
			return (future == nullptr ? 0 : future->get_output_size_());
		}
		static bool set_future_finish_(AbstractBaseFuture* future)
		{
			return (future == nullptr ? false : future->set_finish_());
		}
		static bool set_future_value_(AbstractBaseFuture* future, uint8_t chunk)
		{
			return (future == nullptr ? false : future->set_chunk_(chunk));
		}
		static bool set_future_value_(AbstractBaseFuture* future, const uint8_t* chunk, uint8_t size)
		{
			return (future == nullptr ? false : future->set_chunk_(chunk, size));
		}
		static bool set_future_error_(AbstractBaseFuture* future, int error)
		{
			return (future == nullptr ? false : future->set_error_(error));
		}
		static uint8_t get_storage_value_size_(const AbstractBaseFuture* future)
		{
			return (future == nullptr ? 0 : future->get_input_size_());
		}
		static bool get_storage_value_(AbstractBaseFuture* future, uint8_t& chunk)
		{
			return (future == nullptr ? false : future->get_chunk_(chunk));
		}
		static bool get_storage_value_(AbstractBaseFuture* future, uint8_t* chunk, uint8_t size)
		{
			return (future == nullptr ? false : future->get_chunk_(chunk, size));
		}

		/// @cond notdocumented
		AbstractBaseFutureManager() = default;
		AbstractBaseFutureManager(const AbstractBaseFutureManager&) = delete;
		AbstractBaseFutureManager& operator=(const AbstractBaseFutureManager&) = delete;
		AbstractBaseFutureManager(AbstractBaseFutureManager&&) = delete;
		AbstractBaseFutureManager& operator=(AbstractBaseFutureManager&&) = delete;
		~AbstractBaseFutureManager() = default;
		/// @endcond
	};

	/// @cond notdocumented
	// Forward declaration
	class AbstractMultiFutureManager;
	/// @endcond

	//TODO DOC
	//TODO class definition and position
	class AbstractManagedFuture : public AbstractBaseFuture
	{
	protected:
		/// @cond notdocumented
		// Constructor used by FutureManager
		AbstractManagedFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
			:	AbstractBaseFuture{output_data, output_size, input_data, input_size} {}
		~AbstractManagedFuture();

		AbstractManagedFuture(const AbstractManagedFuture&) = delete;
		AbstractManagedFuture& operator=(const AbstractManagedFuture&) = delete;
		AbstractManagedFuture(AbstractManagedFuture&&) = delete;
		AbstractManagedFuture& operator=(AbstractManagedFuture&&) = delete;

		void move_(AbstractManagedFuture&& that, uint8_t full_output_size, uint8_t full_input_size);

	private:
		AbstractMultiFutureManager* manager_ = nullptr;

		friend class AbstractMultiFutureManager;
	};

	//TODO DOC?
	// More specialized abstract FutureManager with list of Futures
	class AbstractMultiFutureManager : public AbstractBaseFutureManager
	{
	public:
		/**
		 * Register a newly instantiated Future with this `AbstractMultiFutureManager`.
		 * A Future is useless until it has been registered.
		 * 
		 * This method is synchronized, it shall be called from outside an ISR.
		 * If you need the same feature called from an ISR, then you shall use 
		 * `AbstractMultiFutureManager.register_future_()` instead.
		 * 
		 * @param future a reference to a newly constructed Future that we want to register
		 * in order to use it as expected
		 * @retval true if @p future has been successfully registered by this `AbstractMultiFutureManager`
		 * @retval false if @p future could not be registered by this `AbstractMultiFutureManager`,
		 * either because the limit of registrable `Future`s has been reached already, or
		 * because @p future is already registered.
		 * 
		 * @sa Future
		 * @sa register_future_()
		 */
		bool register_future(AbstractManagedFuture& future)
		{
			synchronized return register_future_(future);
		}

		/**
		 * Register a newly instantiated Future with this `AbstractMultiFutureManager`.
		 * A Future is useless until it has been registered.
		 * 
		 * This method is not synchronized, it shall be called exclusively from an ISR,
		 * or possibly from inside a `synchronized` block.
		 * If you need the same feature with synchronization, then you shall use 
		 * `AbstractMultiFutureManager.register_future()` instead.
		 * 
		 * @param future a reference to a newly constructed Future that we want to register
		 * in order to use it as expected
		 * @retval true if @p future has been successfully registered by this `AbstractMultiFutureManager`
		 * @retval false if @p future could not be registered by this `AbstractMultiFutureManager`,
		 * either because the limit of registrable `Future`s has been reached already, or
		 * because @p future is already registered.
		 * 
		 * @sa Future
		 * @sa register_future()
		 */
		bool register_future_(AbstractManagedFuture& future)
		{
			// You cannot register an already registered future
			if (future.id() != 0)
				return false;
			// Optimization: we start search AFTER the last removed id
			for (uint8_t i = last_removed_id_; i < size_; ++i)
				if (register_at_index_(future, i))
					return true;
			for (uint8_t i = 0; i <= last_removed_id_; ++i)
				if (register_at_index_(future, i))
					return true;
			return false;
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

	protected:
		/// @cond notdocumented
		AbstractMultiFutureManager(AbstractManagedFuture** futures, uint8_t size)
			: size_{size}, futures_{futures}
		{
			memset(futures_, 0, size_ * sizeof(AbstractManagedFuture*));
		}

		~AbstractMultiFutureManager() = default;

		bool register_at_index_(AbstractManagedFuture& future, uint8_t index)
		{
			if (futures_[index] != nullptr)
				return false;
			update_future_(future.id_, &future, nullptr);
			future.id_ = static_cast<uint8_t>(index + 1);
			future.manager_ = this;
			future.status_ = FutureStatus::NOT_READY;
			futures_[index] = &future;
			return true;
		}
		AbstractManagedFuture* find_future(uint8_t id) const
		{
			if ((id == 0) || (id > size_))
				return nullptr;
			return futures_[id - 1];
		}
		//TODO add destroy_future(id, address) to optimize update_future_

		bool update_future(uint8_t id, AbstractManagedFuture* old_address, AbstractManagedFuture* new_address)
		{
			synchronized return update_future_(id, old_address, new_address);
		}
		// Called by Future themselves (on construction, destruction, assignment)
		bool update_future_(uint8_t id, AbstractManagedFuture* old_address, AbstractManagedFuture* new_address)
		{
			//TODO do we really need this check? only id_ != 0 should be checked
			// Check id is plausible and address matches
			if (find_future(id) != old_address)
				return false;
			futures_[id - 1] = new_address;
			if (new_address == nullptr)
				last_removed_id_ = id;
			return true;
		}
		/// @endcond

	private:
		const uint8_t size_;
		AbstractManagedFuture** futures_;
		uint8_t last_removed_id_ = 0;

		friend class AbstractManagedFuture;
	};

	/// @cond notoducmented
	inline AbstractManagedFuture::~AbstractManagedFuture()
	{
		// Notify FutureManager about destruction
		if (manager_)
			manager_->update_future(id_, this, nullptr);
	}

	inline void AbstractManagedFuture::move_(
		AbstractManagedFuture&& that, uint8_t full_output_size, uint8_t full_input_size)
	{
		// In case this Future is valid, it must be first invalidated with FutureManager
		if (manager_)
			manager_->update_future_(id_, this, nullptr);

		// Now copy all attributes from rhs (output_data_ was already initialized when this was constructed)
		id_ = that.id_;
		manager_ = that.manager_;
		status_ = that.status_;
		error_ = that.error_;
		output_size_ = that.output_size_;
		input_size_ = that.input_size_;
		// Calculate data pointer attribute for next set value calls
		output_current_ = output_data_ + full_output_size - output_size_;
		input_current_ = input_data_ + full_input_size - input_size_;

		// Notify FutureManager about Future move
		if (manager_ && !manager_->update_future_(id_, &that, this))
			status_ = FutureStatus::INVALID;

		// Make rhs Future invalid
		that.id_ = 0;
		that.manager_ = nullptr;
		that.status_ = FutureStatus::INVALID;
	}
	/// @endcond

	/// @cond notdocumented
	template<typename T> struct FutureManager_trait
	{
		static constexpr const bool IS_FUTURE_MANAGER = false;
	};
	/// @endcond
}

#endif /* FUTURE_COMMONS_HH */
/// @endcond
