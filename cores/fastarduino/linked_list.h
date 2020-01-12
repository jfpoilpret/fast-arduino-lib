//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * Utility API to handle linked list containers.
 */
#ifndef LINKEDLIST_HH
#define LINKEDLIST_HH

#include "utilities.h"
#include "types_traits.h"

/**
 * Contains all FastArduino generic containers:
 * - linked list
 * - ring buffer queue
 * Those containers are often used directly by several FastArduino API but you
 * may also use them for your own needs.
 */
namespace containers
{
	/// @cond notdocumented
	class LinkImpl
	{
	public:
		LinkImpl() INLINE = default;
		LinkImpl(const LinkImpl&) = default;
		LinkImpl& operator=(const LinkImpl&) = delete;

	protected:
		LinkImpl* next_ = nullptr;
		friend class LinkedListImpl;
		template<class T, class B> friend struct types_traits::derives_from;
	};

	class LinkedListImpl
	{
	public:
		LinkedListImpl() INLINE = default;
		LinkedListImpl(const LinkedListImpl&) = default;
		LinkedListImpl& operator=(const LinkedListImpl&) = delete;
		void insert(LinkImpl* item);
		bool remove(LinkImpl* item);
		template<typename F> void traverse(F f);

	protected:
		LinkImpl* head_ = nullptr;
	};
	/// @endcond

	/**
	 * Linked list of type @p T_ items.
	 * This list offers 3 operations:
	 * - insert a new item (at the beginning of the list)
	 * - remove any item from the list (O(n))
	 * - traverse all items of the list and execute a function on each
	 * 
	 * For the sake of SRAM size optimization, the list is not double linked.
	 * 
	 * A concrete example of `LinkedList` use in FastArduino API can be found 
	 * in `events::Dispatcher`. 
	 * 
	 * @tparam T_ the type of items stored in the list; this must be a `Link<T>`.
	 * @sa Link
	 */
	template<typename T_> class LinkedList : private LinkedListImpl
	{
	public:
		LinkedList()
		{
			// Check that T_ is a LinkImpl subclass
			using DERIVES_FROM_LINK = types_traits::derives_from<T_, LinkImpl>;
			UNUSED DERIVES_FROM_LINK dummy = DERIVES_FROM_LINK();
		}

		LinkedList(const LinkedList<T_>&) = default;
		LinkedList<T_>& operator=(const LinkedList<T_>&) = delete;

		/**
		 * The type of items in this list.
		 */
		using T = T_;

		/**
		 * Insert @p item at the beginning of this list.
		 * Note that @p item is not *copied* by the list, only a reference is kept,
		 * thus it is caller's responsibility to ensure that @p item will live
		 * at least as long as this list lives.
		 * @param item a reference to the inserted item
		 * @sa remove()
		 */
		void insert(T& item) INLINE
		{
			LinkedListImpl::insert(&item);
		}

		/**
		 * Remove @p item from this list.
		 * The item is first searched in the list (from the beginning to the end),
		 * and removed if found.
		 * Note that search is based on @p item **reference** (i.e. pointer), not 
		 * content.
		 * @param item a reference of the item to remove
		 * @retval true if @p item was found in this list and removed
		 * @retval false if @p item was not found in this list
		 */
		bool remove(T& item) INLINE
		{
			return LinkedListImpl::remove(&item);
		}

		/**
		 * Traverse all items of this list and execute @p f functor.
		 * 
		 * @tparam F the functor type: this type may just be a function pointer
		 * or a class overloading `operator()`; in any case, it is passed a reference
		 * to a @p T item and must return a `bool`; if it returns `true` for a given
		 * item, then that item will be removed from this list.
		 * @param func the functor to execute on each item
		 */
		template<typename F> void traverse(F func)
		{
			T* current = head();
			while (current != nullptr)
			{
				T* next = current->next();
				if (func(*current)) remove(*current);
				current = next;
			}
		}

	private:
		T* head() INLINE
		{
			return (T*) head_;
		}
	};

	/**
	 * A wrapper for items stored in a `LinkedList`.
	 * To use this class, you have to use the following idiom:
	 * @code
	 * class MyType: public Link<MyType> {...}
	 * @endcode
	 * 
	 * A concrete example of `Link` use in FastArduino API can be found 
	 * in `events::EventHandler`. 
	 * 
	 * If you have an existing class which you want to directly use as a `LinkedList`
	 * item type, but cannot modify it, then you should use `LinkWrapper` instead.
	 * 
	 * @tparam T_ the type of item wrapped by this class
	 * @sa LinkWrapper
	 * @sa LinkedList
	 */
	template<typename T_> class Link : private LinkImpl
	{
	public:
		/**
		 * The type of item wrapped by this class.
		 */
		using T = T_;
	
	protected:
		Link() = default;
		Link(const Link<T_>&) = default;
		Link<T_>& operator=(const Link<T_>&) = delete;

	private:
		T* next() INLINE
		{
			return (T*) next_;
		}
		friend class LinkedList<T>;
		template<class T, class B> friend struct types_traits::derives_from;
	};

	/**
	 * A wrapper for items stored in a `LinkedList`.
	 * Contrarily to `Link` class, you may wrap any existing type (even a simple
	 * type such as `bool`, `uint16_t`, `char`...)
	 * 
	 * The follwoing snippet shows how to use this class:
	 * @code
	 * class MyType {...};
	 * 
	 * using LINK = LinkWrapper<MyType>;
	 * 
	 * LinkedList<LINK> list;
	 * LINK item1 = LINK{MyType{...}};
	 * list.insert(item1);
	 * @endcode
	 * 
	 * @tparam T_ the type of item wrapped by this class
	 * @tparam TREF_ the default reference type to @p T_
	 * @tparam TREF_ the reference type of items wrapped; this is `T_&` by default,
	 * but this may be changed, for small size types (one or two bytes long) to 
	 * `T_` itself, in order to avoid additional code to handle reference extraction
	 * of an item.
	 * @tparam CTREF_ the constant reference type of items wrapped; this is
	 * `const T_&` by default, but this may be changed, for small size types
	 * (one or two bytes long) to `T_` itself, in order to avoid additional code
	 * to handle reference extraction of an item. 
	 * 
	 * @sa LinkedList
	 * @sa Link
	 */
	template<typename T_, typename TREF_ = T_&, typename CTREF_ = const T_&>
	class LinkWrapper : private LinkImpl
	{
	public:
		LinkWrapper(const LinkWrapper<T_, TREF_, CTREF_>&) = default;
		LinkWrapper<T_, TREF_, CTREF_>& operator=(const LinkWrapper<T_, TREF_, CTREF_>&) = delete;

		/** The type of item wrapped by this class. */
		using T = T_;

		/** The reference type of item wrapped by this class. */
		using TREF = TREF_;

		/** The constant reference type of item wrapped by this class. */
		using CTREF = CTREF_;

		/**
		 * Create a wrapper, usable in a `LinkedList`, for @p item.
		 */
		LinkWrapper(CTREF item) : item_{item} {}

		/**
		 * Return a reference to the wrapped item.
		 */
		TREF item()
		{
			return item_;
		}

		/**
		 * Return a constant reference to the wrapped item.
		 */
		CTREF item() const
		{
			return item_;
		}

	private:
		using TYPE = LinkWrapper<T, TREF, CTREF>;

		TYPE* next() INLINE
		{
			return (TYPE*) next_;
		}

		T item_;

		friend class LinkedList<TYPE>;
		template<class T, class B> friend struct types_traits::derives_from;
	};
}

#endif /* LINKEDLIST_HH */
/// @endcond
