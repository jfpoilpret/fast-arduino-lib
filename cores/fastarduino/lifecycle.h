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
 * - objects can be moved around and still be properly referenced by their repository
 */
#ifndef LIFECYCLE_HH
#define LIFECYCLE_HH

#include <string.h>
#include "move.h"
#include "types_traits.h"

/**
 * Contains the API around LifeCycle management implementation.
 * This concept is here to be extended with concrete needs (e.g. Futures).
 * The API is based on the following:
 * - A LifeCycleManager is a repository of some kind of objects
 * - A LifeCycleManager has a limited number of managed objects
 * - Lifecycle management is performed through the template type `LifeCycle<T>`
 * that manages a T instance; lifecycle allows to move an object across memory
 * location but always have its associated LifeCycleManager keep its latest location
 * - LifeCycle management starts when a `LifeCycle<T>` instance is registered with a
 * LifeCycleManager
 * - LifeCycle management ends when a registered `LifeCycle<T>` instance is destroyed
 * or explicitly deregistered from its LifeCycleManager
 * 
 * In addition, two template Proxy and LightProxy classes can be used as a proxy 
 * to either a static instance or a lifecycle-managed instance in a consistent way.
 */
namespace lifecycle
{
	/// @cond notdocumented
	// Forward declaration
	class AbstractLifeCycleManager;
	template<typename T> class LifeCycle;
	/// @endcond

	/**
	 * The abstract base class of all `LifeCycle<T>`.
	 * Once registered with a LifeCycleManager, it holds a unique id and the 
	 * reference to its LifeCycleManager.
	 * 
	 * You shall normally never use this class directly, but only its template 
	 * subclass `LifeCycle<T>`.
	 * 
	 * @sa LifeCycle
	 * @sa AbstractLifeCycleManager.register_()
	 */
	class AbstractLifeCycle
	{
	public:
		/// @cond notdocumented
		AbstractLifeCycle() = default;
		AbstractLifeCycle(AbstractLifeCycle&& that);
		~AbstractLifeCycle();
		AbstractLifeCycle& operator=(AbstractLifeCycle&& that);

		AbstractLifeCycle(const AbstractLifeCycle&) = delete;
		AbstractLifeCycle& operator=(const AbstractLifeCycle&) = delete;
		/// @endcond

		/**
		 * The unique identifier of this lifecycle-managed instance, as provided 
		 * by the LifeCycleManager it was registered with.
		 * When `0`, it means this instance has not been registered yet, or it has 
		 * been unregistered since.
		 * 
		 * @note any registered instance may be implicitly unregistered after 
		 * being "moved" to another instance (through usage of its move-constructor
		 * or move-assignment operator).
		 * 
		 * @sa AbstractLifeCycleManager.register_()
		 * @sa AbstractLifeCycleManager.unregister_()
		 * @sa std::move()
		 */
		uint8_t id() const
		{
			return id_;
		}

		/**
		 * A pointer to the AbstractLifeCycleManager handling this instance.
		 * When `nullptr`, it means this instance has not been registered yet, 
		 * or it has been unregistered since.
		 * 
		 * @note any registered instance may be implicitly unregistered after 
		 * being "moved" to another instance (through usage of its move-constructor
		 * or move-assignment operator).
		 * 
		 * @sa AbstractLifeCycleManager.register_()
		 * @sa AbstractLifeCycleManager.unregister_()
		 * @sa std::move()
		 */
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
	 * @sa LifeCycle
	 */
	class AbstractLifeCycleManager
	{
	public:
		/**
		 * Register a `LifeCycle<T>` instance with this LifeCycleManager.
		 * From now on, @p instance is tracked by this LifeCycleManager,
		 * in particular if it is moved around, its latest address is updated.
		 * It is assigned a unique identifier so that it can be retrieved, later 
		 * on, by calling find_().
		 * 
		 * @warning This method is NOT synchronized and should be called either
		 * from an ISR, or from a synchronized block.
		 * 
		 * @tparam T the type encapsulated in a `LifeCycle<T>` type
		 * @param instance a reference to an `LifeCycle<T>` to register with this
		 * LifeCycleManager
		 * @return a unique identifier for the registered instance
		 * @retval 0 if an error occurred; this can happen when too many instances 
		 * are already registered with this LifeCycleManager, or when @p instance
		 * is already registered (with yhis or any LifeCycleManager).
		 * 
		 * @sa LifeCycle
		 * @sa unregister_()
		 * @sa find_()
		 */
		template<typename T> uint8_t register_(LifeCycle<T>& instance)
		{
			return register_impl_(instance);
		}

