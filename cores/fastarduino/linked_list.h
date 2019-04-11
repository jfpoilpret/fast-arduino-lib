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

//TODO maybe define a LinkWrapper (Link subclass) that is real wrapper of item?

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
		LinkImpl() INLINE : next_{0}
		{
		}

	protected:
		LinkImpl* next_;
		friend class LinkedListImpl;
	};

	class LinkedListImpl
	{
	public:
		LinkedListImpl() INLINE : head_{0}
		{
		}
		void insert(LinkImpl* item);
		bool remove(LinkImpl* item);
		template<typename F> void traverse(F f);

	protected:
		LinkImpl* head_;
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
		//TODO static assert that T is a Link<?>: need specific traits

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
		 * @param f the functor to execute on each item
		 */
		template<typename F> void traverse(F f)
		{
			T* current = head();
			while (current != 0)
			{
				T* next = current->next();
				if (f(*current)) remove(*current);
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
	 * @tparam T_ the type of item wrapped by this class
	 * @sa LinkedList
	 */
	template<typename T_> class Link : private LinkImpl
	{
	public:
		/**
		 * The type of item wrapped by this class.
		 */
		using T = T_;

	private:
		T* next() INLINE
		{
			return (T*) next_;
		}
		friend class LinkedList<T>;
	};
}

#endif /* LINKEDLIST_HH */
/// @endcond
