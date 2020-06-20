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
#include <fastarduino/errors.h>
#include <fastarduino/move.h>
#include <fastarduino/time.h>
#include <fastarduino/streams.h>

namespace future
{
	/// @cond notdocumented
	// Forward declarations
	class AbstractFuture;
	template<typename OUT, typename IN> class Future;
	/// @endcond

	class AbstractFutureManager
	{
	public:
		static AbstractFutureManager& instance()
		{
			return *instance_;
		}

		bool register_future(AbstractFuture& future)
		{
			synchronized return register_future_(future);
		}

		bool register_future_(AbstractFuture& future);

		template<typename OUT, typename IN> bool register_future(Future<OUT, IN>& future)
		{
			return register_future((AbstractFuture&) future);
		}

		template<typename OUT, typename IN> bool register_future_(Future<OUT, IN>& future)
		{
			return register_future_((AbstractFuture&) future);
		}

		uint8_t available_futures() const
		{
			synchronized return available_futures_();
		}

		uint8_t available_futures_() const
		{
			uint8_t free = 0;
			for (uint8_t i = 0; i < size_; ++i)
				if (futures_[i] == nullptr)
					++free;
			return free;
		}

		uint8_t get_future_value_size(uint8_t id) const
		{
			synchronized return get_future_value_size_(id);
		}

		bool set_future_finish(uint8_t id) const
		{
			synchronized return set_future_finish_(id);
		}

		bool set_future_value(uint8_t id, uint8_t chunk) const
		{
			synchronized return set_future_value_(id, chunk);
		}

		bool set_future_value(uint8_t id, const uint8_t* chunk, uint8_t size) const
		{
			synchronized return set_future_value_(id, chunk, size);
		}

		template<typename T> bool set_future_value(uint8_t id, const T& value) const
		{
			synchronized return set_future_value_(id, value);
		}

		bool set_future_error(uint8_t id, int error) const
		{
			synchronized return set_future_error_(id, error);
		}

		uint8_t get_storage_value_size(uint8_t id) const
		{
			synchronized return get_storage_value_size_(id);
		}

		bool get_storage_value(uint8_t id, uint8_t& chunk) const
		{
			synchronized return get_storage_value_(id, chunk);
		}

		bool get_storage_value(uint8_t id, uint8_t* chunk, uint8_t size) const
		{
			synchronized return get_storage_value_(id, chunk, size);
		}

		uint8_t get_future_value_size_(uint8_t id) const;

		bool set_future_finish_(uint8_t id) const;

		bool set_future_value_(uint8_t id, uint8_t chunk) const;

		bool set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const;

		template<typename T> bool set_future_value_(uint8_t id, const T& value) const;

		bool set_future_error_(uint8_t id, int error) const;

		uint8_t get_storage_value_size_(uint8_t id) const;

		bool get_storage_value_(uint8_t id, uint8_t& chunk) const;

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

	template<uint8_t SIZE>
	class FutureManager : public AbstractFutureManager
	{
	public:
		FutureManager() : AbstractFutureManager{buffer_, SIZE}, buffer_{} {}

	private:
		AbstractFuture* buffer_[SIZE];
	};

	enum class FutureStatus : uint8_t
	{
		INVALID = 0,
		NOT_READY,
		READY,
		ERROR
	};

	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, future::FutureStatus s);
	/// @endcond

	class AbstractFuture
	{
	public:
		uint8_t id() const
		{
			return id_;
		}

		FutureStatus status() const
		{
			return status_;
		}

		FutureStatus await() const
		{
			while (status_ == FutureStatus::NOT_READY)
				time::yield();
			return status_;
		}

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

	template<typename OUT_ = void, typename IN_ = void>
	class Future : public AbstractFuture
	{
		static_assert(sizeof(OUT_) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");
		static_assert(sizeof(IN_) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

	public:
		using OUT = OUT_;
		using IN = IN_;

		explicit Future(const IN& input = IN{})
			: AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}, input_{input} {}

		~Future() = default;

		Future(Future<OUT, IN>&& that) : AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}
		{
			move(std::move(that));
		}

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

		bool get(OUT& result)
		{
			if (await() != FutureStatus::READY)
				return false;
			result = output_;
			invalidate();
			return true;
		}

	protected:
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