		/**
		 * Unregisters a `LifeCycle<T>` instance, identified by @p id, already 
		 * registered with this LifeCycleManager.
		 * 
		 * @note This is automatically called by `LifeCycle<T>` destructor.
		 * 
		 * @warning This method is NOT synchronized and should be called either
		 * from an ISR, or from a synchronized block.
		 * 
		 * @param id the unique identifier for the `LifeCycle<T>` instance to 
		 * unregister; this is the value returned by register_().
		 * @retval true if the `LifeCycle<T>` instance was found and successfully
		 * deregistered
		 * @retval false if there is no registered instance identified by @p id
		 * 
		 * @sa register_()
		 */
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

		/**
		 * Return the number of available "slots" for registration of new
		 * `LifeCycle<T>` instances.
		 * 
		 * @note This method is atomic, hnece it can freely be called from an ISR
		 * or normal code without any synchronization required.
		 */
		uint8_t available_() const
		{
			return free_slots_;
		}
		
		/**
		 * Move an already registered `LifeCycle<T>` instance (identified by @p id)
		 * to a new location, determine by @p dest.
		 * Once this method is called, the previous `LifeCycle<T>` instance becomes
		 * unusable (its identifier is reset to `0`).
		 * 
		 * @warning This method is automatically called when a registered `LifeCycle<T>`
		 * is moved to another instance. You should normally never need to call 
		 * this method directly. 
		 * 
		 * @warning This method is NOT synchronized and should be called either
		 * from an ISR, or from a synchronized block.
		 * 
		 * @param id the unique identifier for the `LifeCycle<T>` instance to 
		 * move; this is the value returned by register_().
		 * @param dest a reference to a `LifeCycle<T>` instance to receive the instance
		 * currently identified by @p id
		 * @retval true if the `LifeCycle<T>` instance was found and successfully
		 * moved to @p dest
		 * @retval false if there is no registered instance identified by @p id
		 */
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

		/**
		 * Find an existing `LifeCycle<T>` registered with this LifeCycleManager
		 * and identified by @p id.
		 * This is the only way to get a pointer to the right location of an
		 * instance.
		 * 
		 * @warning A LifeCycleManager can hold `LifeCycle<T>` instances of any
		 * @p T type, it does not keep information about actual type of a LifeCycle;
		 * hence it cannot ensure safe casting in `find_()` method. It is the 
		 * responsibility of the application developer to ensure consistency of
		 * the type used in `register_()` and in `find_()` for the same @p id !
		 * 
		 * @warning This method is NOT synchronized and should be called either
		 * from an ISR, or from a synchronized block.
		 * 
		 * @tparam T the type encapsulated in a `LifeCycle<T>` type
		 * @param id the unique identifier for the `LifeCycle<T>` instance to 
		 * look for; this is the value returned by register_().
		 * @return a pointer to the found `LifeCycle<T>` instance
		 * @retval nullptr if there is no registered instance identified by @p id
		 * 
		 * @sa LifeCycle
		 * @sa register_()
		 */
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
			// You cannot register any instance if there are no free slots remaining
			if (free_slots_ == 0) return 0;
			// You cannot register an already registered future
			if (instance.id_ != 0) return 0;
			// Find first free slot for registration
			AbstractLifeCycle** slot = &slots_[0];
			for (uint8_t i = 0; i < size_; ++i)
			{
				if (*slot == nullptr)
				{
					const uint8_t id = i + 1;
					instance.id_ = id;
					instance.manager_ = this;
					*slot = &instance;
					--free_slots_;
					return id;
				}
				++slot;
			}
			// No free slot found; should never come here since we checked free_slots_ first
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

		const uint8_t size_;
		AbstractLifeCycle** slots_;
		uint8_t free_slots_;
	};

	/**
	 * An actual LifeCycleManager implementation, based on `AbstractLifeCycleManager`,
	 * adding static storage for it.
	 * 
	 * @tparam SIZE the maximum number of `LifeCycle<T>` instances this 
	 * LifeCycleManager can register,
	 * 
	 * @sa AbstractLifeCycleManager
	 * @sa register_()
	 * @sa unregister_()
	 * @sa available_()
	 */
	template<uint8_t SIZE> class LifeCycleManager : public AbstractLifeCycleManager
	{
	public:
		LifeCycleManager() : AbstractLifeCycleManager{slots_buffer_, SIZE} {}

	private:
		AbstractLifeCycle* slots_buffer_[SIZE];
	};

