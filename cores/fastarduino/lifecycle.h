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
#include "types_traits.h"

/**
 * Contains the API around LifeCycle management implementation.
 * This concept is here to be extended with concrete needs (e.g. Futures).
 * The API is based on the following:
 * - A LifeCycleManager is a repository of some kind of objects
 * - A LifeCycleManager has a limited number of managed objects
 * - Lifecycle management is performed through the template type LifeCycle<T>
 * that manages a T instance; lifecycle allows to move an object across memory
 * location but always have its associated LifeCycleManager keep its latest location
 * - LifeCycle management starts when a LifeCycle<T> instance is registered with a
 * LifeCycleManager
 * - LifeCycle management ends when a registered LifeCycle<T> instance is destroyed
 * or explicitly deregistered from its LifeCycleManager
 * 
 * In addition, a template Proxy class can be used as a proxy to either a static
 * instance or a lifecycle-managed instance in a consistent way.
 */
namespace lifecycle
{
	/// @cond notdocumented
	// Forward declaration
	class AbstractLifeCycleManager;
	template<typename T> class LifeCycle;
	/// @endcond

	/**
	 * The abstract base class of all LifeCycle<T>.
	 * Once registered with a LifeCycleManager, it holds a unique id and the 
	 * reference to its LifeCycleManager.
	 * 
	 * You shall nomrally never use this class directly, butonly its template 
	 * subclass LifeCycle<T>.
	 * 
	 * @sa LifeCycle<T>
	 * @sa AbstractLifeCycleManager.register_()
	 */
	class AbstractLifeCycle
	{
	public:
		//TODO DOC
		AbstractLifeCycle() = default;
		AbstractLifeCycle(AbstractLifeCycle&& that);
		~AbstractLifeCycle();
		AbstractLifeCycle& operator=(AbstractLifeCycle&& that);

		AbstractLifeCycle(const AbstractLifeCycle&) = delete;
		AbstractLifeCycle& operator=(const AbstractLifeCycle&) = delete;

		uint8_t id() const
		{
			return id_;
		}
		AbstractLifeCycleManager* manager() const
		{
			return manager_;
		}

	private:
		uint8_t id_ = 0;
		AbstractLifeCycleManager* manager_ = nullptr;

		friend class AbstractLifeCycleManager;
	};

	/**
	 * The abstract base class of all LifeCycleManager. It encapsulates all
	 * needed API for lifecycle management.
	 * 
	 * @sa LifeCycleManager
	 * @sa LifeCycle<T>
	 */
	class AbstractLifeCycleManager
	{
	public:
		//TODO DOC
		template<typename T> uint8_t register_(LifeCycle<T>& instance)
		{
			return register_impl_(instance);
		}

		bool unregister_(uint8_t id)
		{
			AbstractLifeCycle** slot = find_slot_(id);
			if (slot == nullptr || *slot == nullptr)
				return false;
			AbstractLifeCycle& source = **slot;
			source.id_ = 0;
			source.manager_ = nullptr;
			*slot = nullptr;
			++free_slots_;
			return true;
		}

		uint8_t available_() const
		{
			return free_slots_;
		}
		
		bool move_(uint8_t id, AbstractLifeCycle& dest)
		{
			// Check this instance is managed
			AbstractLifeCycle** slot = find_slot_(id);
			if (slot == nullptr || *slot == nullptr) return false;
			// Perform move
			AbstractLifeCycle& source = **slot;
			dest.id_ = source.id_;
			dest.manager_ = this;
			source.id_ = 0;
			source.manager_ = nullptr;
			*slot = &dest;
			return true;
		}

		template<typename T> LifeCycle<T>* find_(uint8_t id) const
		{
			return static_cast<LifeCycle<T>*>(find_impl_(id));
		}

	protected:
		/// @cond notdocumented
		AbstractLifeCycleManager(AbstractLifeCycle** slots, uint8_t size)
			:	size_{size}, slots_{slots}, free_slots_{size}
		{
			memset(slots, 0, size * sizeof(AbstractLifeCycle*));
		}
		~AbstractLifeCycleManager() = default;
		AbstractLifeCycleManager(const AbstractLifeCycleManager&) = delete;
		AbstractLifeCycleManager& operator=(const AbstractLifeCycleManager&) = delete;
		/// @endcond

	private:
		uint8_t register_impl_(AbstractLifeCycle& instance)
		{
			// Youcannot register any instance if there are no free slots remaining
			if (free_slots_ == 0) return 0;
			// You cannot register an already registered future
			if (instance.id_ != 0) return 0;
			// Optimization: we start search AFTER the last removed id
			for (uint8_t i = last_removed_id_; i < size_; ++i)
				if (register_at_index_(instance, i))
					return i + 1;
			for (uint8_t i = 0; i <= last_removed_id_; ++i)
				if (register_at_index_(instance, i))
					return i + 1;
			return 0;
		}

		AbstractLifeCycle* find_impl_(uint8_t id) const
		{
			AbstractLifeCycle** slot = find_slot_(id);
			if (slot == nullptr)
				return nullptr;
			else
				return *slot;
		}

		AbstractLifeCycle** find_slot_(uint8_t id) const
		{
			if (id == 0 || id > size_)
				return nullptr;
			else
				return &slots_[id - 1];
		}

		bool register_at_index_(AbstractLifeCycle& instance, uint8_t index)
		{
			if (slots_[index] != nullptr)
				return false;
			instance.id_ = static_cast<uint8_t>(index + 1);
			instance.manager_ = this;
			slots_[index] = &instance;
			--free_slots_;
			return true;
		}

		const uint8_t size_;
		AbstractLifeCycle** slots_;
		uint8_t free_slots_;
		uint8_t last_removed_id_ = 0;
	};

	//TODO DOC
	template<uint8_t SIZE> class LifeCycleManager : public AbstractLifeCycleManager
	{
	public:
		LifeCycleManager() : AbstractLifeCycleManager{slots_buffer_, SIZE} {}

	private:
		AbstractLifeCycle* slots_buffer_[SIZE];
	};

	//TODO DOC
	template<typename T> class LifeCycle : public AbstractLifeCycle, public T
	{
	public:
		LifeCycle(const T& value = T{}) : AbstractLifeCycle{}, T{value} {}
		LifeCycle(LifeCycle<T>&& that) = default;
		~LifeCycle() = default;
		LifeCycle<T>& operator=(LifeCycle<T>&& that) = default;
	};

	//TODO DOC
	template<typename T> class Proxy
	{
	public:
		Proxy(T& dest) : id_{0}, dest_{&dest} {}
		template<typename U>
		Proxy(const LifeCycle<U>& dest) : id_{dest.id()}, manager_{dest.manager()}
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
		}
		Proxy(const Proxy&) = default;
		Proxy& operator=(const Proxy&) = default;

		T& operator*()
		{
			return *target();
		}
		T* operator&()
		{
			return target();
		}
		T* operator->()
		{
			return target();
		}

		uint8_t id() const
		{
			return id_;
		}
		T* destination() const
		{
			return dest_;
		}
		AbstractLifeCycleManager* manager() const
		{
			return manager_;
		}

	private:
		T* target() const
		{
			if (manager_ == nullptr)
				return dest_;
			else
				return manager_->find_<T>(id_);
		}

		const uint8_t id_;
		T* dest_ = nullptr;
		AbstractLifeCycleManager* manager_ = nullptr;
	};
}

#endif /* LIFECYCLE_HH */
/// @endcond