	/**
	 * A mixin class allowing lifecycle management of any object of any type @p T.
	 * 
	 * @tparam T the type of object for which we want to manage lifecycle
	 * 
	 * @sa LifeCycleManager
	 */
	template<typename T> class LifeCycle : public AbstractLifeCycle, public T
	{
	public:
		/**
		 * Create a new `LifeCycle<T>` mixin for an object of type @p T.
		 * This can be registered with a LifeCycleManager and then tracked
		 * wherever it gets moved.
		 * 
		 * @warning Type T must be default-constructable if you use this 
		 * LifeCycle constructor.
		 */
		LifeCycle() : AbstractLifeCycle{}, T{} {}

		/**
		 * Create a new `LifeCycle<T>` mixin for object @p value of type @p T.
		 * This can be registered with a LifeCycleManager and then tracked
		 * wherever it gets moved.
		 * 
		 * @warning Type T must be copy-constructable if you use this 
		 * LifeCycle constructor.
		 * 
		 * @param value the original value of this @p T instance
		 */
		LifeCycle(const T& value) : AbstractLifeCycle{}, T{value} {}

		/**
		 * Create a new `LifeCycle<T>` mixin for object @p value of type @p T.
		 * This can be registered with a LifeCycleManager and then tracked
		 * wherever it gets moved.
		 * 
		 * @warning Type T must be move-constructable if you use this 
		 * LifeCycle constructor.
		 * 
		 * @param value the original value of this @p T instance
		 */
		LifeCycle(T&& value) : AbstractLifeCycle{}, T{std::move(value)} {}

		/**
		 * Crate a new `LifeCycle<T>` mixin object of type @p T, by moving an
		 * already existing `LifeCycle<T>` instance @p that.
		 * Once this is constructed, @p that was automatically unregistered from
		 * its LifeCycleManager.
		 * 
		 * @param that the rvalue of an existing `LifeCycle<T>` instance (possibly 
		 * registered already or not) to be moved into this.
		 * 
		 * @sa std::move()
		 * @sa AbstractLifeCycleManager::move_()
		 */
		LifeCycle(LifeCycle<T>&& that) = default;

		/**
		 * Destroy this `LifeCycle<T>` instance.
		 * That destructor unregisters this instance from its LifeCycleManager
		 * (if already registered).
		 */
		~LifeCycle() = default;

		/**
		 * Assign this `LifeCycle<T>` with @p that, by moving that already
		 * existing `LifeCycle<T>` instance.
		 * Before assignment, this is automatically unregistered from its 
		 * LifeCycleManager, if needed.
		 * Once assignment is completed, @p that was automatically unregistered from
		 * its LifeCycleManager.
		 * 
		 * @param that the rvalue of an existing `LifeCycle<T>` instance (possibly 
		 * registered already or not) to be moved into this.
		 * 
		 * @sa std::move()
		 * @sa AbstractLifeCycleManager::move_()
		 */
		LifeCycle<T>& operator=(LifeCycle<T>&& that) = default;
	};

	// Forward declarations for Proxy and LightProxy classes
	/// @cond notdocumented
	template<typename T> class Proxy;
	template<typename T> bool operator==(const Proxy<T>& a, const Proxy<T>& b);
	template<typename T> bool operator!=(const Proxy<T>& a, const Proxy<T>& b);
	template<typename T> class LightProxy;
	template<typename T> bool operator==(const LightProxy<T>& a, const LightProxy<T>& b);
	template<typename T> bool operator!=(const LightProxy<T>& a, const LightProxy<T>& b);
	/// @endcond

	//TODO define AbstractProxy to factor out common code?
	/**
	 * A proxy class that encapsulates access to a fixed @p T instance, or to
	 * a dynamic `LifeCycle<T>` instance.
	 * This allows to define a method which argument does not have to care whether
	 * the object passed should be dynamic or static.
	 * 
	 * @warning Since proxying a @p T instance incurs overhead (data size, code size
	 * and speed), you should use `Proxy<T>` only when it makes sense.
	 * 
	 * @tparam T the type of the object proxied
	 * 
	 * @sa LightProxy
	 */
	template<typename T> class Proxy
	{
	public:
		/**
		 * Create a `Proxy<T>` to a static reference.
		 * @param dest the reference to a @p T instance to proxify.
		 */
		Proxy(const T& dest)
			:	id_{0}, ptr_{reinterpret_cast<uintptr_t>(&dest)}, is_dynamic_{false} {}

		/**
		 * Create a `Proxy<T>` to a `LifeCycle<U>` instance (dynamic reference).
		 * 
		 * @tparam U the type of reference held by @p dest; must be @p T or a
		 * subclass of @p T, otherwise code will not compile.
		 * @param dest the reference to a `LifeCycle<U>` instance to proxify; if
		 * @p dest is later moved, it will be automatically tracked by this Proxy<T>.
		 * 
		 * @sa LifeCycle
		 */
		template<typename U>
		Proxy(const LifeCycle<U>& dest)
			:	id_{dest.id()}, ptr_{reinterpret_cast<uintptr_t>(dest.manager())}, is_dynamic_{true}
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
		}

		/// @cond notdocumented
		constexpr Proxy() = default;
		template<typename U> Proxy(const Proxy<U>& that)
			:	id_{that.id_}, ptr_{that.ptr_}, is_dynamic_{that.is_dynamic_}
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;

		}
		template<typename U> Proxy& operator=(const Proxy<U>& that)
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
			id_ = that.id_;
			ptr_ = that.ptr_;
			is_dynamic_ = that.is_dynamic_;
			return *this;
		}
		/// @endcond

		/**
		 * Return a reference to the proxified @p T instance.
		 */
		T& operator*()
		{
			return *target();
		}

		/**
		 * Overloaded `->` operator, allowing access to proxified type @p T instance
		 * members.
		 */
		T* operator->()
		{
			return target();
		}

		/**
		 * Tell if this Proxy is dynamic or static.
		 * A dynamic proxy was constructed with a `LifeCycle<T>` instance and thus 
		 * ensures that the actual instance will aways be referenced even if it gets 
		 * moved.
		 * A static proxy was constructed directly with a @p T instance.
		 */
		bool is_dynamic() const
		{
			return is_dynamic_;
		}

		/**
		 * The identifier of the proxified `LifeCycle<U>` or `0` if a static instance
		 * was proxified.
		 */
		uint8_t id() const
		{
			return id_;
		}

		/**
		 * A pointer to the static instance proxified, or `nullptr` if a dynamic 
		 * instance (a `LifeCycle<U>`) was proxified.
		 */
		T* destination() const
		{
			return (is_dynamic_ ? nullptr : reinterpret_cast<T*>(ptr_));
		}

		/**
		 * The LifeCycleManager managing the proxified `LifeCycle<U>`, or `nullptr`
		 * if a static instance was proxified.
		 */
		AbstractLifeCycleManager* manager() const
		{
			return (is_dynamic_ ? reinterpret_cast<AbstractLifeCycleManager*>(ptr_) : nullptr);
		}

	private:
		T* target() const
		{
			if (is_dynamic_)
				return reinterpret_cast<AbstractLifeCycleManager*>(ptr_)->find_<T>(id_);
			else
				return reinterpret_cast<T*>(ptr_);
		}

		struct
		{
			uint8_t id_;
			uintptr_t ptr_ : 15;
			bool is_dynamic_ : 1;
		};

		template<typename U> friend class Proxy;
		friend bool operator==<T>(const Proxy<T>&, const Proxy<T>&);
		friend bool operator!=<T>(const Proxy<T>&, const Proxy<T>&);
	};

	//TODO API DOC
	template<typename T> Proxy<T> make_proxy(const T& dest)
	{
		return Proxy<T>{dest};
	}
	template<typename T> Proxy<T> make_proxy(const LifeCycle<T>& dest)
	{
		return Proxy<T>{dest};
	}

	/// @cond notdocumented
	template<typename T>
	bool operator==(const Proxy<T>& a, const Proxy<T>& b)
	{
		if (&a == &b) return true;
		return (a.is_dynamic_ == b.is_dynamic_) && (a.ptr_ == b.ptr_) && (a.id_ == b.id_);
	}

	template<typename T>
	bool operator!=(const Proxy<T>& a, const Proxy<T>& b)
	{
		return (a.is_dynamic_ != b.is_dynamic_) || (a.ptr_ != b.ptr_) || (a.id_ != b.id_);
	}
	/// @endcond

	/**
	 * A light proxy class that encapsulates access to a fixed @p T instance, or to
	 * a dynamic `LifeCycle<T>` instance.
	 * Each instance uses 2 bytes, instead of 3 bytes for a Proxy instance.
	 * The downside is that a dynamic LightProxy (i.e. constructed with a `LifeCycle<T>`)
	 * has to be passed the proper LifeCycleManager every time we want to get the
	 * pointer to the actual proxied instance.
	 * 
	 * @warning Since proxying a @p T instance incurs overhead (data size, code size
	 * and speed), you should use `LightProxy<T>` only when it makes sense.
	 * 
	 * @tparam T the type of the object proxied
	 * 
	 * @sa Proxy
	 */
	template<typename T> class LightProxy
	{
	public:
		/**
		 * Create a `LightProxy<T>` to a static reference.
		 * @param dest the reference to a @p T instance to proxify.
		 */
		LightProxy(const T& dest) : ptr_{reinterpret_cast<uintptr_t>(&dest)}, is_dynamic_{false}  {}

		/**
		 * Create a `LightProxy<T>` to a `LifeCycle<U>` instance (dynamic reference).
		 * 
		 * @tparam U the type of reference held by @p dest; must be @p T or a
		 * subclass of @p T, otherwise code will not compile.
		 * @param dest the reference to a `LifeCycle<U>` instance to proxify; if
		 * @p dest is later moved, it will eb automatically tracked by this `Proxy<T>`.
		 * 
		 * @sa LifeCycle
		 */
		template<typename U>
		LightProxy(const LifeCycle<U>& dest) : id_{dest.id()}, is_dynamic2_{true}
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
		}

		/**
		 * Create a `LightProxy<T>` from a `Proxy<T>`.
		 * @param proxy the original `Proxy<T>` to copy into this LightProxy
		 * 
		 * @sa Proxy
		 */
		LightProxy(const Proxy<T>& proxy)
		{
			if (proxy.is_dynamic())
			{
				id_ = proxy.id();
				is_dynamic2_ = true;
			}
			else
			{
				ptr_ = reinterpret_cast<uintptr_t>(proxy.destination());
				is_dynamic_ = false;
			}
		}

		/// @cond notdocumented
		constexpr LightProxy() : content_{} {}
		template<typename U> LightProxy(const LightProxy<U>& that) : content_{that.content_}
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
		}
		template<typename U> LightProxy& operator=(const LightProxy<U>& that)
		{
			// Statically check (at compile-time) that U is a subclass of T
			UNUSED types_traits::derives_from<U, T> check;
			content_ = that.content_;
			return *this;
		}
		/// @endcond

		/**
		 * Return a pointer to the proxified @p T instance.
		 * @param manager a pointer to the LifeCycleManager which was used to 
		 * register the underlying `LifeCycle<T>` instance; can be `nullptr` if this
		 * is a static proxy. Behaviour is undefined if `nullptr` and this LightProxy
		 * is dynamic.
		 */
		T* operator()(const AbstractLifeCycleManager* manager = nullptr) const
		{
			if (is_dynamic_)
				return manager->find_<T>(id_);
			else
				return reinterpret_cast<T*>(ptr_);
		}

		/**
		 * Tell if this LightProxy is dynamic or static.
		 * A dynamic proxy was constructed with a `LifeCycle<T>` instance and thus 
		 * ensures that the actual instance will aways be referenced even if it gets 
		 * moved.
		 * A static proxy was constructed directly with a @p T instance.
		 */
		bool is_dynamic() const
		{
			return is_dynamic_;
		}

		/**
		 * The identifier of the proxified `LifeCycle<U>` or `0` if a static instance
		 * was proxified.
		 */
		uint8_t id() const
		{
			return id_;
		}

		/**
		 * A pointer to the static instance proxified, or `nullptr` if a dynamic 
		 * instance (a `LifeCycle<U>`) was proxified.
		 */
		T* destination() const
		{
			return (is_dynamic_ ? nullptr : reinterpret_cast<T*>(ptr_));
		}

	private:
		union
		{
			uint16_t content_;
			struct
			{
				uintptr_t ptr_ : 15;
				uint16_t is_dynamic_ : 1;
			};
			struct
			{
				uint8_t id_;
				uint8_t reserved_ : 7;
				uint8_t is_dynamic2_ : 1;
			};
		};

		template<typename U> friend class LightProxy;
		friend bool operator==<T>(const LightProxy<T>&, const LightProxy<T>&);
		friend bool operator!=<T>(const LightProxy<T>&, const LightProxy<T>&);
	};

	//TODO API DOC
	template<typename T> LightProxy<T> make_light_proxy(const T& dest)
	{
		return LightProxy<T>{dest};
	}
	template<typename T> LightProxy<T> make_light_proxy(const LifeCycle<T>& dest)
	{
		return LightProxy<T>{dest};
	}

	/// @cond notdocumented
	template<typename T>
	bool operator==(const LightProxy<T>& a, const LightProxy<T>& b)
	{
		if (&a == &b) return true;
		return (a.is_dynamic_ == b.is_dynamic_) && (a.ptr_ == b.ptr_);
	}

	template<typename T>
	bool operator!=(const LightProxy<T>& a, const LightProxy<T>& b)
	{
		return (a.is_dynamic_ != b.is_dynamic_) || (a.ptr_ != b.ptr_);
	}
	/// @endcond
}

#endif /* LIFECYCLE_HH */
/// @endcond
